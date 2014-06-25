/*
 * ak_adc.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __AK_ADC_H_
#define __AK_ADC_H_

#include <mach/map.h>

#define RESET_CTRL_REG			(AK_VA_SYSCTRL + 0x20)
#define AD_DA_CLK1_REG			(AK_VA_SYSCTRL + 0x0C)
#define SAR_ADC_CFG_REG			(AK_VA_SYSCTRL + 0x98)
#define SAR_IF_CFG_REG			(AK_VA_SYSCTRL + 0x5C)
#define SAR_TIMING_CFG_REG		(AK_VA_SYSCTRL + 0x60)
#define SAR_THRESHOLD_REG		(AK_VA_SYSCTRL + 0x64)
#define SAR_IF_SMP_DAT_REG		(AK_VA_SYSCTRL + 0x68)
#define SAR_IF_INT_STATUS_REG		(AK_VA_SYSCTRL + 0x6C)


#define ADC1_MAIN_CLK			(12000000)
#define ADC1_DEFAULT_CLK		(1500000)		/* FIXME: bat work in 1.5MHz */
#define DEFAULT_SAMPLE			(1000)			/* FIXME: somebody need change it */
#define AK_AVCC				(3300)			/* AVCC always 3.3V ? */

#define AK_ADC1_AD0			(0)			/* FIXME: AD0=AIN0=BAT ? AD1=AIM1 ? AD2=AIN2 ? */
#define AK_ADC1_AD1			(1)
#define AK_ADC1_BAT			(2)

int adc1_init(void);
unsigned long adc1_read_channel(int channel);

#define adc1_read_bat()		adc1_read_channel(AK_ADC1_AD0) 
#define adc1_read_ad5()		adc1_read_channel(AK_ADC1_AD1)	/* is it right ? */ 

#endif
