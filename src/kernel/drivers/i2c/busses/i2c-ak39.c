/* linux/drivers/i2c/busses/i2c-ak39.c
 *
 * Copyright (C) 2010 Anyka
 *
 * AK39xx I2C Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/semaphore.h>

#include <asm/irq.h>
#include <mach/clock.h>
#include <asm/gpio.h>
#include <mach/i2c.h>
#include <mach/reset.h>

//#define AK39_I2C_INTERRUPT_MODE
#define AK39_I2C_POLL_MODE

/* i2c controller state */
enum ak39_i2c_state {
	STATE_READ,
	STATE_WRITE
};

enum read_data_addr {
	READ_ADDR,
	READ_DATA
};

struct ak39_i2c {
	spinlock_t		lock;
	wait_queue_head_t	wait;
	unsigned int		suspended:1;

	struct i2c_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;

	unsigned int		irq;
	unsigned long		clkrate;

	enum ak39_i2c_state	state;
	enum read_data_addr	read_value;
	
	void __iomem		*regs;
	struct clk			*clk;
	struct device		*dev;
	struct resource		*ioarea;
	struct i2c_adapter	adap;

#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif
};

static struct semaphore xfer_sem;

/* ~~~~~~~~~~~ poll mode ~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef AK39_I2C_POLL_MODE

/*
 * Poll the i2c status register until the specified bit is set.
 * Return 1 if transfer finished.
 */
static short ak39_poll_status(void)
{
	do {
	} while (!(__raw_readl(AK39_I2C_CTRL) & INT_PEND_FLAG));

	return 1;
}

static int xfer_read(struct ak39_i2c *i2c, unsigned char *buf, int length)
{
	int i,j, ctrl_value;
	unsigned long ret, reg_value;
	unsigned char *p = buf;
	int idx = 0;

	if((length - 1) > 15)
		ctrl_value = 16;
	else
		ctrl_value = length;

	ret = __raw_readl(AK39_I2C_CTRL);
	ret |= (AK39_I2C_START | AK39_I2C_ACKEN | AK39_I2C_TXRXSEL | AK39_I2C_CLR_DELAY);
	ret &= ~((0xf) << AK39_I2C_TRX_BYTE);
	ret |= ((ctrl_value - 1) << AK39_I2C_TRX_BYTE);
	ret &= ~(INT_PEND_FLAG);
	__raw_writel(ret, AK39_I2C_CTRL);

	
	for (i = 0; i < length / 16; i++) {
		if (!ak39_poll_status()) {
			dev_dbg(&i2c->adap.dev, "RXRDY timeout\n");
			return -ETIMEDOUT;
		}

		for (j = 0; j < 4; j++) {
			reg_value = __raw_readl(AK39_I2C_DATA0 + j * 4);			
			*(unsigned long *)p = reg_value;
			p +=4;
		}
	}

	if (!ak39_poll_status()) {
		dev_dbg(&i2c->adap.dev, "RXRDY timeout\n");
		return -ETIMEDOUT;
	}

	idx = 0;
	for (; idx < (length % 16) / 4; idx++) {
		unsigned long regval = __raw_readl(AK39_I2C_DATA0 + idx * 4);
		*(unsigned long *)p = regval;
		p +=4;
	}

	if (length % 4) {
		unsigned long regval;
		if (!ak39_poll_status()) {
			dev_dbg(&i2c->adap.dev, "RXRDY timeout\n");
			return -ETIMEDOUT;
		}
		
		regval = __raw_readl(AK39_I2C_DATA0 + idx / 4);
		for (i = 0; i < length % 4; i++) {
			*p++ = (regval >> (i * 8)) & 0xFF;
		}
	}

	return 0;
}

static void wait_xfer_write(struct ak39_i2c *i2c, int length)
{
	unsigned long ret;
	
	ret = __raw_readl(AK39_I2C_CTRL);
	ret |= (AK39_I2C_START);
	ret &= ~(AK39_I2C_TXRXSEL | INT_PEND_FLAG);
	ret &= ~((0xf) << AK39_I2C_TRX_BYTE);
	ret |= ((length - 1)<< AK39_I2C_TRX_BYTE);
	__raw_writel(ret, AK39_I2C_CTRL);

	if (!ak39_poll_status()) {
		dev_info(&i2c->adap.dev, "TXRDY timeout\n");
		return ;
	}
}

