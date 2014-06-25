/*
 * driver/tty/serial/ak39_uart.c
 */

#if defined(CONFIG_SERIAL_AK39_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/clock.h>

#include "ak39_uart.h"

extern void printch(char);
extern void printascii(const char *);


#if 0
#define dbg(x...)	printk(x)
#else
#define dbg(x...)	do {} while(0)
#endif

/* UART name and device definitions */
#define NR_PORTS		2

#define AK39_SERIAL_NAME	"ttySAK"
#define AK39_SERIAL_MAJOR	204
#define AK39_SERIAL_MINOR	64


#ifdef CONFIG_SERIAL_AK39_CONSOLE

static struct console ak39_serial_console;

#define AK39_SERIAL_CONSOLE &ak39_serial_console
#else
#define AK39_SERIAL_CONSOLE NULL
#endif

struct ak39_uart_port {
	char			*name;
	struct uart_port	port;

	unsigned char __iomem   *rxfifo_base;
	unsigned char __iomem   *txfifo_base;

	unsigned int		rxfifo_offset;
	unsigned int		nbr_to_read;
	unsigned int		timeout_cnt;

	unsigned char		claimed;
	struct clk		*clk;
#ifdef CONFIG_CPU_FREQ
	struct notifier_block freq_transition;
#endif
};

/* macros to change one thing to another */
#define tx_enabled(port)	((port)->unused[0])
#define rx_enabled(port)	((port)->unused[1])

static int uart_intevent_decode(unsigned long status, unsigned int maskbit, unsigned int statusbit)
{
	if ((status & 1<<maskbit) && (status & 1<<statusbit))
		return 1;
	else
		return 0;
}

static inline void uart_subint_disable(struct ak39_uart_port *ourport, unsigned long mask)
{
	unsigned long uart_reg;
	
	/* disable tx_end interrupt */
	uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
	uart_reg &= ~mask;
	__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
}

static inline void uart_subint_enable(struct ak39_uart_port *ourport, unsigned long unmask)
{
	unsigned long uart_reg;

	/* enable tx_end interrupt */
	uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
	uart_reg |= unmask;
	__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
}

static void uart_subint_clear(struct ak39_uart_port *ourport, unsigned int subint)
{
	unsigned long uart_reg;

	switch (subint) {

	case TX_THR_INT:
		uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
		uart_reg |= (1<<subint);
		__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
	break;

	case RX_THR_INT:
		uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
		uart_reg &= AKUART_INT_MASK;
		uart_reg |= (1<<subint);
		__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
		break;
	
	case RECVDATA_ERR_INT:
		uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
		uart_reg &= AKUART_INT_MASK;
		uart_reg |= (0x1 << subint);
		__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
		break;

	case RX_TIMEOUT:
		uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
		uart_reg |= (0x1 << subint);
		uart_reg &= ~( 0x1<<3 );
		__raw_writel(uart_reg, ourport->port.membase + UART_CONF2); 
		
		/* start to receive data */
		uart_reg = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
		uart_reg |= (1 << 31);
		__raw_writel(uart_reg, ourport->port.membase + BUF_THRESHOLD);	
		break;

	default:
		printk(KERN_ERR "ak39xx 9xx 9xx 9xx 9xx 9xx 9xx 9xx 9xx kown subint type: %d\n", subint);
		break;
	}

	return;
}

static inline struct ak39_uart_port *to_ourport(struct uart_port *port)
{
	return container_of(port, struct ak39_uart_port, port);
}

static inline void uart_txend_interrupt(struct ak39_uart_port *ourport, unsigned short status)
{
	unsigned long uart_reg;

	/*handle Tx_end end interrupt */
	uart_reg = __raw_readl(ourport->port.membase + UART_CONF2);
	switch(status)
	{
		case ENABLE:
			uart_reg |= (UARTN_CONFIG2_TX_END_INT_EN);
			break;
			
		case DISABLE:
			uart_reg &= ~(UARTN_CONFIG2_TX_END_INT_EN);
			break;
			
		default:
			break;
	}
	__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);
}

/* clear a UARTn buffer status flag */
static inline void clear_uart_buf_status(struct ak39_uart_port *ourport, unsigned short status)
{
    unsigned long regval;
	unsigned long flags;

	local_irq_save(flags);
	
	regval = __raw_readl(AK_VA_L2CTRL + 0x8C);
	switch(status)
	{
		case RX_STATUS: 
			regval |= (0x1 << (17 + ourport->port.line * 2));
			break;
			
		case TX_STATUS:
			regval |= (0x1 << (16 + ourport->port.line * 2));
			break;
				
		default:
			break;
    }
	__raw_writel(regval,  AK_VA_L2CTRL + 0x8C);

	local_irq_restore(flags);
}

