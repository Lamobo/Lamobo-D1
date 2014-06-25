/*
 * linux/arch/arm/plat-anyka/rtc.c
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <asm/mach/time.h>
#include <plat/rtc.h>

static int rtc_cnt = 0;

#undef REG32
#define REG32(_reg_)  (*(volatile unsigned long *)(_reg_))

//reboot system by watchdog
void ak_reboot_sys_by_wtd(void)
{
	unsigned long val;	
	unsigned long flags;
	//static spinlock_t loc_lock;

	//spin_lock_init(&loc_lock);
	//spin_lock(&loc_lock);
	local_irq_save(flags);
	ak_rtc_power(RTC_ON);

	//select wdt
	val = ak_rtc_read(AK_RTC_SETTING);	
	val |= (1 << 10);	
	ak_rtc_write(AK_RTC_SETTING, val);	
	
	//clear timer
	val = ak_rtc_read(AK_RTC_SETTING);
	val |= (1<<6);
	ak_rtc_write(AK_RTC_SETTING, val);
	

	//enable watchdog timer
	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val |= (1<<13);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);
		
	//set timer
	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val &= (1<<13);
	val |= (5 & 0x1FFF);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);

	
	//open watchdog and watchdog output
	val = ak_rtc_read(AK_RTC_SETTING);
	val |= ((1<<5) | (1<<2));
	val &= ~(1<<11);
	ak_rtc_write(AK_RTC_SETTING, val);

	local_irq_restore(flags);
//	spin_unlock(&loc_lock);
}
EXPORT_SYMBOL(ak_reboot_sys_by_wtd);

void ak_reboot_sys_by_wakeup(void)
{
	//static spinlock_t loc_lock;
	struct rtc_time ptm;
	struct rtc_time *tm = &ptm;
	//unsigned char mdays[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
	unsigned long flags;
	
	unsigned long rtcset;
	unsigned long rtc_time1;
	unsigned long rtc_time2;
	unsigned long rtc_time3;	
	
	unsigned long rtc_alarm1;
	unsigned long rtc_alarm2;
	unsigned long rtc_alarm3;	
	unsigned int val_1, val_2;
	unsigned long time;
	
	//spin_lock_init(&loc_lock);
	//spin_lock(&loc_lock);
	local_irq_save(flags);
	ak_rtc_power(RTC_ON);
	
	rtcset = ak_rtc_read(AK_RTC_SETTING);
	rtcset |= RTC_SETTING_REAL_TIME_RE;
	ak_rtc_write(AK_RTC_SETTING, rtcset);
	
	rtc_time1 = ak_rtc_read(AK_RTC_REAL_TIME1);
	rtc_time2 = ak_rtc_read(AK_RTC_REAL_TIME2);
	rtc_time3 = ak_rtc_read(AK_RTC_REAL_TIME3);
	
	tm->tm_year  = ((rtc_time3 >> 4) & 0x7F) - EPOCH_START_YEAR + RTC_START_YEAR;
	tm->tm_mon   = (rtc_time3 & 0xF) - 1;
	tm->tm_mday  = (rtc_time2 >> 5) & 0x1F;
	tm->tm_hour  = rtc_time2 & 0x1F;
	tm->tm_min   = (rtc_time1 >> 6) & 0x3F;
	tm->tm_sec   = rtc_time1 & 0x3F;
	tm->tm_wday  = (rtc_time2 >> 10) & 0x7;
	tm->tm_isdst = -1;	
	
	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);
	
	
	rtc_alarm1 &= ~(0xFFF);
	rtc_alarm2 &= ~(0x3FF);
	rtc_alarm3 &= ~(0x7FF);
	
	#define DELAY_TIME 5
	
	rtc_tm_to_time(tm, &time);

	time += DELAY_TIME;

	rtc_time_to_tm(time, tm);
	/*tm->tm_sec += DELAY_TIME;
	
	if (tm->tm_sec >= 60) 
	{
		tm->tm_sec -= 60;
		tm->tm_min++;
		if (tm->tm_min >= 60)
		{
			tm->tm_min = 0;
			tm->tm_hour++;						
			if (tm->tm_hour >= 24)
			{
				tm->tm_hour = 0;
				tm->tm_mday++;
				if ((tm->tm_year%400==0) || ((tm->tm_year%100!=0) && (tm->tm_year%4==0))) 
					mdays[2] = 29;
				if (tm->tm_mday > mdays[tm->tm_mon])
				{
					tm->tm_mday = 1;
					tm->tm_mon++;
					if (tm->tm_mon > 12)
					{
						tm->tm_mon = 1;
						tm->tm_year++;
					}
				}
			}
		}
			
	}*/
	rtc_alarm1 |= ((tm->tm_min << 6) + tm->tm_sec);
	rtc_alarm2 |= ((tm->tm_mday << 5) + tm->tm_hour);
	rtc_alarm3 |= (((tm->tm_year + EPOCH_START_YEAR - RTC_START_YEAR) << 4) + (tm->tm_mon + 1));
	
	ak_rtc_write(AK_RTC_ALARM_TIME1, rtc_alarm1);
	ak_rtc_write(AK_RTC_ALARM_TIME2, rtc_alarm2);
	ak_rtc_write(AK_RTC_ALARM_TIME3, rtc_alarm3);
	
	val_1 = REG32(OTHER_WAKEUP_CTRL);
	
	
	val_2 = ak_rtc_read(AK_RTC_SETTING);
	
	if (1)
	{
		/* enable wakeup signal */
		val_1 |= RTC_WAKEUP_EN;// | RTC_WAKEUP_SIGNAL_LOWACTIVE);
		REG32(OTHER_WAKEUP_CTRL) =  val_1;
		
		val_2 |= (1<<2);
		ak_rtc_write(AK_RTC_SETTING, val_2);
	} 
	
	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);
	
	rtc_alarm1 |= (1<<13);
	rtc_alarm2 |= (1<<13);
	rtc_alarm3 |= (1<<13);
	
	ak_rtc_write(AK_RTC_ALARM_TIME1, rtc_alarm1);
	ak_rtc_write(AK_RTC_ALARM_TIME2, rtc_alarm2);
	ak_rtc_write(AK_RTC_ALARM_TIME3, rtc_alarm3);	

	local_irq_restore(flags);
