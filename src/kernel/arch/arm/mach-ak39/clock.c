/*
 * linux/arch/arm/mach-ak39/clock.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/err.h>

#include <mach/gpio.h>
#include <mach/clock.h>
#include <linux/delay.h>
#include <plat-anyka/anyka_types.h>


/* clock information */

static LIST_HEAD(clocks);

/* We originally used an mutex here, but some contexts (see resume)
 * are calling functions such as clk_set_parent() with IRQs disabled
 * causing an BUG to be triggered.
 */
DEFINE_SPINLOCK(clocks_lock);

/* enable and disable calls for use with the clk struct */

static int clk_null_enable(struct clk *clk, int enable)
{
	return 0;
}


/**
 * @brief: enable module clock.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 */
int clk_enable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR(clk) || clk == NULL)
		return -EINVAL;
	
	clk_enable(clk->parent);

	spin_lock_irqsave(&clocks_lock, flags);

	if ((clk->usage++) == 0)
		(clk->enable)(clk, 1);

	spin_unlock_irqrestore(&clocks_lock, flags);
	return 0;
}

/**
 * @brief: disable module clock.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 */
void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR(clk) || clk == NULL)
		return;

	spin_lock_irqsave(&clocks_lock, flags);

	if ((--clk->usage) == 0)
		(clk->enable)(clk, 0);

	spin_unlock_irqrestore(&clocks_lock, flags);
	clk_disable(clk->parent);
}

/**
 * @brief: get module clock.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 */
unsigned long clk_get_rate(struct clk *clk)
{
	if (IS_ERR(clk))
		return 0;

	if (clk->rate != 0)
		return clk->rate;

	if (clk->ops != NULL && clk->ops->get_rate != NULL)
		return (clk->ops->get_rate)(clk);

	if (clk->parent != NULL)
		return clk_get_rate(clk->parent);

	return clk->rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (!IS_ERR(clk) && clk->ops && clk->ops->round_rate)
		return (clk->ops->round_rate)(clk, rate);

	return rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret;

	if (IS_ERR(clk))
		return -EINVAL;

	/* We do not default just do a clk->rate = rate as
	 * the clock may have been made this way by choice.
	 */

	WARN_ON(clk->ops == NULL);
	WARN_ON(clk->ops && clk->ops->set_rate == NULL);

	if (clk->ops == NULL || clk->ops->set_rate == NULL)
		return -EINVAL;

	spin_lock(&clocks_lock);
	ret = (clk->ops->set_rate)(clk, rate);
	spin_unlock(&clocks_lock);

	return ret;
}

/**
 * @brief: get parent clock of module.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *clk: struct clk, module clock info
 */
struct clk *clk_get_parent(struct clk *clk)
{
	return clk->parent;
}

/**
 * @brief: set parent clock of module.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 */
int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = 0;

	if (IS_ERR(clk))
		return -EINVAL;

	spin_lock(&clocks_lock);

	if (clk->ops && clk->ops->set_parent)
		ret = (clk->ops->set_parent)(clk, parent);

	spin_unlock(&clocks_lock);

	return ret;
}

EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_round_rate);
EXPORT_SYMBOL(clk_set_rate);
EXPORT_SYMBOL(clk_get_parent);
EXPORT_SYMBOL(clk_set_parent);
/* base clocks */

#if 0
static u32 __power2(u32 x)
{
	u32 s = 1;

	while (x--)
		s <<= 1;

	return s;
}
#endif

static int clk_default_setrate(struct clk *clk, unsigned long rate)
{
	clk->rate = rate;
	return 0;
}


/**
 * @brief: setting module clock of drivers 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] *reg: clock register address of SOC chip
 * @param [in] enable: enable bit
 */
static int inline ak39xx_gate(void __iomem *reg, 
			struct clk *clk, int enable)
{
	unsigned int ctrlbit = clk->ctrlbit;
	u32 con;
	
	con = __raw_readl(reg);
	
	if (enable)
		con &= ~ctrlbit;
	else
		con |= ctrlbit;
	
	__raw_writel(con, reg);
	return 0;
}