/* clear TX and RX internal status */
static inline void clear_internal_status(struct ak39_uart_port *ourport, unsigned short status)
{
    unsigned long regval;
	unsigned long flags;

	local_irq_save(flags);
	
	regval = __raw_readl(ourport->port.membase + UART_CONF1);
    switch(status)
	{
		case RX_STATUS:  
			__raw_writel(regval | (0x1 << 29), ourport->port.membase + UART_CONF1);
			break;

		case TX_STATUS:
			__raw_writel(regval | (0x1 << 28), ourport->port.membase + UART_CONF1);
    		break;			
			
		default:
			break;
    }

	local_irq_restore(flags);
}

/* clear TX_th and RX_th count interrupt */
static inline void clear_Int_status(struct ak39_uart_port *ourport, unsigned short status)
{
    unsigned long regval;
	unsigned long flags;

	local_irq_save(flags);
	
	regval = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
    switch(status)
	{
		case RX_STATUS:  
			__raw_writel(regval | (0x1 << 5), ourport->port.membase + BUF_THRESHOLD);
			break;

		case TX_STATUS:
			__raw_writel(regval | (0x1 << 11), ourport->port.membase + BUF_THRESHOLD);
    		break;			
			
		default:
			break;
    }

	local_irq_restore(flags);
}


/* enable/disable interrupt of  RX_th */
static inline void uart_Rx_interrupt(struct ak39_uart_port *ourport, unsigned short status)
{
    unsigned long regval;
	unsigned long flags;

	local_irq_save(flags);
	
	regval = __raw_readl(ourport->port.membase + UART_CONF2);
	if(status)
		__raw_writel(regval | (0x1 << RX_INTTERUPT), ourport->port.membase + UART_CONF2);
	else
		__raw_writel(regval & ~(0x1 << RX_INTTERUPT), ourport->port.membase + UART_CONF2);

	local_irq_restore(flags);
}

static inline int uart_hwport_init(struct ak39_uart_port *ourport)
{
	/* set share pin to UARTn, and disable pull-up */
	switch (ourport->port.line) {
	case 0:
		ak_group_config(ePIN_AS_UART1);

		/* clear tx/rx buffer status flag */
		rL2_CONBUF8_15 |= (0x3 << 16);
		/* enable buffer status bit may be changed */		
		rL2_FRACDMAADDR |= (0x1 << 29);
		break;

	case 1:
		/* set share pin */
		ak_group_config(ePIN_AS_UART2);

		rL2_CONBUF8_15 |= (0x3 << 18);
		rL2_FRACDMAADDR |= (0x1 << 29);
		break;
		
	default:
		printk(KERN_ERR "unknown uart port\n");
		return -1;
		break;
	}

	return 0;
}

static int uart_enable_clock(struct ak39_uart_port *ourport, int enable)
{
	unsigned long regval;

	regval = __raw_readl(AK_VA_SYSCTRL + 0x1C);

	switch (ourport->port.line) {
		case 0:
			if (enable)
				regval &= ~(1 << 7);
			else 
				regval |= (1 << 7);
			
		case 1:
			if (enable)
				regval &= ~(1 << 8);
			else 
				regval |= (1 << 8);
			break;

		default:
			printk(KERN_ERR "unknown uart port\n");
			return -1;
	}
	__raw_writel(regval, AK_VA_SYSCTRL + 0x1C);

	return 0;
}

/* power management control */
static void ak39_serial_pm(struct uart_port *port, unsigned int level, unsigned int old)
{
	switch (level) {
	case 3: /* disable */	
		//dbg("%s: enterring pm level: %d\n", __FUNCTION__, level);
		break;

	case 0:	/* enable  */
		//dbg("%s: enterring pm level: %d\n", __FUNCTION__, level);
		break;

	default:
		dbg(KERN_ERR "ak39xx serial: unknown pm %d\n", level);
		break;
	}
}

/* is tx fifo empty */
static unsigned int ak39_serial_tx_empty(struct uart_port *port)
{
	unsigned long uart_reg;

	uart_reg = __raw_readl(port->membase + UART_CONF2);

	if (uart_reg & (1 << TXFIFO_EMPTY))
		return 1;

	return 0;
}