static int xfer_write(struct ak39_i2c *i2c, unsigned char *buf, int length)
{	
	unsigned int i;
	unsigned long reg_value;
	unsigned int num_count, num_char;
	int ctrl_value, num_bck = length;
	unsigned char *p = buf;
	
	if((length - 1) > 15)
		ctrl_value = 16;
	else
		ctrl_value = length;
	
	num_count = num_bck / 16;
	num_char = num_bck % 16;
	
	while(num_count--) {
		for(i = 0; i < 4; i++) {			
			reg_value = *(unsigned long *)p;
			p += 4;
   			__raw_writel(reg_value, AK39_I2C_DATA0 + i * 4);
		}
		
		wait_xfer_write(i2c, 16);
	}
    if(num_char > 0) {
        unsigned long value = 0;
        for(i = 0; i < num_char; i+=4) {
			value = *(unsigned long *)p;
			p +=4;
			__raw_writel(value, AK39_I2C_DATA0 + i);
		}
		wait_xfer_write(i2c, num_char);
	}
	
	return 0;
}

static int ak39_i2c_doxfer(struct ak39_i2c *i2c, struct i2c_msg *msgs, int num)
{
	unsigned int addr = (msgs->addr & 0x7f) << 1;
	int i, ret, rw_flag;

	dev_dbg(&i2c->adap.dev, "ak39_i2c_doxfer: processing %d messages:\n", num);
	
	for (i = 0; i < num; i++) {
		if (msgs->flags & I2C_M_RD) {
			addr |= AK39_I2C_READ;
			rw_flag = 1;
		} else {
			rw_flag = 0;
		}
		if (msgs->flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;

		__raw_writel((AK39_I2C_CMD_EN | AK39_I2C_START_BIT)|addr, AK39_I2C_CMD1);
		if(msgs->len && msgs->buf) {
			if (rw_flag == 1)
				ret = xfer_read(i2c, msgs->buf, msgs->len);
			else
				ret = xfer_write(i2c, msgs->buf, msgs->len);
			
			if (ret)
				return ret;
		}
		dev_dbg(&i2c->adap.dev, "transfer complete\n");
		msgs++;		/* next message */
	}
	return i;
}
#endif

/* ~~~~~~~~~~~ INTER MODE ~~~~~~~~~~~~~~~~~~~ */

#ifdef AK39_I2C_INTERRUPT_MODE 

static void clear_int_flag(void)
{
	unsigned long tmp;

	tmp = __raw_readl(AK39_I2C_CTRL);
	tmp &= ~INT_PEND_FLAG;
	__raw_writel(tmp, AK39_I2C_CTRL);
}

/* irq disable functions */
static void ak39_i2c_disable_irq(struct ak39_i2c *i2c)
{
	unsigned long tmp;
	
	tmp = __raw_readl(AK39_I2C_CTRL);
	__raw_writel(tmp & ~AK39_I2C_INTEN, AK39_I2C_CTRL);
}

static inline void ak39_i2c_master_complete(struct ak39_i2c *i2c, int ret)
{
	dev_dbg(i2c->dev, "master_complete %d\n", ret);

	i2c->msg_ptr = 0;
	i2c->msg = NULL;
	i2c->msg_num = 0;
	if (ret)
		i2c->msg_idx = ret;
	
	wake_up(&i2c->wait);
}

static inline void ak39_i2c_stop(struct ak39_i2c *i2c, int ret)
{
	dev_dbg(i2c->dev, "STOP\n");

	ak39_i2c_master_complete(i2c, ret);
	ak39_i2c_disable_irq(i2c);
}

static int ak39_i2c_irq_transfer(struct ak39_i2c *i2c)
{
	unsigned int addr = (i2c->msg->addr & 0x7f) << 1;
	unsigned int num_char, i, length;
	unsigned long stat, regval = 0;
	unsigned char *p = i2c->msg->buf;

read_next:
	if(i2c->msg_idx < i2c->msg_num) {
		if (i2c->msg->len == 0) {
			ak39_i2c_stop(i2c, 0);
			return 0;
		}
		
		stat = __raw_readl(AK39_I2C_CTRL);
		stat &= ~(0xf << 9);
		if (i2c->msg->flags & I2C_M_RD) {
			addr |= AK39_I2C_READ ;
			i2c->state = STATE_READ ;
			stat |= AK39_I2C_TXRXSEL ;
		} 
		else {
			i2c->state = STATE_WRITE ;
			stat &= ~AK39_I2C_TXRXSEL ;
		}
	
		if (i2c->msg->flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;
		
		__raw_writel((AK39_I2C_CMD_EN | AK39_I2C_START_BIT)|addr, AK39_I2C_CMD1);
		
		switch(i2c->state) {
			case STATE_WRITE:
				
				if (i2c->msg->len > 16) {
						printk("Error, needed debug more data transmitted.\n");
						return 0;
				}
				if (i2c->msg_ptr < i2c->msg->len) {
					
					num_char = i2c->msg->len % 16;
					
					if (num_char > 0) {
						__raw_writel(stat | ((num_char - 1) << 9), AK39_I2C_CTRL);
						for(i = 0; i < num_char; i+=4) {
							regval = *(unsigned long *)p;
							p += 4;
							__raw_writel(regval, AK39_I2C_DATA0 + i);
						}
						i2c->msg_ptr += num_char;			
					}
					
					if(i2c->msg_ptr >= i2c->msg->len){
						i2c->msg_ptr = 0;
						i2c->msg_idx++;
						i2c->msg++;
					}
					
					stat = __raw_readl(AK39_I2C_CTRL);
					__raw_writel(stat | AK39_I2C_START, AK39_I2C_CTRL);				
				}
				break;

			case STATE_READ:
				length = i2c->msg->len;
				
				if (i2c->msg->len > 16) {
					printk("Error, needed debug more data transmitted.\n");
					return 0;
				}
				__raw_writel(stat | ((length - 1) << 9), AK39_I2C_CTRL);
				
				if(i2c->read_value == READ_ADDR) {
					stat = __raw_readl(AK39_I2C_CTRL);
					__raw_writel(stat | AK39_I2C_START, AK39_I2C_CTRL);
					i2c->read_value = READ_DATA;
				} else {
					if (i2c->msg_ptr < i2c->msg->len) {

						num_char = i2c->msg->len % 16;

						if (num_char > 0) {
							for (i = 0; i < num_char; i+=4) {
								regval = __raw_readl(AK39_I2C_DATA0 + i);
								*(unsigned long *)p = regval;
								p += 4;
							}
							i2c->msg_ptr += num_char;
						}
						
						if (i2c->msg_ptr >= i2c->msg->len) {
							/* we need to go to the next i2c message */
							dev_dbg(i2c->dev, "READ: Next Message\n");
											
							i2c->msg_ptr = 0;
							i2c->msg_idx++;
							i2c->msg++;
							
							if(i2c->msg_idx >= i2c->msg_num) {
								ak39_i2c_stop(i2c, 0);
								return 0;
							}
						}
						i2c->read_value = READ_ADDR;
						goto read_next;
					}							
				}
				break;
		}	
	} 
	else {
		ak39_i2c_stop(i2c, 0);
	}
	return 0;
}

/* ak39_i2c_irq
 *
 * top level IRQ servicing routine
*/
static irqreturn_t ak39_i2c_irq(int irqno, void *dev_id)
{
	struct ak39_i2c *i2c = dev_id;

	clear_int_flag();

	/* pretty much this leaves us with the fact that we've
	 * transmitted or received whatever byte we last sent */
	
	ak39_i2c_irq_transfer(i2c);

	return IRQ_HANDLED;
}

static int ak39_i2c_doxfer(struct ak39_i2c *i2c, struct i2c_msg *msgs, int num)
{
	unsigned long timeout, stat;
	int ret;

	if (i2c->suspended)
		return -EIO;

	spin_lock_irq(&i2c->lock);
	i2c->msg     = msgs;
	i2c->msg_num = num;
	i2c->msg_ptr = 0;
	i2c->msg_idx = 0;
	i2c->read_value = READ_ADDR;

	ak39_i2c_irq_transfer(i2c);
	stat = __raw_readl(AK39_I2C_CTRL);
	stat |= AK39_I2C_INTEN | AK39_I2C_ACKEN;
	__raw_writel(stat, AK39_I2C_CTRL);

	spin_unlock_irq(&i2c->lock);
	timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 5);

	ret = i2c->msg_idx;

	/* having these next two as dev_err() makes life very
	 * noisy when doing an i2cdetect */

	if (timeout == 0)
		dev_info(i2c->dev, "timeout\n");
	else if (ret != num)
		dev_info(i2c->dev, "incomplete xfer (%d)\n", ret);

	return ret;
}
#endif


/* ak39_i2c_xfer
 *
 * first port of call from the i2c bus code when an message needs
 * transferring across the i2c bus.
*/
static int ak39_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	struct ak39_i2c *i2c = (struct ak39_i2c *)adap->algo_data;
	int retry;
	int ret;

	down(&xfer_sem);
	for (retry = 0; retry < adap->retries; retry++) {
	
		ret = ak39_i2c_doxfer(i2c, msgs, num);
	
		if (ret != -EAGAIN)
		{
			up(&xfer_sem);
			return ret;
		}

		dev_dbg(i2c->dev, "Retrying transmission (%d)\n", retry);

		udelay(100);
	}
	
	up(&xfer_sem);
	return -EREMOTEIO;
}