/**
 * @brief: setting 12M clock 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_12M_enable(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_PERI_PLL_CTRL1, clk, enable);
}


/**
 * @brief: setting 25M clock 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_25M_enable(struct clk *clk, int enable)
{
	unsigned int ctrlbit = clk->ctrlbit;
	u32 con;

	con = __raw_readl(CLOCK_PERI_PLL_CTRL1);

	if (enable)
		con |= (ctrlbit|AK_CLKCON_CLK_25M_IN);
	else
		con &= ~(ctrlbit|AK_CLKCON_CLK_25M_IN);

	__raw_writel(con, CLOCK_PERI_PLL_CTRL1);
	return 0;
}

struct clk clk_xtal_12M = {
	.name 		= "xtal_12M",
	.usage 		= 0,
	.rate 		= 12 * MHz,
	.parent 	= NULL,
	.enable 	= ak39xx_12M_enable,
	.ctrlbit	= AK_CLKCON_CLK_12M,
};

struct clk clk_xtal_25M = {
	.name		= "xtal_25M",
	.rate		= 25 * MHz,
	.parent		= NULL,
	.enable		= ak39xx_25M_enable,
	.ctrlbit	= AK_CLKCON_CLK_25M,
};

struct clk clk_xtal_32K = {
	.name = "xtal_32K",
	.id = -1,
	.rate = 32768,
	.parent = NULL,
};


struct clk clk_cpu_pll = {
	.name	= "cpu_pll",
	.usage	= 0,
	.rate	= 0,
	.parent	= NULL,
};

struct clk clk_asic_pll = {
	.name	= "asic_pll",
	.usage	= 0,
	.rate	= 0,
	.parent	= NULL,
};

struct clk clk_peri_pll = {
	.name	= "peri_pll",
	.usage	= 0,
	.rate	= 0,
	.parent	= NULL,
};

struct clk clk_cpu = {
	.name	= "cpu_pll",
	.usage	= 0,
	.rate	= 0,
	.parent	= &clk_cpu_pll,
};

/* AHB clock maybe from cpu pll, also maybe vclk[0] */
struct clk clk_ahb = {
	.name	= "ahb_clk",
	.usage	= 0,
	.rate	= 0,
	.parent	= &clk_cpu_pll,
};

struct clk clk_mem = {
	.name	= "mem_clk",
	.usage	= 0,
	.rate	= 0,
	.parent	= &clk_cpu_pll,
};


struct clk clk_vclk = {
	.name		= "vclk",
	.usage		= 0,
	.rate		= 0,
	.parent		= &clk_asic_pll,
};

struct clk clk_asic = {
	.name	= "asic_clk",
	.usage	= 0,
	.rate	= 0,
	.parent = &clk_vclk,
};


/**
 * @brief: setting opclk clock 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_opclk_ctrl(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_PERI_PLL_CTRL2, clk, enable);
}

struct clk clk_opclk = {
	.name		= "clk_opclk",
	.usage		= 0,
	.parent 	= &clk_asic,
	.enable 	= ak39xx_opclk_ctrl,
	.ctrlbit	= AK_CLKCON_ASICCLK_MAC,
};

/**
 * @brief: setting mclk clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_mclk_ctrl(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);
}

/**
 * @brief: setting video module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_video_ctrl(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);
}


/**
 * @brief: setting camera module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_camera_ctrl(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);
}

/**
 * @brief: setting asic clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_asicclk_ctrl(struct clk *clk, int enable)
{
	return ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);
}


/**
 * @brief: setting clk dac module clock 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_clk_dac(struct clk *clk, int enable)
{
	u32 con;

	ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);

	con = __raw_readl(CLOCK_ADC2_DAC_CTRL);

	if (enable)
		con |= AK_CLKCON_CLK_DAC;
	else
		con &= ~AK_CLKCON_CLK_DAC;

	__raw_writel(con, CLOCK_ADC2_DAC_CTRL);
	return 0;
}


/**
 * @brief: setting hclk dac module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_hclk_dac(struct clk *clk, int enable)
{
	u32 con;

	ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);

	con = __raw_readl(CLOCK_ADC2_DAC_HS_CTRL);

	if (enable)
		con |= AK_CLKCON_HCLK_DAC;
	else
		con &= ~AK_CLKCON_HCLK_DAC;

	__raw_writel(con, CLOCK_ADC2_DAC_HS_CTRL);
	return 0;
}

/**
 * @brief: setting adc1 module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] *reg: clock register address of SOC chip
 * @param [in] enable: enable bit
 */