/* no modem control lines */
static unsigned int ak39_serial_get_mctrl(struct uart_port *port)
{
	/* FIXME */
	dbg("%s\n", __FUNCTION__);
	return 0;
}

static void ak39_serial_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/* todo - possibly remove AFC and do manual CTS */
	dbg("%s\n", __FUNCTION__);
}


static void ak39_serial_start_tx(struct uart_port *port)
{
	struct ak39_uart_port *ourport = to_ourport(port);

	dbg("%s\n", __FUNCTION__); 

	if (!tx_enabled(port))
	{
		uart_txend_interrupt(ourport, ENABLE);
		tx_enabled(port) = 1;
	}
}

static void ak39_serial_stop_tx(struct uart_port *port)
{
	struct ak39_uart_port *ourport = to_ourport(port);

	dbg("%s\n", __FUNCTION__); 

	if (tx_enabled(port))
   	{
   		uart_txend_interrupt(ourport, DISABLE);
		tx_enabled(port) = 0;
	}
}

static void ak39_serial_stop_rx(struct uart_port *port)
{
	struct ak39_uart_port *ourport = to_ourport(port);

	dbg("%s\n", __FUNCTION__);

	if (rx_enabled(port))
   	{
		uart_Rx_interrupt(ourport, DISABLE);
		rx_enabled(port) = 0;
	}
}

static void ak39_serial_enable_ms(struct uart_port *port)
{
	dbg("%s\n", __FUNCTION__);
}

static void ak39_serial_break_ctl(struct uart_port *port, int break_state)
{
	dbg("%s\n", __FUNCTION__);
}

static irqreturn_t ak39_uart_irqhandler(int irq, void *dev_id)
{
	struct ak39_uart_port	*ourport = dev_id;
	struct uart_port *port = &ourport->port;
	struct circ_buf *xmit = &port->state->xmit;	    //&port->info->xmit;
	struct tty_struct *tty = port->state->port.tty;	//port->info->tty;
	unsigned int flag= TTY_NORMAL;

	unsigned char __iomem   *pbuf;
	unsigned char *pxmitbuf;
	unsigned long uart_status;
	unsigned int rxcount = 0;
	unsigned char ch = 0;
	unsigned int i;
	int txcount , tx_tail;
	unsigned int l2_offset = 0;
	unsigned long regval;

	uart_status = __raw_readl(ourport->port.membase + UART_CONF2);

	/* clear R_err interrupt */
	if ( uart_intevent_decode(uart_status, RECVDATA_ERR_INT_ENABLE, RECVDATA_ERR_INT) )
	{
		uart_subint_clear(ourport, RECVDATA_ERR_INT);
	}

	if ( uart_intevent_decode(uart_status, TX_END_INTERRUPT, TX_END_STATUS) )
	{
		/* if there is not anything more to transmit, or the uart is now
		 * stopped, disable the uart and exit
		 */
		if (uart_circ_empty(xmit) || uart_tx_stopped(port))
		{
			ak39_serial_stop_tx(port);
			goto rx_irq;
		}

		txcount = uart_circ_chars_pending(xmit);

		if(txcount > 32)
			txcount = 32;
		pbuf = ourport->txfifo_base;
		pxmitbuf = xmit->buf;

		/* clear a uartx buffer status */
		clear_uart_buf_status(ourport, TX_STATUS);

		/* clear the tx internal status */
		clear_internal_status(ourport, TX_STATUS);

		__raw_writel(0x0, ourport->txfifo_base + 0x3C);

		l2_offset = 0;
		tx_tail = xmit->tail;
		regval = 0;
		for(i = 0; i < txcount; i++)
		{
			regval |= pxmitbuf[tx_tail]<<((i & 3) * 8 );
			if((i & 3) == 3)
			{
				__raw_writel(regval, pbuf + l2_offset);
				l2_offset = l2_offset + 4;
				regval = 0;
			}
			tx_tail = (tx_tail + 1) & (UART_XMIT_SIZE - 1);
			port->icount.tx += 1;
		}
		if(i & 3)
		{
			__raw_writel(regval, pbuf + l2_offset);
		}

		regval = (__raw_readl(ourport->port.membase + UART_CONF2)&(~UARTN_CONFIG2_TX_BYT_CNT_MASK)) | (UARTN_CONFIG2_TX_BYT_CNT(txcount))  | (UARTN_CONFIG2_TX_BYT_CNT_VLD);
		__raw_writel(regval, ourport->port.membase + UART_CONF2);
		xmit->tail = tx_tail;

		if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
			uart_write_wakeup(port);

		if (uart_circ_empty(xmit))
			ak39_serial_stop_tx(port);
	}
	
rx_irq:

	/* rx threshold interrupt */
	if ( uart_intevent_decode(uart_status, RX_THR_INT_ENABLE, RX_THR_INT) ||
		uart_intevent_decode(uart_status, RX_TIMEOUT_INT_ENABLE, RX_TIMEOUT))
	{
		if ( uart_intevent_decode(uart_status, RX_THR_INT_ENABLE, RX_THR_INT))
			uart_subint_clear(ourport, RX_THR_INT);
		else {
			uart_subint_clear(ourport, RX_TIMEOUT);
		}

		while(__raw_readl(ourport->port.membase + UART_CONF2) & (UARTN_CONFIG2_MEM_RDY));
		
		ourport->nbr_to_read = (__raw_readl(ourport->port.membase + DATA_CONF)>>13) & 0x7f;

		if (ourport->nbr_to_read  != ourport->rxfifo_offset) {
			l2_offset = ourport->rxfifo_offset; 
			pbuf = ourport->rxfifo_base + l2_offset;

			/* copy data */
			if (ourport->nbr_to_read > l2_offset) {
            			rxcount = ourport->nbr_to_read - l2_offset;
				for (i=0; i<rxcount; i++) {
					ch = __raw_readb(pbuf + i);
					uart_insert_char(port, 0, 0, ch, flag);
				}	
		        } else {
				rxcount = (UART_RX_FIFO_SIZE - l2_offset);
				for (i=0; i<rxcount; i++) {
					ch = __raw_readb(pbuf + i);
					uart_insert_char(port, 0, 0, ch, flag);
				}
			
				pbuf = ourport->rxfifo_base;
				for (i=0; i < ourport->nbr_to_read; i++) {
					ch = __raw_readb(pbuf + i);
					uart_insert_char(port, 0, 0, ch, flag);
				}
			}
			ourport->rxfifo_offset = ourport->nbr_to_read;
		}

		tty_flip_buffer_push(tty);
	}

	return IRQ_HANDLED;

}

