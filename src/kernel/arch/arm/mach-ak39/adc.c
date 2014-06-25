/*
 * ak_adc.c - ak ADC1 operation API
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <mach/adc.h>
#include <mach/gpio.h>

static spinlock_t adc1_lock;

/**
 * @BRIEF  config saradc clk
 * @AUTHOR Gao Wangsheng
 * @DATE   2013-4-24
 * @PARAM  div     SARADC CLK = 12Mhz/(div+1); 
 * @PARAM  div value always 2
 * @RETURN 
 * @NOTE: 
 */
static void adc1_clk_cfg(unsigned long div)
{
	unsigned long saradc_chief_driven;
	unsigned long saradc_module_driven;

	//reserve the driven value	
	saradc_chief_driven = ((REG32(SAR_IF_CFG_REG)) & (0x1<<0));
	saradc_module_driven = ((REG32(SAR_IF_CFG_REG)) & (0x7<<5));

	//disable module chief driven by sar adc clk 
	REG32(SAR_IF_CFG_REG) &= (~(1<<0));

		//disable Ain0_sampling   Ain1_sampling   Bat_sampling
	REG32(SAR_IF_CFG_REG) &= (~(0x7<<5));

	//close sar adc clk
	REG32(AD_DA_CLK1_REG) &= (~(0x1<<3));

	//cofig div 
	REG32(AD_DA_CLK1_REG) &= (~0x7);
	REG32(AD_DA_CLK1_REG) |= (div & 0x7);

	//open sar adc clk
	REG32(AD_DA_CLK1_REG) |= (0x1<<3);

	//get back the driven cfg
	REG32(SAR_IF_CFG_REG) |= (saradc_chief_driven | saradc_module_driven);
}

static void power_on_adc1(void)
{
	REG32(AD_DA_CLK1_REG) &= (~(1 << 31));
}

static void power_off_adc1(void)
{
	REG32(AD_DA_CLK1_REG) |= (1 << 31);
}

static void enable_adc1_channel(int channel)
{
	REG32(SAR_IF_CFG_REG) &= ~(1 << 0);
	REG32(SAR_IF_CFG_REG) |= (1 << (channel + 5));
	REG32(SAR_IF_CFG_REG) |= (1 << 0);
}

static void disable_adc1_channel(int channel)
{
	REG32(SAR_IF_CFG_REG) &= ~(1 << 0);
	REG32(SAR_IF_CFG_REG) &= ~(1 << (channel + 5));
	REG32(SAR_IF_CFG_REG) |= (1 << 0);
}

/**
 * @brief:	Read AD0/AD1/BAT voltage
 * @author:	Zhongjunchao
 * @data:	2011-7-20
 * 
 * @Warning: 	Please don`t use this function in IRQ handle routine!
 */
unsigned long adc1_read_channel(int channel)
{
	int count;
	unsigned long val = 0;
	unsigned long flags;

	spin_lock_irqsave(&adc1_lock, flags);	

	power_on_adc1();
	enable_adc1_channel(channel);
	mdelay(1);

	val = ((REG32(SAR_IF_SMP_DAT_REG) >> (channel * 10)) & 0x3ff); 
	if (channel == AK_ADC1_BAT) {
		for(count = 0; count < 4; count++) {
			mdelay(1);
			val += ((REG32(SAR_IF_SMP_DAT_REG) >> (channel * 10)) & 0x3ff); 
		}
		val /= 5;
	}
	val = (val * AK_AVCC) >> 10;

	disable_adc1_channel(channel);
	power_off_adc1();

	spin_unlock_irqrestore(&adc1_lock, flags);

	return val;
}

/**
 * @brief:	ADC1 initialization
 * @author:	Zhongjunchao
 * @data:	2011-7-20
 * @modify:	2013-4-27
 *
 * @note:	This function will init ADC1 clock and sample rate, and then enable
 * 		ADC1 but not power on. Any device use ADC1 please use the read value API.
 * 		We use some default value, if ADC1 can`t work make be will change it.
 * 		Please use this function in machine init.
 */
int adc1_init(void)
{
	unsigned long samplerate;
	unsigned long adc1_clk, clkdiv;
	unsigned long spl_cycle, spl_hold, spl_wait;

	spin_lock_init(&adc1_lock);

	/* reset adc1 */
	REG32(RESET_CTRL_REG) &= (~(1<<30));

	/* config adc1 clk, default 1.5MHz(BAT use only in 1.5MHz) */
	clkdiv = ADC1_MAIN_CLK/ADC1_DEFAULT_CLK - 1;
	clkdiv &= 0x7;
	adc1_clk_cfg(clkdiv);

	/* release reset */
	REG32(RESET_CTRL_REG) |= (1<<30);

	/* disable all sample */
	REG32(SAR_IF_CFG_REG) &= ( ~( (1<<0) | (0x7<<5) ) );

	/* clear all adc1 interrupt state */
	REG32(SAR_IF_INT_STATUS_REG) = 0x0;

	/* mask all adc1 interrupt */
	REG32(SAR_IF_CFG_REG) &= (~ (0xf<<1));	

	/* power on adc1 */
	power_on_adc1();

	//select AVCC
	REG32(SAR_ADC_CFG_REG) &= (~((1<<16)|(1<<22)));

	/* one channel one time, default samplerate is 5000 */
	adc1_clk = ADC1_MAIN_CLK / (clkdiv + 1);
	samplerate = DEFAULT_SAMPLE;
	spl_cycle = adc1_clk / samplerate;
	spl_wait = 1;
	spl_hold = spl_wait + 16 + 1;

	REG32(SAR_IF_CFG_REG) &= (~(0xff << 14));
	REG32(SAR_IF_CFG_REG) |= (spl_wait << 14);

	REG32(SAR_TIMING_CFG_REG) = 0;
	REG32(SAR_TIMING_CFG_REG) = spl_cycle | (spl_hold << 16) ;		

	/* spl_cnt config, what is it ? */
	REG32(SAR_IF_CFG_REG) &= (~(0x7 << 8));
	REG32(SAR_IF_CFG_REG) |= (1 << 8);

	/* disable the bat div radio */
	REG32(SAR_ADC_CFG_REG) &= ~(1 << 1) ;   

	/* we don`t know who will use ADC1, so we close it */
	power_off_adc1();

	return 0;
}