/* declare our i2c functionality */
static u32 ak39_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

/* i2c bus registration info */

static const struct i2c_algorithm ak39_i2c_algorithm = {
	.master_xfer		= ak39_i2c_xfer,
	.functionality		= ak39_i2c_func,
};

/* ak39_i2c_calcdivisor
 *
 * return the divisor settings for a given frequency
*/
static int ak39_i2c_calcdivisor(unsigned long clkin, unsigned int wanted,
				   unsigned int *div1, unsigned int *divs)
{
	unsigned int calc_divs = clkin / wanted / 2;
	unsigned int calc_div1;

	if (calc_divs > (16*16))
		calc_div1 = 512;
	else
		calc_div1 = 16;

	calc_divs /= calc_div1;

	if (calc_divs == 0)
		calc_divs = 1;
	if (calc_divs > 16)
		calc_divs = 16;

	*divs = calc_divs;
	*div1 = calc_div1;

	return clkin / ((calc_divs * 2)* calc_div1);
}

/* ak39_i2c_clockrate
 *
 * work out a divisor for the user requested frequency setting,
 * either by the requested frequency, or scanning the acceptable
 * range of frequencies until something is found
*/
static int ak39_i2c_clockrate(struct ak39_i2c *i2c, unsigned int *got)
{
	struct ak39_platform_i2c *pdata = i2c->dev->platform_data;
	unsigned long clkin = ak_get_asic_clk();
	unsigned int divs, div1;
	unsigned long target_frequency;
	u32 i2c_val, sda_delay;
	int freq;

	i2c->clkrate = clkin;
	clkin /= 1000;		/* clkin now in KHz */
	
	dev_dbg(i2c->dev, "pdata desired frequency %lu\n", pdata->frequency);

	target_frequency = pdata->frequency ? pdata->frequency : 100000;
	//target_frequency *= 4;
	target_frequency /= 1000; /* Target frequency now in KHz */

	freq = ak39_i2c_calcdivisor(clkin, target_frequency, &div1, &divs);
	if (freq > target_frequency) {
		dev_err(i2c->dev,
			"Unable to achieve desired frequency %luKHz."	\
			" Lowest achievable %dKHz\n", target_frequency, freq);
		// return -EINVAL;
	}

	*got = freq;

	i2c_val = __raw_readl(AK39_I2C_CTRL);
	i2c_val &= ~(AK39_TX_CLK_DIV | AK39_I2C_TXDIV_512);
	i2c_val |= (divs-1);

	if (div1 == 512)
		i2c_val |= AK39_I2C_TXDIV_512;
	
	__raw_writel(i2c_val, AK39_I2C_CTRL);

	if (pdata->sda_delay) {
		sda_delay = (freq / 1000) * pdata->sda_delay;
		sda_delay /= 1000000;
		sda_delay = DIV_ROUND_UP(sda_delay, 5);
		if (sda_delay > 3)
			sda_delay = AK39_I2C_CLR_DELAY;
		
		sda_delay |= AK39_I2C_SDA_DELAY;
	} else
		sda_delay = ~AK39_I2C_CLR_DELAY;

	i2c_val |= sda_delay;
	__raw_writel(i2c_val, AK39_I2C_CTRL);

	return 0;
}