static void ak39_serial_shutdown(struct uart_port *port)
{
	struct ak39_uart_port *ourport = to_ourport(port);
	unsigned int uart_reg = 0;

	/*
	 * 1st, free irq.
	 * 2nd, disable/mask hw uart setting.
	 * 3rd, close uart clock.
	 */
	
	/* mask all interrupt */
	__raw_writel(0, ourport->port.membase + UART_CONF2);
	
	uart_reg = __raw_readl(ourport->port.membase + 0xc);
	uart_reg  &= (~(1<<5));
	__raw_writel(uart_reg, ourport->port.membase + 0xc);
	uart_reg = __raw_readl(ourport->port.membase + 0x0);
	uart_reg  &= (~(1<<29));
	__raw_writel(uart_reg, ourport->port.membase + 0x0);
	/* clear uartx interrupt */
	uart_reg  &= (~1<<21);
	__raw_writel(uart_reg, ourport->port.membase + 0x0);

	/* clear uartn TX/RX buf status flag and call ak_setpin_as_gpio() */
	switch(ourport->port.line) {
		case 0:
			rL2_CONBUF8_15 |= (0x3<<16);
			break;
		case 1:
			rL2_CONBUF8_15 |= (0x3<<18);
			break;
	}	
	free_irq(port->irq, ourport);	
	uart_enable_clock(ourport, 0);
}

/*
 * 1, setup gpio.
 * 2, enable clock.
 * 3, request irq and setting up uart control.
 * 4, enable subirq.
 */