static int ak39xx_clk_adc1(struct clk *clk, int enable)
{
	u32 con;

	ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);

	con = __raw_readl(CLOCK_ADC2_DAC_CTRL);

	if (enable)
		con |= AK_CLKCON_CLK_ADC1;
	else
		con &= ~AK_CLKCON_CLK_ADC1;

	__raw_writel(con, CLOCK_ADC2_DAC_CTRL);
	return 0;
}


/**
 * @brief: setting adc2 module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_clk_adc2(struct clk *clk, int enable)
{
	u32 con;

	ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);

	con = __raw_readl(CLOCK_ADC2_DAC_HS_CTRL);

	if (enable)
		con |= AK_CLKCON_CLK_ADC2;
	else
		con &= ~AK_CLKCON_CLK_ADC2;

	__raw_writel(con, CLOCK_ADC2_DAC_HS_CTRL);
	return 0;
}

/**
 * @brief: setting hclk module clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_hclk_adc2(struct clk *clk, int enable)
{
	u32 con;

	ak39xx_gate(CLOCK_GATE_CTRL1, clk, enable);

	con = __raw_readl(CLOCK_ADC2_DAC_HS_CTRL);

	if (enable)
		con |= AK_CLKCON_HCLK_ADC2;
	else
		con &= ~AK_CLKCON_HCLK_ADC2;

	__raw_writel(con, CLOCK_ADC2_DAC_HS_CTRL);
	return 0;
}

/**
 * @brief: setting sensor module clock 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *clk: struct clk, module clock info
 * @param [in] enable: enable bit
 */
static int ak39xx_sensor_ctrl(struct clk *clk, int enable)
{
	unsigned int ctrlbit = clk->ctrlbit;
	u32 con;
	
	con = __raw_readl(CLOCK_PERI_PLL_CTRL2);
	
	if (enable)
		con |= ctrlbit;
	else
		con &= ~ctrlbit;
	
	__raw_writel(con, CLOCK_PERI_PLL_CTRL2);
	return 0;
}

static struct clk init_clocks[] = {
	{
		.name		= "dram",
		.usage		= 0,
		.parent		= &clk_cpu,
		.enable		= ak39xx_mclk_ctrl,
		.ctrlbit	= AK_CLKCON_MCLK_DRAM,
	}, {
		.name		= "video",
		.usage		= 0,
		.parent		= &clk_vclk,
		.enable		= ak39xx_video_ctrl,
		.ctrlbit	= AK_CLKCON_VCLK2_VIDEO,
	}, {
		.name		= "camera",
		.usage		= 0,
		.parent		= &clk_vclk,
		.enable		= ak39xx_camera_ctrl,
		.ctrlbit	= AK_CLKCON_VCLK1_CAMERA,
	}, {
		.name		= "usb-host",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_USB,
	}, {
		.name		= "usb-slave",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_USB,
	}, {
		.name		= "encryption",
		.usage		= 0,			
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_ENCRY,
	}, {
		.name		= "mac",
		.usage		= 0,
		.parent		= &clk_opclk,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_MAC,		
	}, {
		.name		= "gpio",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_GPIO,
	}, {
		.name		= "IrDA",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_IRDA,
	}, {
		.name		= "i2c",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_I2C,
	}, {
		.name		= "l2mem",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_L2MEM,
	}, {
		.name		= "uart1",
		.devname	= "ak39xx-uart.1",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_UART2,
	}, {
		.name		= "uart0",
		.devname	= "ak39xx-uart.0",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_UART1,
	}, {
		.name		= "spi2",
		.usage		= 0,			
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_SPI2,
	}, {
		.name		= "spi1",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_SPI1,
	}, {
		.name		= "dac_clk",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_clk_dac,
		.ctrlbit	= AK_CLKCON_ASICCLK_DAC,
	}, {
		.name		= "dac_hclk",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_hclk_dac,
		.ctrlbit	= AK_CLKCON_ASICCLK_DAC,
	}, {
		.name		= "adc1_clk",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_clk_adc1,
		.ctrlbit	= AK_CLKCON_ASICCLK_ADC,
	}, {
		.name		= "adc2_clk",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_clk_adc2,
		.ctrlbit	= AK_CLKCON_ASICCLK_ADC,
	}, {
		.name		= "adc2_hclk",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_hclk_adc2,
		.ctrlbit	= AK_CLKCON_ASICCLK_ADC,

	}, {
		.name		= "sdio",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_SDIO,
	}, {
		.name		= "mci",
		.usage		= 0,
		.parent		= &clk_asic,
		.enable		= ak39xx_asicclk_ctrl,
		.ctrlbit	= AK_CLKCON_ASICCLK_MCI,
	}, {
		.name		= "sensor",
		.usage		= 0,
		.parent		= &clk_peri_pll,
		.enable		= ak39xx_sensor_ctrl,
		.ctrlbit	= AK_CLKCON_SCLK_CIS,
	}, 
};