#ifdef CONFIG_CPU_FREQ

#define freq_to_i2c(_n) container_of(_n, struct ak39_i2c, freq_transition)

static int ak39_i2c_cpufreq_transition(struct notifier_block *nb,
					  unsigned long val, void *data)
{
	struct ak39_i2c *i2c = freq_to_i2c(nb);
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;
	unsigned int old_clk = freqs->old_cpufreq.asic_clk;
	unsigned int new_clk = freqs->new_cpufreq.asic_clk;
	unsigned long flags;
	unsigned int got;
	int ret;

	if (val == CPUFREQ_PRECHANGE)
	{
		down(&xfer_sem);
	}
	else if (val == CPUFREQ_POSTCHANGE)
	{	
		if (old_clk != new_clk)
		{
			spin_lock_irqsave(&i2c->lock, flags);
			ret = ak39_i2c_clockrate(i2c, &got);
			//printk("setting freq %u\n", got);
			spin_unlock_irqrestore(&i2c->lock, flags);

			if (ret < 0)
				dev_err(i2c->dev, "cannot find frequency\n");
			else
				dev_info(i2c->dev, "setting freq %d\n", got);
			
		}
		up(&xfer_sem);
	}
	
	return 0;
}

static inline int ak39_i2c_register_cpufreq(struct ak39_i2c *i2c)
{
	i2c->freq_transition.notifier_call = ak39_i2c_cpufreq_transition;
	return cpufreq_register_notifier(&i2c->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void ak39_i2c_deregister_cpufreq(struct ak39_i2c *i2c)
{
	cpufreq_unregister_notifier(&i2c->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}
#else
static inline int ak39_i2c_register_cpufreq(struct ak39_i2c *i2c)
{
	return 0;
}
static inline void ak39_i2c_deregister_cpufreq(struct ak39_i2c *i2c)
{
}
#endif

/* ak39_i2c_init
 *
 * initialise the controller, set the IO lines and frequency
*/
static int ak39_i2c_init(struct ak39_i2c *i2c)
{
	struct ak39_platform_i2c *pdata;
	unsigned int freq ;

	/*
	 * Reset I2C Controller
	 */
	ak39_soft_reset(AK39_SRESET_I2C);

	/* get the plafrom data */
	pdata = i2c->dev->platform_data;

	/* inititalise the gpio */
	ak_group_config(ePIN_AS_I2C);

	__raw_writel(AK39_I2C_INTEN | AK39_I2C_ACKEN, AK39_I2C_CTRL);
	
	/* we need to work out the divisors for the clock... */
	if (ak39_i2c_clockrate(i2c, &freq) != 0) {
		__raw_writel(0, AK39_I2C_CTRL);
		dev_err(i2c->dev, "cannot meet bus frequency required\n");
		return -EINVAL;
	}

	/* todo - check that the i2c lines aren't being dragged anywhere */

	dev_dbg(i2c->dev, "bus frequency set to %d KHz\n", freq);
	return 0;
}


/* ak39_i2c_probe
 *
 * called by the bus driver when a suitable device is found
*/
static int ak39_i2c_probe(struct platform_device *pdev)
{
	struct ak39_i2c *i2c;
	struct ak39_platform_i2c *pdata;
	struct resource *res;
	int ret;
	
	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data\n");
		return -EINVAL;
	}

	i2c = kzalloc(sizeof(struct ak39_i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "no memory for state\n");
		return -ENOMEM;
	}		

	strlcpy(i2c->adap.name, "ak39-i2c", sizeof(i2c->adap.name));
	i2c->adap.owner   = THIS_MODULE;
	i2c->adap.algo    = &ak39_i2c_algorithm;
	i2c->adap.retries = 2;
	i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	/* find the clock and enable it */

	i2c->dev = &pdev->dev;
	i2c->clk = clk_get(&pdev->dev, "i2c");
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		ret = -ENOENT;
		goto err_noclk;
	}

	dev_dbg(&pdev->dev, "clock source %p\n", i2c->clk);
		
	clk_enable(i2c->clk);

	/* map the registers */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto err_clk;
	}

	i2c->ioarea = request_mem_region(res->start, resource_size(res), pdev->name);
	if (i2c->ioarea == NULL) {
		dev_err(&pdev->dev, "cannot request IO\n");
		ret = -ENXIO;
		goto err_clk;
	}

	i2c->regs = ioremap(res->start, resource_size(res));
	if (i2c->regs == NULL) {
		dev_err(&pdev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto err_ioarea;
	}

	dev_dbg(&pdev->dev, "registers %p (%p, %p)\n", i2c->regs, i2c->ioarea, res);

	/* setup info block for the i2c core */

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	/* initialise the i2c controller */

	ret = ak39_i2c_init(i2c);
	if (ret != 0)
		goto err_iomap;

	/* find the IRQ for this unit (note, this relies on the init call to
	 * ensure no current IRQs pending
	 */
	i2c->irq = ret = platform_get_irq(pdev, 0);
	if (ret <= 0) {
		dev_err(&pdev->dev, "cannot find IRQ\n");
		goto err_iomap;
	}
	
#ifdef AK39_I2C_INTERRUPT_MODE	
	ret = request_irq(i2c->irq, ak39_i2c_irq, IRQF_DISABLED, dev_name(&pdev->dev), i2c);
	if (ret != 0) {
		dev_err(&pdev->dev, "cannot claim IRQ %d\n", i2c->irq);
		goto err_iomap;
	}
#endif

	sema_init(&xfer_sem, 1);
	ret = ak39_i2c_register_cpufreq(i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register cpufreq notifier\n");
		goto err_irq;
	}
	
	i2c->adap.nr = pdata->bus_num;
	ret = i2c_add_numbered_adapter(&i2c->adap);

	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add bus to i2c core\n");
		goto err_cpufreq;
	}

	platform_set_drvdata(pdev, i2c);

	dev_info(&pdev->dev, "%s: AK39 I2C adapter\n", dev_name(&i2c->adap.dev));
	
	return 0;

 err_cpufreq:
	ak39_i2c_deregister_cpufreq(i2c);

 err_irq:
	free_irq(i2c->irq, i2c);

 err_iomap:
	iounmap(i2c->regs);

 err_ioarea:
	release_resource(i2c->ioarea);
	kfree(i2c->ioarea);

 err_clk:
	clk_disable(i2c->clk);
	clk_put(i2c->clk);

 err_noclk:
	kfree(i2c);
	return ret;
}