static int ak39_serial_startup(struct uart_port *port)
{
	struct ak39_uart_port *ourport = to_ourport(port);
	unsigned long uart_reg;
	int ret;

	if ( rx_enabled(port) && tx_enabled(port))
		return 0;

	/* enable uart clock */
	uart_enable_clock(ourport, 1);
	
	/* set share pin to UARTn */
	uart_hwport_init(ourport);

	//clear L2 Buffer
	clear_uart_buf_status(ourport, RX_STATUS);
	
	uart_reg = UARTN_CONFIG1_RTS_EN_BY_CIRCUIT | UARTN_CONFIG1_EN |UARTN_CONFIG1_RX_STA_CLR|UARTN_CONFIG1_TX_STA_CLR|UARTN_CONFIG1_TIMEOUT_EN;
	__raw_writel(uart_reg, ourport->port.membase + UART_CONF1);

	/* mask all interrupt */
	__raw_writel(0, ourport->port.membase + UART_CONF2);

	/*
	 * config stop bit and timeout value
	 * set timeout = 32, stop bit = 1;
	 */
	uart_reg = (0x1f << 16);
	__raw_writel(uart_reg, ourport->port.membase + UART_STOPBIT_TIMEOUT);	


   /* 
      * set threshold to 32bytes 
      * set RX_th_cfg_h = 0, set RX_th_cfg_l = 31
      */
	uart_reg = __raw_readl(ourport->port.membase + DATA_CONF);
	uart_reg &= ~UARTN_RX_TH_CFG_H_MASK;
	__raw_writel(uart_reg, ourport->port.membase + DATA_CONF);	
	uart_reg = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
    uart_reg &= ~(UARTN_RX_TH_CFG_L_MASK);
	uart_reg |= UARTN_RX_TH_CFG_L(0x1F);	/* 32 Bytes */
	//uart_reg |= (1 << 11);
	__raw_writel(uart_reg, ourport->port.membase + BUF_THRESHOLD);
	
	clear_internal_status(ourport, RX_STATUS);

	/* ourport->rxfifo_offset = 0; */
	ourport->rxfifo_offset = 0;

   	/* to clear  RX_th count interrupt */
	uart_reg = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
	uart_reg |= (UARTN_RX_TH_CLR);
	__raw_writel(uart_reg, ourport->port.membase + BUF_THRESHOLD);
	udelay(10);
	uart_reg = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
	uart_reg &= ~(UARTN_RX_TH_CLR);
	__raw_writel(uart_reg, ourport->port.membase + BUF_THRESHOLD);
		udelay(10);
	uart_reg = __raw_readl(ourport->port.membase + BUF_THRESHOLD);
	uart_reg |= (UARTN_RX_START);
	__raw_writel(uart_reg, ourport->port.membase + BUF_THRESHOLD);
	

    /*
	 *  enable timeout, rx mem_rdy and rx_th tx_end interrupt 
	 */
	 uart_reg = (UARTN_CONFIG2_RX_TH_INT_EN|UARTN_CONFIG2_RX_BUF_FULL_INT_EN|UARTN_CONFIG2_TIMEOUT_INT_EN|UARTN_CONFIG2_R_ERR_INT_EN);
	__raw_writel(uart_reg, ourport->port.membase + UART_CONF2);

	rx_enabled(port) = 1;
	tx_enabled(port) = 0;

	//ourport->rxfifo_offset =0;
	
	/* register interrupt */
	ret = request_irq(port->irq, ak39_uart_irqhandler, IRQF_DISABLED, ourport->name, ourport);
	if (ret) {
		printk(KERN_ERR "can't request irq %d for %s\n", port->irq, ourport->name);
		goto startup_err;
	}
	return 0;

startup_err:
	ak39_serial_shutdown(port);
	return ret;
}

static void ak39_serial_set_termios(struct uart_port *port, 
				struct ktermios *termios, struct ktermios *old)
{
	struct ak39_uart_port *ourport = to_ourport(port);
	unsigned int baud;
	unsigned long flags;
	unsigned long regval;
	unsigned long asic_clk;

	asic_clk = ak_get_asic_clk();

	termios->c_cflag &= ~(HUPCL | CMSPAR);
	termios->c_cflag |= CLOCAL;

	/*
	 * Ask the core to calculate the divisor for us.
	 * min: 2.4kbps, max: 2.4Mbps
	 */
	baud = uart_get_baud_rate(port, termios, old, 2400, 115200*20);

	spin_lock_irqsave(&port->lock, flags);

	/* baudrate setting */
	regval = __raw_readl(port->membase + UART_CONF1);
	regval &= ~(0xffff);
	regval &= ~(0x1 << 22);
	regval |= ((asic_clk / baud - 1) & 0xffff);
	regval |= (1 << 28) | (1 << 29);

	if (asic_clk % baud)
		regval |= (0x1 << 22);

	ourport->rxfifo_offset = 0;

