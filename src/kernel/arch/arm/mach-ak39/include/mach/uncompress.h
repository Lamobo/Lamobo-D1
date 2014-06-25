/*
 * linux/arch/arm/mach-ak39/include/mach/uncompress.h
 *
 */
#ifndef __UNCOMPRESS_H_
#define __UNCOMPRESS_H_

#include <asm/sizes.h>
#include <mach/map.h>

#if defined(CONFIG_CPU_AK3910) || defined(CONFIG_CPU_AK3916) || defined(CONFIG_CPU_AK3918)
#define CONFIG_UART0_INIT
#endif

#define BAUD_RATE  					115200
#define ENDDING_OFFSET				60

#undef REG32
#define REG32(_reg)             	(*(volatile unsigned long *)(_reg))

#define CLK_ASIC_PLL_CTRL			(AK_PA_SYSCTRL + 0x08)

/* L2 buffer address */
#define UART0_TXBUF_ADDR			REG_PA_ADDR(0x48000000, 0x1000) //0x48001000
#define UART0_RXBUF_ADDR        	REG_PA_ADDR(0x48000000, 0x1080)
#define UART1_TXBUF_ADDR        	REG_PA_ADDR(0x48000000, 0x1100)
#define UART1_RXBUF_ADDR        	REG_PA_ADDR(0x48000000, 0x1180)

/* L2 buffer control register */
#define L2BUF_CONF2_REG         	REG_PA_VAL(0x20140000, 0x008C) //0x2014008c
#define UART0_TXBUF_CLR_BIT     	16
#define UART0_RXBUF_CLR_BIT     	17
#define UART1_TXBUF_CLR_BIT     	18
#define UART1_RXBUF_CLR_BIT     	19

/* pullup/pulldown configure registers */
#define PPU_PPD1_REG           	REG_PA_VAL(AK_PA_SYSCTRL, 0x80) //0x08000080
#define RTS1_PU_BIT             24
#define CTS1_PU_BIT             23
#define TXD1_PU_BIT             22
#define RXD1_PU_BIT             21
#define TXD0_PU_BIT             20
#define RXD0_PU_BIT             19

/* Clock control register */
#define CLK_CTRL_REG1			REG_PA_VAL(AK_PA_SYSCTRL, 0x1C) //0x0800000C
#define UART0_CLKEN_BIT			7
#define UART1_CLKEN_BIT			8

/*********** Shared pin control reigsters ********/
#define SRDPIN_CTRL1_REG     	REG_PA_VAL(AK_PA_SYSCTRL, 0x74) //0x08000074
#define UART0_RXD				14
#define UART0_TXD				15
#define UART1_RXD				16
#define UART1_TXD				18
#define UART1_CTS				20
#define UART1_RTS				22

/** ************ UART registers *****************************/
#define UART0_CONF1_REG			REG_PA_VAL(0x20130000, 0x00) //0x20130000
#define UART0_CONF2_REG			REG_PA_VAL(0x20130000, 0x04)
#define UART0_DATA_CONF_REG		REG_PA_VAL(0x20130000, 0x08)
#define UART0_BUF_THRE_REG		REG_PA_VAL(0x20130000, 0x0C)
#define UART0_BUF_RX_REG		REG_PA_VAL(0x20130000, 0x10)
#define UART0_BUF_RX_BACKUP_REG	REG_PA_VAL(0x20130000, 0x14)
#define UART0_BUF_STOPBIT_REG	REG_PA_VAL(0x20130000, 0x18)

#define UART1_CONF1_REG			REG_PA_VAL(0x20138000, 0x00) //0x20138000
#define UART1_CONF2_REG			REG_PA_VAL(0x20138000, 0x04)
#define UART1_DATA_CONF_REG		REG_PA_VAL(0x20138000, 0x08)
#define UART1_BUF_THRE_REG		REG_PA_VAL(0x20138000, 0x0C)
#define UART1_BUF_RX_REG		REG_PA_VAL(0x20138000, 0x10)
#define UART1_BUF_RX_BACKUP_REG	REG_PA_VAL(0x20138000, 0x14)
#define UART1_BUF_STOPBIT_REG	REG_PA_VAL(0x20138000, 0x18)

/* bit define of UARTx_CONF1_REG */
#define CTS_SEL_BIT             18
#define RTS_SEL_BIT             19
#define PORT_ENABLE_BIT         21  //0: disable, 1:enable
#define TX_STATUS_CLR_BIT       28
#define RX_STATUS_CLR_BIT       29

/* bit define of UARTx_CONF2_REG */
#define TX_COUNT_BIT            4
#define TX_COUNT_VALID_BIT      16
#define TX_END_BIT              19
#define TX_END_MASK             (1 << TX_END_BIT)

