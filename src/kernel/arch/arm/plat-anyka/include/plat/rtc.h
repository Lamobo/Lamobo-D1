/*
 * linux/arch/arm/plat-anyka/include/plat/rtc.h
 *
 * AK RTC related routines
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_RTC_H
#define __ASM_ARCH_RTC_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/hardware.h>

#define EPOCH_START_YEAR		(1900)
#define RTC_START_YEAR			(1980)
#define RTC_YEAR_COUNT		(127)

#define AK_RTC_CONF			(AK_VA_SYSCTRL + 0x50)
#define AK_RTC_DATA			(AK_VA_SYSCTRL + 0x54)
#define OTHER_WAKEUP_CTRL	(AK_VA_SYSCTRL + 0x34)
#define OTHER_WAKEUP_STAT	(AK_VA_SYSCTRL + 0x38)

#define RTC_RDY_INT_CTRL   		(AK_VA_SYSCTRL + 0x2C)
#define RTC_RDY_INT_STAT  		(AK_VA_SYSCTRL + 0x30)
#define RTC_RDY_CTRL_BIT		(1 << 7)
#define RTC_RDY_STAT_BIT		(1 << 7)

#define RTC_WAKEUP_EN			(1 << 12)
#define RTC_CONF_RTC_WR_EN	(1 << 25)
#define RTC_CONF_RTC_EN		(1 << 24)
#define RTC_CONF_RTC_READ	((1 << 21) | (2 << 18) | (1 << 17))
#define RTC_CONF_RTC_WRITE	((1 << 21) | (2 << 18) | (0 << 17))

#define AK_RTC_REAL_TIME1	(0x0)
#define AK_RTC_REAL_TIME2	(0x1)
#define AK_RTC_REAL_TIME3	(0x2)
#define AK_RTC_ALARM_TIME1	(0x3)
#define AK_RTC_ALARM_TIME2	(0x4)
#define AK_RTC_ALARM_TIME3	(0x5)
#define AK_WDT_RTC_TIMER_CONF	(0x6)
#define AK_RTC_SETTING	(0x7)
#define AK_RTC_REG_MAX	AK_RTC_SETTING

#define RTC_ON 1
#define RTC_OFF 0
#define RTC_SETTING_REAL_TIME_RE	(1 << 4)
#define RTC_SETTING_REAL_TIME_WR	(1 << 3)
#define RTC_WAIT_TIME_OUT			2000
/*
 * When the RTC module begins to receive/send data, bit [24] of Interrupt Enable/Status
 * Register of System Control Module (Add: 0x0800, 004C) is set to 0; and then this
 * bit is set to 1 automatically to indicate that the data has been well received/sent
 */
static void inline ak_rtc_wait_ready(void)
{
	unsigned long timeout = 0;

	while (!(__raw_readl(RTC_RDY_INT_STAT) & RTC_RDY_STAT_BIT)) {
		++timeout;
		if (timeout >= RTC_WAIT_TIME_OUT) {
			//printk("--ak_rtc_wait_ready\n");
			break;
		}
	}		
}

static void inline rtc_ready_irq_enable(void)
{
	unsigned long regval;

	/*
	 * Mask RTC Ready Interrupt
	 */
	regval = __raw_readl(RTC_RDY_INT_CTRL);
	__raw_writel(regval | (RTC_RDY_CTRL_BIT), RTC_RDY_INT_CTRL);

	/*
	 * Wait for RTC Ready Interrupt to be cleared
	 */
	ak_rtc_wait_ready();

	/*
	 * Enable RTC Register Read/Write
	 */
	regval = __raw_readl(AK_RTC_CONF);
	regval |= RTC_CONF_RTC_WR_EN;
	__raw_writel(regval, AK_RTC_CONF);
}

static void inline rtc_ready_irq_disable(void)
{
	unsigned long regval;
	/*
	 * Disable RTC Register Read/Write
	 */
	regval = __raw_readl(AK_RTC_CONF);
	regval &= ~RTC_CONF_RTC_WR_EN;
	__raw_writel(regval, AK_RTC_CONF);
	
	/*
	 * Unmask RTC Ready Interrupt
	 */
	regval = __raw_readl(RTC_RDY_INT_CTRL);
	__raw_writel(regval & ~RTC_RDY_CTRL_BIT, RTC_RDY_INT_CTRL);
}

void ak_rtc_power(int op);

unsigned int ak_rtc_read(unsigned int addr);
unsigned int ak_rtc_write(unsigned int addr, unsigned int value);
unsigned int ak_rtc_set_wpin(bool level);
void ak_reboot_sys_by_wtd(void);
void ak_reboot_sys_by_wakeup(void);
int test_rtc_inter_reg(unsigned int addr);


#endif /*  __ASM_ARCH_RTC_H */