/**
 * ak39xx_register_clock() - register a clock
 * @clk: The clock to register
 *
 * Add the specified clock to the list of clocks known by the system.
 */
int ak39xx_register_clock(struct clk *clk)
{
	if (clk->enable == NULL)
		clk->enable = clk_null_enable;

	/* fill up the clk_lookup structure and register it*/
	clk->lookup.dev_id = clk->devname;
	clk->lookup.con_id = clk->name;
	clk->lookup.clk = clk;
	clkdev_add(&clk->lookup);

	return 0;
}

#if 0
/**
 * ak39xx_register_clocks() - register an array of clock pointers
 * @clks: Pointer to an array of struct clk pointers
 * @nr_clks: The number of clocks in the @clks array.
 *
 * Call ak39xx_register_clock() for all the clock pointers contained
 * in the @clks list. Returns the number of failures.
 */
int ak39xx_register_clocks(struct clk **clks, int nr_clks)
{
	int fails = 0;

	for (; nr_clks > 0; nr_clks--, clks++) {
		if (ak39xx_register_clock(*clks) < 0) {
			struct clk *clk = *clks;
			printk(KERN_ERR "%s: failed to register %p: %s\n",
			       __func__, clk, clk->name);
			fails++;
		}
	}

	return fails;
}
#endif

/**
 * s3c_register_clocks() - register an array of clocks
 * @clkp: Pointer to the first clock in the array.
 * @nr_clks: Number of clocks to register.
 *
 * Call s3c24xx_register_clock() on the @clkp array given, printing an
 * error if it fails to register the clock (unlikely).
 */
void __init ak39xx_register_clocks(struct clk *clkp, int nr_clks)
{
	int ret;

	for (; nr_clks > 0; nr_clks--, clkp++) {
		ret = ak39xx_register_clock(clkp);
		if (ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
			       clkp->name, ret);
		}
	}
}


/* initialise the clock system */

T_cpu_mode ak_get_cpu_mode(void)
{
	unsigned long regval = __raw_readl(CLOCK_CPU_PLL_CTRL);

	if ((regval & AK39_CLK_CPU3X_MODE) && (~(regval & AK39_CLK_CPU2X_MODE)))
		return CPU_MODE_CPU3X;
	else if ((~(regval & AK39_CLK_CPU3X_MODE)) && (regval & AK39_CLK_CPU2X_MODE))
		return CPU_MODE_CPU2X;
	else if ((~(regval & AK39_CLK_CPU3X_MODE)) && (~(regval & AK39_CLK_CPU2X_MODE)))
		return CPU_MODE_NORMAL;
}
EXPORT_SYMBOL(ak_get_cpu_mode);

bool ak_cpu_is_normal_mode(void)
{
	return ak_get_cpu_mode() == CPU_MODE_NORMAL;
}
EXPORT_SYMBOL(ak_cpu_is_normal_mode);

bool ak_cpu_is_2x_mode(void)
{
	return ak_get_cpu_mode() == CPU_MODE_CPU2X;
}
EXPORT_SYMBOL(ak_cpu_is_2x_mode);

bool ak_cpu_is_3x_mode(void)
{
	return ak_get_cpu_mode() == CPU_MODE_CPU3X;
}
EXPORT_SYMBOL(ak_cpu_is_3x_mode);