#if defined CONFIG_UART0_INIT
#define UART_TXBUF_CLR_BIT      UART0_TXBUF_CLR_BIT
#define SRDPIN_UART_RXTX_BIT    ((1 << UART0_RXD)|(1 << UART0_RXD))
#define RXD_PU_BIT              RXD0_PU_BIT
#define TXD_PU_BIT              TXD0_PU_BIT
#define UART_CLKEN_BIT			UART0_CLKEN_BIT
#define UART_TXBUF_ADDR         UART0_TXBUF_ADDR
#define UART_CONF1_REG          UART0_CONF1_REG
#define UART_CONF2_REG          UART0_CONF2_REG
#define UART_DATA_CONF_REG      UART0_DATA_CONF_REG
#define UART_BUF_STOPBIT_REG	UART0_BUF_STOPBIT_REG
#elif defined CONFIG_UART1_INIT
#define UART_TXBUF_CLR_BIT      UART1_TXBUF_CLR_BIT
#define SRDPIN_UART_RXTX_BIT    ((0x2 << UART1_RXD)|(0x2 << UART1_RXD))
#define RXD_PU_BIT              RXD1_PU_BIT
#define TXD_PU_BIT              TXD1_PU_BIT
#define UART_CLKEN_BIT			UART1_CLKEN_BIT
#define UART_TXBUF_ADDR         UART1_TXBUF_ADDR
#define UART_CONF1_REG          UART1_CONF1_REG
#define UART_CONF2_REG          UART1_CONF2_REG
#define UART_DATA_CONF_REG      UART1_DATA_CONF_REG
#define UART_BUF_STOPBIT_REG	UART1_BUF_STOPBIT_REG

#else
#error One of UART0 ~ UART1 Must be defined
#endif

static unsigned int __uidiv(unsigned int num, unsigned int den)
{
	unsigned int i;

	if (den == 1)
		return num;

	i = 1;
	while (den * i < num)
		i++;

	return i-1;
}

static unsigned long __get_asic_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long asic_pll_clk;
	unsigned long regval;

	regval = REG32(CLK_ASIC_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	asic_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od)); // clk unit: MHz

	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		 && ((pll_m >= 84) && (pll_m <= 254)))
		return asic_pll_clk;
	return 0;
}

static unsigned long __get_vclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = REG32(CLK_ASIC_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;
	if (div == 0)
		return __get_asic_pll_clk() >> 1;
	
	return __get_asic_pll_clk() >> div;
}

unsigned long __get_asic_clk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = REG32(CLK_ASIC_PLL_CTRL);
	div = regval & (1 << 24);
	if (div == 0) 
		return __get_vclk();
	
	return __get_vclk() >> 1;
}

static void uart_init(void)
{
	unsigned int asic_clk, clk_div;

	/* enable uart clock control */
	CLK_CTRL_REG1 &= ~(0x1 << UART_CLKEN_BIT);

	/* configuration shared pins to UART */
	SRDPIN_CTRL1_REG |= SRDPIN_UART_RXTX_BIT;

	/* configuration uart pin pullup disable */
	PPU_PPD1_REG |= (0x1 << RXD_PU_BIT) | (0x1 << TXD_PU_BIT);

	asic_clk = __get_asic_clk()*1000000;
	clk_div = __uidiv(asic_clk, BAUD_RATE) - 1;
	UART_CONF1_REG &= ~((0x1 << TX_STATUS_CLR_BIT) | (0x1 << RX_STATUS_CLR_BIT) | 0xFF);
	UART_CONF1_REG |= (0x1 << TX_STATUS_CLR_BIT) | (0x1 << RX_STATUS_CLR_BIT) | clk_div; 
	
#ifdef CONFIG_UART1_INIT
	/* Disable flow control */
	UART_CONF1_REG |= (0x1 << CTS_SEL_BIT) | (0x1 << RTS_SEL_BIT);
#endif
	UART_BUF_STOPBIT_REG = (0x1F << 16) | (0x1 << 0);

	/* enable uart port */
	UART_CONF1_REG |= (0x1 << PORT_ENABLE_BIT);
}


/* print a char to uart */
static void putc(char c)
{
	/* Clear uart tx buffer */
	L2BUF_CONF2_REG   |= (0x1 << UART_TXBUF_CLR_BIT);

	/* write char to uart buffer */
	REG32(UART_TXBUF_ADDR) = (unsigned long)c;
	REG32(UART_TXBUF_ADDR + ENDDING_OFFSET) = (unsigned long)'\0';
	
	/* Clear uart tx count register */
	UART_CONF1_REG |= (0x1 << TX_STATUS_CLR_BIT);

	/* Send buffer, each time only send 1 byte */
	UART_CONF2_REG |= (1 << TX_COUNT_BIT) | (0x1 << TX_COUNT_VALID_BIT);

	/* Wait for finish */
	while((UART_CONF2_REG & TX_END_MASK) == 0) {
	}
}

static inline void flush(void)
{
}

static inline void arch_decomp_setup(void)
{
	uart_init();
}

/* nothing to do */
#define arch_decomp_wdog()

#endif   /* __UNCOMPRESS_H_ */
