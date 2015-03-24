/*
 * arch/arm/mach-ak39/irq.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/irq.h>

#include <mach/gpio.h>


/* interrupt mask	0: mask	1: unmask */
#define AK_IRQ_MASK				(AK_VA_SYSCTRL + 0x24)
#define AK_FIQ_MASK				(AK_VA_SYSCTRL + 0x28)
#define AK_INT_STATUS			(AK_VA_SYSCTRL + 0x4C)
#define	AK_SYSCTRL_INT_MASK		(AK_VA_SYSCTRL + 0x2C)
#define	AK_SYSCTRL_INT_STATUS	(AK_VA_SYSCTRL + 0x30)

#define AK_L2MEM_IRQ_ENABLE		(AK_VA_L2CTRL + 0x9C)


#ifdef CONFIG_FIQ
/**
 * s3c24xx_set_fiq - set the FIQ routing
 * @irq: IRQ number to route to FIQ on processor.
 * @on: Whether to route @irq to the FIQ, or to remove the FIQ routing.
 *
 * Change the state of the IRQ to FIQ routing depending on @irq and @on. If
 * @on is true, the @irq is checked to see if it can be routed and the
 * interrupt controller updated to route the IRQ. If @on is false, the FIQ
 * routing is cleared, regardless of which @irq is specified.
 */
int ak39_set_fiq(unsigned int irq, bool on)
{
	u32 intmod;
	unsigned offs;
	unsigned long regval;

	regval = __raw_readl(AK_IRQ_MASK);
	if (on) 		
		regval |= (1UL << d->irq);
	else 
		regval &= ~(1UL << d->irq);
	__raw_writel(regval, AK_IRQ_MASK);

#if 0	
	if (on) {
		offs = irq - FIQ_START;
		if (offs > 31)
			return -EINVAL;

		intmod = 1 << offs;
	} else {
		intmod = 0;
	}
	__raw_writel(intmod, AK_FIQ_MASK);
#endif
	
	return 0;
}

EXPORT_SYMBOL_GPL(ak39_set_fiq);
#endif

/*
 * Disable interrupt number "irq"
 */
static void ak39_mask_irq(struct irq_data *d)
{
	unsigned long regval;

	regval = __raw_readl(AK_IRQ_MASK);
	regval &= ~(1UL << d->irq);
	__raw_writel(regval, AK_IRQ_MASK);
}

/*
 * Enable interrupt number "irq"
 */
static void ak39_unmask_irq(struct irq_data *d)
{
	unsigned long regval;

	regval = __raw_readl(AK_IRQ_MASK);
	regval |= (1UL << d->irq);
	__raw_writel(regval, AK_IRQ_MASK);
}

static struct irq_chip ak39_irq_chip = {
	.name = "module-irq",
	.irq_mask_ack = ak39_mask_irq,
	.irq_mask = ak39_mask_irq,
	.irq_unmask = ak39_unmask_irq,
};

static void sysctrl_mask_irq(struct irq_data *d)
{
	unsigned long regval;

	regval = __raw_readl(AK_SYSCTRL_INT_MASK);
	regval &= ~(1 << (d->irq - IRQ_SYSCTRL_START));
	__raw_writel(regval, AK_SYSCTRL_INT_MASK);
}

static void sysctrl_unmask_irq(struct irq_data *d)
{
	unsigned long regval;

	regval = __raw_readl(AK_SYSCTRL_INT_MASK);
	regval |= (1 << (d->irq - IRQ_SYSCTRL_START));
	__raw_writel(regval, AK_SYSCTRL_INT_MASK);
}

/* enable rtc alarm to wake up systerm */
static int sysctrl_set_wake(struct irq_data *d, unsigned int on)
{
#if 0	// wait for programming

	unsigned long clkdiv1;

	if (d->irq == IRQ_RTC_ALARM) {

		clkdiv1 = __raw_readl(AK98_CLKDIV1);

		if (on == 1)
			clkdiv1 |= (1 << 16);
		else
			clkdiv1 &= ~(1 << 16);

		__raw_writel(clkdiv1, AK98_CLKDIV1);

		return 0;
	}
#endif
	return 0;
}