unsigned long ak_get_cpu_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long cpu_pll_clk;
	unsigned long regval;

	regval = __raw_readl(CLOCK_CPU_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	//cpu_pll_clk = 12 * pll_m /(pll_n * __power2(pll_od)); // clk unit: MHz
	cpu_pll_clk = 12 * pll_m /(pll_n * (1 << pll_od)); // clk unit: MHz
	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		 && ((pll_m >= 84) && (pll_m <= 254)))
		
		return cpu_pll_clk * MHz;
	
	panic("cpu pll clk: %ld(Mhz) is unusable\n", cpu_pll_clk);
}
EXPORT_SYMBOL(ak_get_cpu_pll_clk);

unsigned long ak_get_asic_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long asic_pll_clk;
	unsigned long regval;

	regval = __raw_readl(CLOCK_ASIC_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	asic_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od)); // clk unit: MHz

	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		 && ((pll_m >= 84) && (pll_m <= 254)))
		
		return asic_pll_clk * MHz;
	
	panic("asic pll clk: %ld(Mhz) is unusable\n", asic_pll_clk);
}
EXPORT_SYMBOL(ak_get_asic_pll_clk);


unsigned long ak_get_peri_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long peri_pll_clk;
	unsigned long regval;

	regval = __raw_readl(CLOCK_PERI_PLL_CTRL1);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	peri_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od)); // clk unit: MHz
	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		 && ((pll_m >= 84) && (pll_m <= 254)))
		
		return peri_pll_clk * MHz;
	
	panic("peri pll clk: %ld(Mhz) is unusable\n", peri_pll_clk);
}
EXPORT_SYMBOL(ak_get_peri_pll_clk);


static unsigned long ak_get_cpu_hclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = __raw_readl(CLOCK_CPU_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;

	if (div == 0)
		return ak_get_cpu_pll_clk() >> 1;
	
	return ak_get_cpu_pll_clk() >> div;
}

static unsigned long ak_get_cpu_dclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = __raw_readl(CLOCK_CPU_PLL_CTRL);
	div = (regval & (0x7 << 20)) >> 20;

	if (div == 0)
		return ak_get_cpu_pll_clk() >> 1;
	
	return ak_get_cpu_pll_clk() >> div;
}

unsigned long ak_get_cpu_clk(void)
{
	if (ak_cpu_is_normal_mode())
		return ak_get_cpu_hclk();
		
	return ak_get_cpu_pll_clk();
}
EXPORT_SYMBOL(ak_get_cpu_clk);

unsigned long ak_get_ahb_clk(void)
{
	if (ak_cpu_is_3x_mode())
		return ak_get_cpu_pll_clk()/3;
	
	return ak_get_cpu_hclk();
}
EXPORT_SYMBOL(ak_get_ahb_clk);

unsigned long ak_get_mem_clk(void)
{
	if (ak_cpu_is_3x_mode())
		return ak_get_cpu_pll_clk()/3;
	
	return ak_get_cpu_dclk();
}
EXPORT_SYMBOL(ak_get_mem_clk);

unsigned long ak_get_vclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = __raw_readl(CLOCK_ASIC_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;
	if (div == 0)
		return ak_get_asic_pll_clk() >> 1;
	
	return ak_get_asic_pll_clk() >> div;
}
EXPORT_SYMBOL(ak_get_vclk);

unsigned long ak_get_asic_clk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = __raw_readl(CLOCK_ASIC_PLL_CTRL);
	div = regval & (1 << 24);
	if (div == 0) 
		return ak_get_vclk();
	
	return ak_get_vclk() >> 1;
}
EXPORT_SYMBOL(ak_get_asic_clk);