//	spin_unlock(&loc_lock);
	ak_rtc_set_wpin(0);
}

EXPORT_SYMBOL(ak_reboot_sys_by_wakeup);

void ak_rtc_power(int op)
{
	unsigned long rtcconf;

	switch(op)
	{
	case RTC_ON:
		if (++rtc_cnt == 1)
		{
			rtcconf = __raw_readl(AK_RTC_CONF);
			rtcconf |= RTC_CONF_RTC_EN;
			__raw_writel(rtcconf, AK_RTC_CONF);
		}
		break;
		/*
		When RTC is powered off, this bit(AK_RTC_CONF [24] ) should be set to 0
	*/
	case RTC_OFF:
		if (!(--rtc_cnt))
		{
			rtcconf = __raw_readl(AK_RTC_CONF);
			rtcconf &= ~RTC_CONF_RTC_EN;
			__raw_writel(rtcconf, AK_RTC_CONF);
		}
		break;
	default:
		printk("Error RTC power operation.\n");
		break;
	}
}

EXPORT_SYMBOL(ak_rtc_power);

/* 
*  check if internal rtc works or not
* return -1 no rtc device;0 have rtc device
*/
int test_rtc_inter_reg(unsigned int addr)
{
	unsigned long regval = 0;
	unsigned long flags;
	int timeout = 0;
	int ret = 0;
	
	local_irq_save(flags);

	// unmask rtc_ready irq
	regval = __raw_readl(RTC_RDY_INT_CTRL);
	__raw_writel(regval | (RTC_RDY_CTRL_BIT), RTC_RDY_INT_CTRL);

	// wait for more 1ms to access rtc register
	while (!(__raw_readl(RTC_RDY_INT_STAT) & RTC_RDY_STAT_BIT))
	{
		if (timeout++ > 1000)
		{
		    ret = -1;
		    break;
		}
		udelay(1);
	}

	// mask rtc_ready irq
	regval = __raw_readl(RTC_RDY_INT_CTRL);
	__raw_writel(regval & ~(RTC_RDY_CTRL_BIT), RTC_RDY_INT_CTRL);
	
	local_irq_restore(flags);
	return ret;
}
EXPORT_SYMBOL(test_rtc_inter_reg);

unsigned int ak_rtc_read(unsigned int addr)
{
	unsigned long regval = 0;
	unsigned long flags;

	if (addr > AK_RTC_REG_MAX) {
		printk("%s(): Invalid RTC Register, address=%d\n",
			__func__, addr);
		return -1;
	}

	local_irq_save(flags);

	rtc_ready_irq_enable();

	regval  = __raw_readl(AK_RTC_CONF);
	regval  &= ~(0x3FFFFF) ;
	regval  |= (RTC_CONF_RTC_READ | (addr << 14));
	__raw_writel(regval, AK_RTC_CONF);

	udelay(100);

	ak_rtc_wait_ready();
	rtc_ready_irq_disable();
	local_irq_restore(flags);

	// according to ATC drivers ,this want to wait 1/32K s here
	udelay(312);

	regval = __raw_readl(AK_RTC_DATA);
	regval &= 0x3FFF;
	
	return regval;
}
EXPORT_SYMBOL(ak_rtc_read);

unsigned int ak_rtc_write(unsigned int addr, unsigned int value)
{
	unsigned long regval = 0;
	unsigned long flags;

	if (addr > AK_RTC_REG_MAX) {
		printk("%s(): Invalid RTC Register, address=%d\n",
			__func__, addr);
		return -1;
	}

	local_irq_save(flags);

	rtc_ready_irq_enable();

	regval  = __raw_readl(AK_RTC_CONF);
	regval  &= ~0x3FFFFF;
	regval  |= (RTC_CONF_RTC_WRITE | (addr << 14) | value);
	__raw_writel(regval, AK_RTC_CONF);

	udelay(100);

	ak_rtc_wait_ready();
	
	rtc_ready_irq_disable();

	local_irq_restore(flags);

	// according to ATC drivers ,this want to wait 1/32K s here
	udelay(312);

	return 0;
}
EXPORT_SYMBOL(ak_rtc_write);

unsigned int ak_rtc_set_wpin(bool level)
{
	unsigned long regval;
	unsigned int bit;
	unsigned int timeout = 0;

	ak_rtc_power(RTC_ON);

	if (test_rtc_inter_reg(0) < 0) {
		printk("Board has not RTC support. exit %s\n", __func__);
		ak_rtc_power(RTC_OFF);
		return -ENODEV;
	}
	
	bit = level ? 8 : 7;
	printk("---ak_rtc_set_wpin:%d---\r\n", bit);
	regval = ak_rtc_read(AK_RTC_SETTING);
	regval |= (1 << bit);
	ak_rtc_write(AK_RTC_SETTING, regval);

	while (ak_rtc_read(AK_RTC_SETTING) & (1 << bit)) {
		++timeout;
		if (timeout >= RTC_WAIT_TIME_OUT) {
			//printk("--ak_rtc_set_wpin--\n");
			break;
		}
	}
	
	return 0;
}
EXPORT_SYMBOL(ak_rtc_set_wpin);