static struct irq_chip ak39_sysctrl_chip = {
	.name = "sysctrl-irq",
	.irq_mask_ack = sysctrl_mask_irq,
	.irq_mask = sysctrl_mask_irq,
	.irq_unmask = sysctrl_unmask_irq,
	.irq_set_wake = sysctrl_set_wake,
};

/**
 * @brief: system module irq handler
 * 
 * @author: caolianming
 * @param [in] irq: irq number
 * @param [in] *desc: irq info description
 */
static void ak39_sysctrl_handler(unsigned int irq, struct irq_desc *desc)
{
	unsigned long regval_mask, regval_sta;
	unsigned long intpnd;
	unsigned int offset;

	regval_mask = __raw_readl(AK_SYSCTRL_INT_MASK);
	regval_sta = __raw_readl(AK_SYSCTRL_INT_STATUS);
	
	intpnd = (regval_mask & 0x7FF) & (regval_sta & 0x7FF);

	for (offset = 0; intpnd && offset < 11; offset++) {

		if (intpnd & (1 << offset))
			intpnd &= ~(1 << offset);
		else
			continue;

		irq = AK39_SYSCTRL_IRQ(offset);		//come back debug
		generic_handle_irq(irq);
	}
}

/**
 * @brief: mask GPIO irq
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *d: system irq data info
 */
static void ak39_gpioirq_mask(struct irq_data *d)
{
	void __iomem *gpio_ctrl = AK_GPIO_INT_MASK1;
	unsigned long regval;
	unsigned int irq = d->irq;

	irq -= IRQ_GPIO_0;
	gpio_ctrl += (irq / 32) * 4;

	regval = __raw_readl(gpio_ctrl);
	regval &= ~(1 << (irq & 31));
	__raw_writel(regval, gpio_ctrl);
}

/**
 * @brief: unmask GPIO irq
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *d: system irq data info
 */
static void ak39_gpioirq_unmask(struct irq_data *d)
{
	void __iomem *gpio_ctrl = AK_GPIO_INT_MASK1;
	unsigned long regval;
	unsigned int irq = d->irq;
	
	irq -= IRQ_GPIO_0;
	gpio_ctrl += (irq / 32) * 4;

	regval = __raw_readl(gpio_ctrl);
	regval |= (1 << (irq & 31));
	__raw_writel(regval, gpio_ctrl);
}


/**
 * @brief: setting irq polarity type
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *d: system irq data info
 * @param [in] type: irq type: IRQ_TYPE_EDGE_RISING, IRQ_TYPE_EDGE_FALLING
 *                            IRQ_TYPE_LEVEL_HIGH, IRQ_TYPE_LEVEL_LOW
 */
static int ak39_gpioirq_set_type(struct irq_data *d, unsigned int type)
{
	void __iomem *reg_irqmod = AK_GPIO_INT_MODE1;
	void __iomem *reg_irqpol = AK_GPIO_INTP1;
	unsigned int irq = d->irq;
	unsigned long regval_pol, regval_mod, offset;
	int p, l;

	irq -= IRQ_GPIO_0;

    offset = irq & 31;

	reg_irqmod += (irq / 32) * 4;	
	reg_irqpol += (irq / 32) * 4;

	regval_mod = __raw_readl(reg_irqmod);
	regval_pol = __raw_readl(reg_irqpol);
	
	switch (type) {
		case IRQ_TYPE_EDGE_RISING:
			p = 1; l = 0; break;
		case IRQ_TYPE_EDGE_FALLING:
			p = 1; l = 1; break;
		case IRQ_TYPE_LEVEL_HIGH:
			p = 0; l = 0; break;
		case IRQ_TYPE_LEVEL_LOW:
			p = 0; l = 1; break;
		default:
			pr_debug("%s: Incorrect GPIO interrupt type 0x%x\n",
					__func__, type);
			return -ENXIO;
	}

	if (p)
		regval_mod |= (1 << (offset));
	else 
		regval_mod &= ~(1 << (offset));	
	if (l) 
		regval_pol |= (1 << (offset));
	else
		regval_pol &= ~(1 << (offset));
		
	__raw_writel(regval_mod, reg_irqmod);
	__raw_writel(regval_pol, reg_irqpol);

	return 0;
}

/**
 * @brief: setting wake up function of irq
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *d: system irq data info
 * @param [in] on: enable
 */