void aisc_freq_set(void)
{
	unsigned long asicclk, asicclk_pll;
	unsigned long div_od, div_n, div_m;
	unsigned long uartdiv;

	asicclk = CONFIG_ASIC_FREQ_VALUE;
	div_od = 2;
	div_n = 2;	
	if ((div_n < 2) || (div_n > 6)
		|| (div_od < 1) || (div_od > 3))
		panic("Asic frequency parameter Error");

	if (asicclk > 100*MHz) {
		/* asic_pll = 2vclk = 2asic */
		asicclk_pll = (asicclk/MHz) << 1;
		div_m = (asicclk_pll*(div_n * (1 << div_od)))/12;
		uartdiv = asicclk/115200-1;
		/* set asic frequency */
		REG32(AK_VA_SYSCTRL + 0x08) = ((1 << 23)|(1 <<17)|(div_od << 12)|(div_n << 8)|(div_m));
	} else {
		/* asic_pll = 2vclk = 4asic */
		asicclk_pll = (asicclk/MHz) << 2;
		div_m = (asicclk_pll*(div_n * (1 << div_od)))/12;
		uartdiv = asicclk/115200-1;
		/* set asic frequency */
		REG32(AK_VA_SYSCTRL + 0x08) = ((1 << 24)|(1 << 23)|(1 <<17)|(div_od << 12)|(div_n << 8)|(div_m));
	}
	
	/* enable asic freq change valid */
	REG32(AK_VA_SYSCTRL + 0x04) |= (1 << 28);
	/* set uart baudrate */
	REG32(AK_VA_UART + 0x0) = ((3<<28)|(3<<21)|(uartdiv));
}
EXPORT_SYMBOL(aisc_freq_set);

/***** end extern call for comm drivers compatible *****/

/* initalise all the clocks */
static int __init ak39xx_init_clocks(void)
{
 	clk_default_setrate(&clk_cpu_pll, ak_get_cpu_pll_clk());
	clk_default_setrate(&clk_asic_pll, ak_get_asic_pll_clk());
	clk_default_setrate(&clk_peri_pll, ak_get_peri_pll_clk());

	clk_default_setrate(&clk_cpu, ak_get_cpu_clk());
	clk_default_setrate(&clk_ahb, ak_get_ahb_clk());
	clk_default_setrate(&clk_mem, ak_get_mem_clk());
	clk_default_setrate(&clk_vclk, ak_get_vclk());
	clk_default_setrate(&clk_asic, ak_get_asic_clk());
	
	printk("AK39 clocks: CPU %ldMHz, MEM %ldMHz, ASIC %ldMHz\n",
			clk_cpu.rate/MHz, clk_mem.rate/MHz, clk_asic.rate/MHz);
	
	/* register clocks */
	if (ak39xx_register_clock(&clk_xtal_12M) < 0)
		printk(KERN_ERR "failed to register 12M xtal\n");

	if (ak39xx_register_clock(&clk_xtal_25M) < 0)
		printk(KERN_ERR "failed to register 25M xtal\n");

	if (ak39xx_register_clock(&clk_xtal_32K) < 0)
		printk(KERN_ERR "failed to register 32K xtal\n");

	if (ak39xx_register_clock(&clk_cpu_pll) < 0)
		printk(KERN_ERR "failed to register cpu pll clk\n");

	if (ak39xx_register_clock(&clk_asic_pll) < 0)
		printk(KERN_ERR "failed to register asic pll clk\n");

	if (ak39xx_register_clock(&clk_peri_pll) < 0)
		printk(KERN_ERR "failed to register peri pll clk\n");

	if (ak39xx_register_clock(&clk_cpu) < 0)
		printk(KERN_ERR "failed to register cpu clk\n");

	if (ak39xx_register_clock(&clk_ahb) < 0)
		printk(KERN_ERR "failed to register AHB clk\n");
	
	if (ak39xx_register_clock(&clk_mem) < 0)
		printk(KERN_ERR "failed to register memory clk\n");
	
	if (ak39xx_register_clock(&clk_vclk) < 0)
		printk(KERN_ERR "failed to register vclk\n");
	
	if (ak39xx_register_clock(&clk_asic) < 0)
		printk(KERN_ERR "failed to register asic clk\n");
	
	if (ak39xx_register_clock(&clk_opclk) < 0)
		printk(KERN_ERR "failed to register opclk\n");

	ak39xx_register_clocks(init_clocks, ARRAY_SIZE(init_clocks));

#if 0
	printk("ak39xx_init_clocks: clk gate reg=0x%p\n", REG32(CLOCK_GATE_CTRL1));
	/*
	 * Enable L2buf clock by default, Disable all other clocks.
	 * uart0 clock has been open in uncompreess.h
	 */
	int i;
	for (i = 0; i < ARRAY_SIZE(init_clocks); i++) {
		if (strcmp(init_clocks[i].name, "uart0") == 0) {
			clk_enable(&init_clocks[i]);
		}
	}
#endif

	return 0;
}

arch_initcall(ak39xx_init_clocks);