	/* flow control setting */
	if(port->line != 0)
	{
		if((termios->c_cflag & CRTSCTS)) /* directly */
		{
			switch (port->line) {
			case 1:
				AK_GPIO_UART1_FLOW(1);
				break;
			}
			regval &= ~(1<<18|1<<19);
		}
		else  /* inversly */
		{
			switch(port->line) {
			case 1:
				ak_setpin_as_gpio(6);
				ak_setpin_as_gpio(7);
				break;
			}
			regval |= (1<<18|1<<19);
		}
	}

	/* parity setting */
	if (termios->c_cflag & PARENB) {
		if (termios->c_cflag & PARODD)
			regval |= (0x2 << 25);  /* odd parity */
		else
			regval |= (0x3 << 25);  /* evnt parity*/
	}
	__raw_writel(regval, port->membase + UART_CONF1);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	/*
	 * Which character status flags should we ignore?
	 */
	port->ignore_status_mask = 0;

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *ak39_serial_type(struct uart_port *port)
{
	switch (port->type) 
	{
		case PORT_AK39:
			return "AK39";
		default:
			return NULL;
	}
}

static void ak39_serial_release_port(struct uart_port *port)
{
	dbg("%s\n", __FUNCTION__);
}

static int ak39_serial_request_port(struct uart_port *port)
{
	dbg("%s\n", __FUNCTION__);
	return 0;
}

static void ak39_serial_config_port(struct uart_port *port, int flags)
{
	struct ak39_uart_port *ourport = to_ourport(port);

	port->type = PORT_AK39;
	ourport->rxfifo_offset = 0;
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int
ak39_serial_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	dbg("%s\n", __FUNCTION__);

	return 0;
}


static struct uart_ops ak39_serial_ops = {
	.pm             = ak39_serial_pm,
	.tx_empty       = ak39_serial_tx_empty,
	.get_mctrl      = ak39_serial_get_mctrl,
	.set_mctrl      = ak39_serial_set_mctrl,
	.stop_tx        = ak39_serial_stop_tx,
	.start_tx       = ak39_serial_start_tx,
	.stop_rx        = ak39_serial_stop_rx,
	.enable_ms      = ak39_serial_enable_ms,
	.break_ctl      = ak39_serial_break_ctl,
	.startup        = ak39_serial_startup,
	.shutdown       = ak39_serial_shutdown,
	.set_termios    = ak39_serial_set_termios,
	.type           = ak39_serial_type,
	.release_port   = ak39_serial_release_port,
	.request_port   = ak39_serial_request_port,
	.config_port    = ak39_serial_config_port,
	.verify_port    = ak39_serial_verify_port,
};


static struct ak39_uart_port ak39_serial_ports[NR_PORTS] = {
	[0] = {
		.name = "uart0",
		.rxfifo_base	= AK39_UART0_RXBUF_BASE,
		.txfifo_base	= AK39_UART0_TXBUF_BASE,
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(ak39_serial_ports[0].port.lock),
			.iotype		= UPIO_MEM,
			.mapbase	= AK39_UART0_PA_BASE,
			.membase	= AK39_UART0_BASE,
			.irq		= IRQ_UART0,
			.uartclk        = 0,
			.fifosize       = 64,
			.ops            = &ak39_serial_ops,
			.flags			= UPF_BOOT_AUTOCONF,
			.line           = 0,
		},
	#ifdef CONFIG_CPU_FREQ
	.freq_transition = {
			.priority = INT_MAX -1,
		},
	#endif
	},
	[1] = {
		.name = "uart1",
		.rxfifo_base	= AK39_UART1_RXBUF_BASE,
		.txfifo_base	= AK39_UART1_TXBUF_BASE,
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(ak39_serial_ports[1].port.lock),
			.iotype		= UPIO_MEM,
			.mapbase	= AK39_UART1_PA_BASE,
			.membase	= AK39_UART1_BASE,
			.irq		= IRQ_UART1,
			.uartclk	= 0,
			.fifosize	= 64,
			.ops		= &ak39_serial_ops,
			.flags		= UPF_BOOT_AUTOCONF,
			.line		= 1,
		},
	},
};

static struct uart_driver ak39_uart_drv = {
	.owner		= THIS_MODULE,
	.dev_name	= AK39_SERIAL_NAME,
	.driver_name	= AK39_SERIAL_NAME,
	.nr			= NR_PORTS,
	.major		= AK39_SERIAL_MAJOR,
	.minor		= AK39_SERIAL_MINOR,
	.cons		= AK39_SERIAL_CONSOLE,
};

