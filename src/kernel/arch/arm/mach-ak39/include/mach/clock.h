/* 
 * arch/arm/mach-ak39/include/mach/clock.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _ANYKA_CLK_H_
#define _ANYKA_CLK_H_

#include <linux/clkdev.h>

#define CLOCK_CPU_PLL_CTRL			(AK_VA_SYSCTRL + 0x04)
#define CLOCK_ASIC_PLL_CTRL			(AK_VA_SYSCTRL + 0x08)
#define CLOCK_ADC2_DAC_CTRL			(AK_VA_SYSCTRL + 0x0C)
#define CLOCK_ADC2_DAC_HS_CTRL		(AK_VA_SYSCTRL + 0x10)
#define CLOCK_PERI_PLL_CTRL1		(AK_VA_SYSCTRL + 0x14)
#define CLOCK_PERI_PLL_CTRL2		(AK_VA_SYSCTRL + 0x18)
#define CLOCK_GATE_CTRL1			(AK_VA_SYSCTRL + 0x1C)
#define CLOCK_SOFT_RESET			(AK_VA_SYSCTRL + 0x20)

/* clock gate control register bit */
#define AK_CLKCON_MCLK_DRAM			(1 << 24)
#define AK_CLKCON_VCLK2_VIDEO		(1 << 20)
#define AK_CLKCON_VCLK1_CAMERA		(1 << 19)
#define AK_CLKCON_ASICCLK_USB		(1 << 15)
#define AK_CLKCON_ASICCLK_ENCRY		(1 << 14)
#define AK_CLKCON_ASICCLK_MAC		(1 << 13)
#define AK_CLKCON_ASICCLK_GPIO		(1 << 12)
#define AK_CLKCON_ASICCLK_IRDA		(1 << 11)
#define AK_CLKCON_ASICCLK_I2C		(1 << 10)
#define AK_CLKCON_ASICCLK_L2MEM		(1 << 9)
#define AK_CLKCON_ASICCLK_UART2		(1 << 8)
#define AK_CLKCON_ASICCLK_UART1		(1 << 7)
#define AK_CLKCON_ASICCLK_SPI2		(1 << 6)
#define AK_CLKCON_ASICCLK_SPI1		(1 << 5)
#define AK_CLKCON_ASICCLK_DAC		(1 << 4)
#define AK_CLKCON_ASICCLK_ADC		(1 << 3)
#define AK_CLKCON_ASICCLK_SDIO		(1 << 2)
#define AK_CLKCON_ASICCLK_MCI		(1 << 1)

/* ADC2/DAC clock control register */
#define AK_CLKCON_CLK_DAC			(1 << 28)
#define AK_CLKCON_CLK_ADC1			(1 << 3)

/* ADC2/DAC high speed clock control register */
#define AK_CLKCON_HCLK_ADC2			(1 << 28)
#define AK_CLKCON_HCLK_DAC			(1 << 18)
#define AK_CLKCON_CLK_ADC2			(1 << 8)


/* PERI PLL channel clock control register1 */
//1: peri pll 	0: external 12MHz
#define AK_CLKCON_CLK_PHY_SEL		(1 << 19)
//1: peri pll	0: external 25MHz
#define AK_CLKCON_CLK_MAC_SEL		(1 << 18)
#define AK_CLKCON_CLK_12M			(1 << 17)
#define AK_CLKCON_CLK_25M			(1 << 16)
#define AK_CLKCON_CLK_25M_IN		(1 << 14)


/* PERI PLL channel clock control register2 */
/* 1: positive clk	0: negative clk */
#define AK_CLKCON_PCLK_CIS			(1 << 20)		//camera
#define AK_CLKCON_SCLK_CIS			(1 << 18)		//sensor
#define AK_CLKCON_CLK_OPCLK			(1 << 8)		//MAC


/**
 * struct clk_ops - standard clock operations
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fields again. We leave
 * enable in struct clk.
 *
 * Adding an extra layer of indirection into the process should
 * not be a problem as it is unlikely these operations are going
 * to need to be called quickly.
 */
struct clk_ops {
	int		    (*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	    (*get_rate)(struct clk *c);
	unsigned long	    (*round_rate)(struct clk *c, unsigned long rate);
	int		    (*set_parent)(struct clk *c, struct clk *parent);
};

struct clk {
	struct list_head	list;
	struct module		*owner;
	struct clk			*parent;
	const char			*name;
	const char			*devname;
	int		      id;
	int		      usage;
	unsigned long rate;
	unsigned long ctrlbit;
	
	struct clk_lookup	lookup;
	struct clk_ops	*ops;
	int		    (*enable)(struct clk *, int enable);
#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
	struct dentry		*dent;	/* For visible tree hierarchy */
#endif
};


/* core clock support */
extern struct clk clk_xtal_12M;
extern struct clk clk_xtal_25M;
extern struct clk clk_xtal_32K;
extern struct clk clk_pll;
extern struct clk clk_cpu;
extern struct clk clk_ahb;
extern struct clk clk_mem;
extern struct clk clk_vclk;
extern struct clk clk_asic;


/* other clocks which may be registered by board support */

typedef enum {
	CPU_MODE_NORMAL,
	CPU_MODE_CPU2X,
	CPU_MODE_CPU3X,	
} T_cpu_mode;

#define AK39_CLK_CPU3X_MODE		(1 << 26)
#define AK39_CLK_CPU2X_MODE		(1 << 24)


#define MHz	1000000UL

T_cpu_mode ak_get_cpu_mode(void);
bool ak_cpu_is_3x_mode(void);
bool ak_cpu_is_2x_mode(void);
bool ak_cpu_is_normal_mode(void);

unsigned long ak_get_cpu_pll_clk(void);
unsigned long ak_get_asic_pll_clk(void);
unsigned long ak_get_peri_pll_clk(void);
unsigned long ak_get_pll_clk(void);
unsigned long ak_get_cpu_clk(void);
unsigned long ak_get_ahb_clk(void);
unsigned long ak_get_mem_clk(void);
unsigned long ak_get_vclk(void);
unsigned long ak_get_asic_clk(void);
void aisc_freq_set(void);

#endif