/* ak39_i2c_remove
 *
 * called when device is removed from the bus
*/
static int ak39_i2c_remove(struct platform_device *pdev)
{
	struct ak39_i2c *i2c = platform_get_drvdata(pdev);

	ak39_i2c_deregister_cpufreq(i2c);

	i2c_del_adapter(&i2c->adap);
	free_irq(i2c->irq, i2c);

	clk_disable(i2c->clk);
	clk_put(i2c->clk);

	iounmap(i2c->regs);

	release_resource(i2c->ioarea);
	
	kfree(i2c->ioarea);
	kfree(i2c);

	return 0;
}

#ifdef CONFIG_PM
static int ak39_i2c_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ak39_i2c *i2c = platform_get_drvdata(pdev);
	struct ak39_platform_i2c *pdata = pdev->dev.platform_data;
	int i;
	
	down(&xfer_sem);
	i2c->suspended = 1;

	clk_disable(i2c->clk);
	clk_put(i2c->clk);

	for (i=0; i<pdata->npins; i++)
		ak_gpio_set(&(pdata->gpios[i]));

	return 0;
}

static int ak39_i2c_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct ak39_i2c *i2c = platform_get_drvdata(pdev);

	i2c->suspended = 0;
	clk_enable(i2c->clk);
	ak39_i2c_init(i2c);
	up(&xfer_sem);
	return 0;
}

static struct dev_pm_ops ak39_i2c_dev_pm_ops = {
	.suspend_noirq = ak39_i2c_suspend_noirq,
	.resume = ak39_i2c_resume,
};

#define AK39_DEV_PM_OPS (&ak39_i2c_dev_pm_ops)
#else
#define AK39_DEV_PM_OPS NULL
#endif

/* device driver for platform bus bits */
static struct platform_driver ak39_i2c_driver = {
	.probe		= ak39_i2c_probe,
	.remove		= ak39_i2c_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "i2c-ak39",
		.pm	= AK39_DEV_PM_OPS,	
	},
};

static int __init i2c_adap_ak39_init(void)
{
	return platform_driver_register(&ak39_i2c_driver);
}
subsys_initcall(i2c_adap_ak39_init);

static void __exit i2c_adap_ak39_exit(void)
{
	platform_driver_unregister(&ak39_i2c_driver);
}
module_exit(i2c_adap_ak39_exit);

MODULE_DESCRIPTION("AK39 I2C Bus driver");
MODULE_AUTHOR("Anaka, <xxx@xxx.com>");
MODULE_LICENSE("GPL");