#ifdef CONFIG_CPU_FREQ

static int ak39_serial_cpufreq_transition(struct notifier_block *nb,
    unsigned long val, void *data)
{
    struct ak39_uart_port *port;
    struct uart_port *uport;

    port = container_of(nb, struct ak39_uart_port, freq_transition);
    uport = &port->port;

    if(val == CPUFREQ_PRECHANGE){
        /* we should really shut the port down whilst the
		 * frequency change is in progress. */
    }
    else if(val == CPUFREQ_POSTCHANGE) {

        struct ktermios *termios;
        struct tty_struct *tty;

        if (uport->state == NULL)
            goto exit;

        tty = uport->state->port.tty;
        if (tty == NULL)
            goto exit;

        termios = tty->termios;
        if (termios == NULL) {
            printk(KERN_WARNING "%s: no termios?\n", __func__);
            goto exit;
        }

        ak39_serial_set_termios(uport, termios, NULL);
    }
exit:
    return 0;

}

static inline int ak39_serial_cpufreq_register(struct ak39_uart_port *port)
{
    port->freq_transition.notifier_call = ak39_serial_cpufreq_transition;

    return cpufreq_register_notifier(&port->freq_transition,
        CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void ak39_serial_cpufreq_unregister(struct ak39_uart_port *port)
{
    cpufreq_unregister_notifier(&port->freq_transition,
        CPUFREQ_TRANSITION_NOTIFIER);
}
#else
static inline int ak39_serial_cpufreq_register(struct ak39_uart_port *port)
{
    return 0;
}

static inline void ak39_serial_cpufreq_unregister(struct ak39_uart_port *port)
{
}

#endif


/* ak39_serial_init_port
 *
 * initialise a single serial port from the platform device given
 */
static int ak39_serial_init_port(struct ak39_uart_port *ourport,
		struct platform_device *platdev)
{
	struct uart_port *port = &ourport->port;

	if (platdev == NULL)
		return -ENODEV;

	/* setup info for port */
	port->dev = &platdev->dev;

	ourport->clk = clk_get(port->dev, "asic_clk");

	return 0;
}

static int ak39_serial_probe(struct platform_device *dev)
{
	struct ak39_uart_port *ourport;
	int ret = 0;

	ourport = &ak39_serial_ports[dev->id];
	
	ret = ak39_serial_init_port(ourport, dev);
	if (ret < 0)
		goto probe_err;

	uart_add_one_port(&ak39_uart_drv, &ourport->port);
	
	platform_set_drvdata(dev, &ourport->port);
	
	ret = ak39_serial_cpufreq_register(ourport);
	if(ret < 0)
		dev_err(&dev->dev, "faild to add cpufreq notifier\n");
	return 0;

probe_err:
	return ret;
}

static int ak39_serial_remove(struct platform_device *dev)
{
	struct uart_port *port = (struct uart_port *)dev_get_drvdata(&dev->dev);

	if (port) {		
		ak39_serial_cpufreq_unregister(to_ourport(port));
		uart_remove_one_port(&ak39_uart_drv, port);
	}
	return 0;
}

/* UART power management code */

#ifdef CONFIG_PM

static int ak39_serial_suspend(struct platform_device *dev, pm_message_t state)
{
	struct uart_port *port = (struct uart_port *)dev_get_drvdata(&dev->dev);

	if (port) 
		uart_suspend_port(&ak39_uart_drv, port);
	return 0;
}

static int ak39_serial_resume(struct platform_device *dev)
{
	struct uart_port *port = (struct uart_port *)dev_get_drvdata(&dev->dev);

	if (port) 	
		uart_resume_port(&ak39_uart_drv, port);
	return 0;
}
#else
#define ak39_serial_suspend NULL
#define ak39_serial_resume	NULL
#endif

static struct platform_driver ak39_serial_drv = {
	.probe          = ak39_serial_probe,
	.remove         = ak39_serial_remove,
	.suspend        = ak39_serial_suspend,
	.resume         = ak39_serial_resume,
	.driver         = {
		.name   = "ak39-uart",
		.owner  = THIS_MODULE,
	},
};


/* module initialisation code */
static int __init ak39_serial_modinit(void)
{
	int ret;

	printk("AK39xx uart driver init, (c) 2013 ANYKA\n");

	//register 
	ret = uart_register_driver(&ak39_uart_drv);
	if (ret < 0) {
		printk(KERN_ERR "failed to register UART driver\n");
		return -1;
	}

	platform_driver_register(&ak39_serial_drv);

	return 0;
}

static void __exit ak39_serial_modexit(void)
{
	platform_driver_unregister(&ak39_serial_drv);
	
	uart_unregister_driver(&ak39_uart_drv);
}

module_init(ak39_serial_modinit);
module_exit(ak39_serial_modexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("anyka");
MODULE_DESCRIPTION("Anyka Serial port driver");


/************* Console code ************/
#ifdef CONFIG_SERIAL_AK39_CONSOLE

static struct uart_port *cons_uart;

static inline void ak39_uart_putchar(struct ak39_uart_port *ourport, unsigned char ch)
{
	unsigned long regval;

	/* clear the tx internal status */
	clear_internal_status(ourport, TX_STATUS);

	/* clear a uartx buffer status */
	clear_uart_buf_status(ourport, TX_STATUS);

	/*to inform the buf is full*/	
	__raw_writel(ch, ourport->txfifo_base);
	__raw_writel(0x0, ourport->txfifo_base + 0x3C);

   	/* to clear  TX_th count interrupt */
	clear_Int_status(ourport, TX_STATUS);

	/* start to transmit */
	regval = __raw_readl(ourport->port.membase + UART_CONF2);
	regval &= AKUART_INT_MASK;
	regval |= (0x1<<4) | (0x1<<16);
	__raw_writel(regval, ourport->port.membase + UART_CONF2);
	

	/* wait for tx end */
	while (!(__raw_readl(ourport->port.membase + UART_CONF2) & (1 << TX_END_STATUS)))
		;
}

static inline void ak39_wait_for_txend(struct ak39_uart_port *ourport)
{
	unsigned int timeout = 10000;

	/*
	 * Wait up to 10ms for the character(s) to be sent
	 */
    while (!(__raw_readl(ourport->port.membase + UART_CONF2) & (1 << TX_END_STATUS))) {
        if (--timeout == 0)
            break;
        udelay(1);
    }
}

static void
ak39_serial_console_putchar(struct uart_port *port, int ch)
{
	struct ak39_uart_port *ourport = to_ourport(port);

	ak39_wait_for_txend(ourport);

	ak39_uart_putchar(ourport, ch);
}

static void
ak39_serial_console_write(struct console *co, const char *s, unsigned int count)
{
	uart_console_write(cons_uart, s, count, ak39_serial_console_putchar);
}

static void __init
ak39_serial_get_options(struct uart_port *port, int *baud, int *parity, int *bits)
{

#if 0
	unsigned long regval;
	struct clk *clk;

	*bits	= 8;

	regval = __raw_readl(port->membase + UART_CONF1);

	if (regval & 0x1<<26) {
		if (regval & 0x1<<25)
			*parity = 'e';
		else
			*parity = 'o';
	}
	else
		*parity = 'n';

	clk = clk_get(port->dev, "asic_clk");
	if (!IS_ERR(clk) && clk != NULL)
		*baud = clk_get_rate(clk) / ((regval & 0xFFFF) + 1);

	printk("calculated baudrate: %d\n", *baud);
#endif
}


static int __init
ak39_serial_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	dbg("ak39_serial_console_setup: co=%p (%d), %s\n", co, co->index, options);

	port = &ak39_serial_ports[co->index].port;

	/* is this a valid port */

	if (co->index == -1 || co->index >= NR_PORTS)
		co->index = 0;

	dbg("ak39_serial_console_setup: port=%p (%d)\n", port, co->index);

	cons_uart = port;

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, search for the first available port that does have
	 * console support.
	 */
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	else
		ak39_serial_get_options(port, &baud, &parity, &bits);

	dbg("ak39_serial_console_setup: baud %d\n", baud);

	return uart_set_options(port, co, baud, parity, bits, flow);
}


static struct console ak39_serial_console = {
	.name		= AK39_SERIAL_NAME,
	.device		= uart_console_device,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.write		= ak39_serial_console_write,
	.setup		= ak39_serial_console_setup
};


/* ak39_serial_initconsole
 *
 * initialise the console from one of the uart drivers
*/
static int ak39_serial_initconsole(void)
{
	printk("AK39 console driver initial\n");

	ak39_serial_console.data = &ak39_uart_drv;

	register_console(&ak39_serial_console);

	return 0;
}

console_initcall(ak39_serial_initconsole);

#endif /* CONFIG_SERIAL_AK39_CONSOLE */