static int ak39_gpio_irq_set_wake(struct irq_data *d, unsigned int on)
{
	unsigned long regval;

	regval = __raw_readl(AK_WGPIO_ENABLE);

	if (d->irq >= IRQ_GPIO_0 && d->irq <= IRQ_GPIO_7)
		regval |= (1 << (d->irq - IRQ_GPIO_0));

	else if (d->irq>= IRQ_GPIO_12 && d->irq <= IRQ_GPIO_14)
		regval |= (1 << (d->irq - IRQ_GPIO_12 + 8));
	
	else if (d->irq == IRQ_GPIO_22)
		regval |= (1 << (d->irq - IRQ_GPIO_22 + 11));
	
	else if (d->irq >= IRQ_GPIO_27 && d->irq <= IRQ_GPIO_30)
		regval |= (1 << (d->irq - IRQ_GPIO_27 + 12));

	else if (d->irq >= IRQ_GPIO_39 && d->irq <= IRQ_GPIO_44)
		regval |= (1 << (d->irq- IRQ_GPIO_41 + 16));

	else if (d->irq >= IRQ_GPIO_47 && d->irq <= IRQ_GPIO_55)
		regval |= (1 << (d->irq - IRQ_GPIO_47 + 22));
	
	else if (d->irq == IRQ_GPIO_57)
		regval |= (1 << (d->irq - IRQ_GPIO_57 + 31));
	
	else {
		printk("Not WGPIO IRQ: %d\n", d->irq);
		return -1;
	}

	__raw_writel(regval, AK_WGPIO_ENABLE);

	return 0;
}

static struct irq_chip ak39_gpioirq_chip = {
	.name = "gpio-irq",
	.irq_mask_ack = ak39_gpioirq_mask,
	.irq_mask = ak39_gpioirq_mask,
	.irq_unmask = ak39_gpioirq_unmask,
	.irq_set_type = ak39_gpioirq_set_type,
	.irq_set_wake = ak39_gpio_irq_set_wake,
};


/**
 * @brief: GPIO irq handler function.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] irq: irq number
 * @param [in] *desc: irq info description
 */
static void ak39_gpio_irqhandler(unsigned int irq, struct irq_desc *desc)
{
	unsigned long enabled_irq;
	unsigned int i;
	unsigned int off;

	for (i = 0; i < 4; i++) {
		enabled_irq = __raw_readl(AK_GPIO_INT_MASK1 + i * 4);

		while (enabled_irq) {
			off = __ffs(enabled_irq);
			enabled_irq &= ~(1 << off);
			if (test_bit(off, AK_GPIO_INTP1 + i * 4) !=
			    test_bit(off, AK_GPIO_INPUT1 + i * 4)) {
				irq = IRQ_GPIO_0 + i * 32 + off;
				generic_handle_irq(irq);
			}
		}
	}
}


/**
 * @brief: machine interrupt initial
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] void:
 */
void __init ak39_init_irq(void)
{
	int i;
	
	/* 1st, clear all interrupts */
	__raw_readl(AK_INT_STATUS);
	__raw_readl(AK_SYSCTRL_INT_STATUS);

	/* 2nd, mask all interrutps */
	__raw_writel(0x0, AK_IRQ_MASK);
	__raw_writel(0x0, AK_FIQ_MASK);
	__raw_writel(0x0, AK_SYSCTRL_INT_MASK);

	/* mask all gpio interrupts */
	__raw_writel(0x0, AK_GPIO_INT_MASK1);
	__raw_writel(0x0, AK_GPIO_INT_MASK2);

	/* mask all l2 interrupts */
	__raw_writel(0x0, AK_L2MEM_IRQ_ENABLE);

	for (i = IRQ_MEM; i <= IRQ_USBOTG_DMA; i++) {
		irq_set_chip_and_handler(i, &ak39_irq_chip, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	irq_set_chained_handler(IRQ_SYSCTRL, ak39_sysctrl_handler);

	for (i = IRQ_SARADC; i <= IRQ_RTC_WATCHDOG; i++) {
		irq_set_chip_and_handler(i, &ak39_sysctrl_chip, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	irq_set_chained_handler(IRQ_GPIO, ak39_gpio_irqhandler);

	for (i = IRQ_GPIO_0; i < NR_IRQS; i++) {
		irq_set_chip_and_handler(i, &ak39_gpioirq_chip, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
}

