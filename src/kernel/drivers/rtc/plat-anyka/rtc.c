/**
*  @file      drivers/rtc/rtc-ak.c
*  @brief 
*   Copyright C 2011 Anyka CO.,LTD

* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*  @author    zhou wenyong
*  @date      2011-08-29
*  @note      
*
*/
/*
 * drivers/rtc/rtc-ak.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 2011-3-22 for AK zwy
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <asm/mach/time.h>

#include <mach/hardware.h>
#include <plat/rtc.h>



//#define RTC_DEBUG
#define REG32(_reg_)  (*(volatile unsigned long *)(_reg_))


#undef PDEBUG           /* undef it, just in case */
#ifdef RTC_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_INFO fmt,## args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, "%s %d: "fmt,__FILE__, __LINE__, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif


static int ak_rtc_wakeup_enable(int en);
static void ak_rtc_alarm_enable(void );
static void ak_rtc_alarm_disable(void );


/**
*  @brief       ak_rtc_int_clear
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   void
*  @return      void
*/
static inline void ak_rtc_int_clear(void) 
{
	unsigned int regval;
	
	PDEBUG("Entering %s\n", __FUNCTION__);
	regval = ak_rtc_read(AK_RTC_SETTING);
	regval |= (1<<0);

	ak_rtc_write(AK_RTC_SETTING, regval);

}


/**
*  @brief       ak_rtc_wakeup_enable
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   en
*  @return      int
*/
static int ak_rtc_wakeup_enable(int en)
{
	unsigned int val;

	PDEBUG("%s(): Entering...\n", __func__);
	PDEBUG("%s: %s\n", __FUNCTION__, en ? "enable" : "disable");

	/* read rtc wakeup setting and clear status */
	val = REG32(OTHER_WAKEUP_CTRL);
	val &= ~RTC_WAKEUP_EN;	
	REG32(OTHER_WAKEUP_CTRL) =  val;


	if (en)
	{
		/* enable wakeup signal */
		val |= RTC_WAKEUP_EN ;//| RTC_WAKEUP_SIGNAL_LOWACTIVE);
		REG32(OTHER_WAKEUP_CTRL) =  val;
	} 
	else 
	{
		/* disable wakeup signal */
		/* val |= RTC_WAKEUP_SIGNAL_CLEAR; */
		/* ak_rtc_write(AK_WDT_TIMER1, val); */
	}
	return 0;

}


/**
*  @brief       ak_rtc_gettime
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[out]  *tm
*  @return      int
*/
static int ak_rtc_gettime(struct device *dev, struct rtc_time *tm)
{	
	unsigned long rtcset;
	unsigned long rtc_time1;
	unsigned long rtc_time2;
	unsigned long rtc_time3;

	PDEBUG("Entering %s\n", __FUNCTION__);
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
	if ((tm->tm_year < (RTC_START_YEAR - EPOCH_START_YEAR))
		|| (tm->tm_year > (RTC_START_YEAR - EPOCH_START_YEAR + RTC_YEAR_COUNT))) {
		printk("%s(): -----RTC Year must between (%d ~ %d), now tm->tm_year:%d--------------\n", 
			__func__, RTC_START_YEAR, (RTC_START_YEAR + RTC_YEAR_COUNT), tm->tm_year);
			
				tm->tm_year  = 2011 - EPOCH_START_YEAR ;
	tm->tm_mon   = 0;
	tm->tm_mday  = 1;
	tm->tm_hour  = 0;
	tm->tm_min   = 0;
	tm->tm_sec   = 0;
	tm->tm_wday  = 6;
		
	}
#if 0
	printk("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n", 
			tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday, 
			tm->tm_hour, tm->tm_min, tm->tm_sec);
#endif
	return 0;
}


/**
*  @brief       ak_rtc_settime
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[out]  *tm
*  @return      int
*/
static int ak_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	unsigned long regval;

	PDEBUG("Entering %s\n", __FUNCTION__);
	printk("%s: %d-%d-%d %d:%d:%d\n", __func__, tm->tm_year, tm->tm_mon + 1, 
					tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	if ((tm->tm_year < (RTC_START_YEAR - EPOCH_START_YEAR))
		|| (tm->tm_year > (RTC_START_YEAR - EPOCH_START_YEAR + RTC_YEAR_COUNT))) {
		printk("%s(): RTC Year must between (%d ~ %d)\n", 
			__func__, RTC_START_YEAR, (RTC_START_YEAR + RTC_YEAR_COUNT));
		return -1;
	}

	ak_rtc_write(AK_RTC_REAL_TIME1, (tm->tm_min << 6) | tm->tm_sec);
	ak_rtc_write(AK_RTC_REAL_TIME2, (tm->tm_wday << 10) | (tm->tm_mday << 5) | tm->tm_hour);
	ak_rtc_write(AK_RTC_REAL_TIME3, ((tm->tm_year + EPOCH_START_YEAR - RTC_START_YEAR) << 4) | (tm->tm_mon + 1));
	
	regval = ak_rtc_read(AK_RTC_SETTING);
	regval |= RTC_SETTING_REAL_TIME_WR;
	ak_rtc_write(AK_RTC_SETTING, regval);

	while (ak_rtc_read(AK_RTC_SETTING) & RTC_SETTING_REAL_TIME_WR)
		;
	
	udelay(40);

	return 0;
}

/**
*  @brief       ak_rtc_getalarm
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[out]  *wkalrm
*  @return      int
*/
static int ak_rtc_getalarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{	
	unsigned long rtc_alarm1;
	unsigned long rtc_alarm2;
	unsigned long rtc_alarm3;

	PDEBUG("Entering %s\n", __FUNCTION__);

	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);

	PDEBUG("%x %x %x\n", rtc_alarm1, rtc_alarm2, rtc_alarm3);
	
	wkalrm->time.tm_sec = rtc_alarm1 & 0x3F;
	wkalrm->time.tm_min = (rtc_alarm1 >> 6) & 0x3F;
	wkalrm->time.tm_hour = rtc_alarm2 & 0x1F;
	wkalrm->time.tm_mday = (rtc_alarm2 >> 5) & 0x1F;
	wkalrm->time.tm_mon = (rtc_alarm3 & 0xF) - 1;
	wkalrm->time.tm_year = ((rtc_alarm3 >> 4) & 0x7F) - EPOCH_START_YEAR + RTC_START_YEAR;
	wkalrm->time.tm_isdst = -1;

	PDEBUG("%d-%d-%d %d:%d:%d",wkalrm->time.tm_year,wkalrm->time.tm_mon + 1,wkalrm->time.tm_mday,
		wkalrm->time.tm_hour,wkalrm->time.tm_min ,wkalrm->time.tm_sec);
	return 0;
}


/**
*  @brief       ak_rtc_setalarm
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[out]  *wkalrm
*  @return      int
*/
static int ak_rtc_setalarm(struct device *dev, struct rtc_wkalrm *wkalrm)
{
	unsigned long rtc_alarm1;
	unsigned long rtc_alarm2;
	unsigned long rtc_alarm3;

	PDEBUG("Entering %s\n", __FUNCTION__);
	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);

	PDEBUG("%u %u %u\n", rtc_alarm1, rtc_alarm2, rtc_alarm3);
	PDEBUG("%d-%d-%d %d:%d:%d",wkalrm->time.tm_year,wkalrm->time.tm_mon + 1,wkalrm->time.tm_mday,
			wkalrm->time.tm_hour,wkalrm->time.tm_min ,wkalrm->time.tm_sec);

	rtc_alarm1 &= ~(0xFFF);
	rtc_alarm2 &= ~(0x3FF);
	rtc_alarm3 &= ~(0x7FF);
	
	
	rtc_alarm1 |= ((wkalrm->time.tm_min << 6) + wkalrm->time.tm_sec);
	rtc_alarm2 |= ((wkalrm->time.tm_mday << 5) + wkalrm->time.tm_hour);
	rtc_alarm3 |= (((wkalrm->time.tm_year + EPOCH_START_YEAR - RTC_START_YEAR) << 4) + (wkalrm->time.tm_mon + 1));
	
	ak_rtc_write(AK_RTC_ALARM_TIME1, rtc_alarm1);
	ak_rtc_write(AK_RTC_ALARM_TIME2, rtc_alarm2);
	ak_rtc_write(AK_RTC_ALARM_TIME3, rtc_alarm3);

	PDEBUG("%u %u %u\n", rtc_alarm1, rtc_alarm2, rtc_alarm3);

	if (wkalrm->enabled == 1)
	{
		ak_rtc_wakeup_enable(1);
		ak_rtc_alarm_enable();
	}
	else
	{
		ak_rtc_alarm_disable();
		ak_rtc_wakeup_enable(0);
	}	
	
	return 0;
}
/**
*  @brief       ak_rtc_alarm_enable
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   void
*  @return      void
*/
static void ak_rtc_alarm_enable(void )
{
	unsigned long rtc_alarm1;
	unsigned long rtc_alarm2;
	unsigned long rtc_alarm3;

	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);

	rtc_alarm1 |= (1<<13);
	rtc_alarm2 |= (1<<13);
	rtc_alarm3 |= (1<<13);

	ak_rtc_write(AK_RTC_ALARM_TIME1, rtc_alarm1);
	ak_rtc_write(AK_RTC_ALARM_TIME2, rtc_alarm2);
	ak_rtc_write(AK_RTC_ALARM_TIME3, rtc_alarm3);	
}

/**
*  @brief       ak_rtc_alarm_disable
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   void
*  @return      void
*/
static void ak_rtc_alarm_disable(void )
{
	unsigned long rtc_alarm1;
	unsigned long rtc_alarm2;
	unsigned long rtc_alarm3;

	rtc_alarm1 = ak_rtc_read(AK_RTC_ALARM_TIME1);
	rtc_alarm2 = ak_rtc_read(AK_RTC_ALARM_TIME2);
	rtc_alarm3 = ak_rtc_read(AK_RTC_ALARM_TIME3);

	rtc_alarm1 &= ~(1<<13);
	rtc_alarm2 &= ~(1<<13);
	rtc_alarm3 &= ~(1<<13);

	ak_rtc_write(AK_RTC_ALARM_TIME1, rtc_alarm1);
	ak_rtc_write(AK_RTC_ALARM_TIME2, rtc_alarm2);
	ak_rtc_write(AK_RTC_ALARM_TIME3, rtc_alarm3);	
}


/**
*  @brief       ak_rtc_ioctl
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[in]   cmd
*  @param[in]   arg
*  @return      int
*/
static int ak_rtc_ioctl(struct device *dev,
		unsigned int cmd, unsigned long arg)
{
	//PDEBUG("Entering %s: cmd %d\n", __FUNCTION__, cmd);
	unsigned int ret = -ENOIOCTLCMD;

	switch (cmd) {
		case RTC_AIE_OFF:
			PDEBUG("RTC_AIE_OFF\n");
			ak_rtc_alarm_disable();
			ak_rtc_wakeup_enable(0);

			ret = 0;
			break;
		case RTC_AIE_ON:
			PDEBUG("RTC_AIE_ON\n");
			ak_rtc_wakeup_enable(1);
			ak_rtc_alarm_enable();
			ret = 0;
			break;

		case RTC_UIE_ON:
		case RTC_UIE_OFF:
			ret = -ENOTTY;
			break;
		case RTC_RD_TIME:
			PDEBUG("Read Time\n");
			break;
		case RTC_SET_TIME:
			PDEBUG("Set Time\n");
			break;
		case RTC_ALM_SET:
			PDEBUG("Set alarm\n");
			break;
		case RTC_ALM_READ:
			PDEBUG("Read alarm\n");
			break;
		case RTC_WKALM_SET:
			PDEBUG("Set alarm\n");
			break;
		case RTC_WKALM_RD:
			PDEBUG("Read alarm\n");
			break;
		default:
			return -ENOTTY;

	}

	return ret;
}

/**
*  @brief       ak_rtc_proc
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @param[out]  *seq
*  @return      int
*/
static int ak_rtc_proc(struct device *dev, struct seq_file *seq)
{
	PDEBUG("Entering %s\n", __FUNCTION__);
	return 0;
}

/**
*  @brief       ak_rtc_alarmirq
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   irq
*  @param[out]  *id
*  @return      irqreturn_t
*/
static irqreturn_t ak_rtc_alarmirq(int irq, void *id)
{
	struct rtc_device *rdev = id;
	PDEBUG("%s(): Entering...\n", __func__);	
	ak_rtc_int_clear();
	rtc_update_irq(rdev, 1, RTC_AF | RTC_IRQF);

	return IRQ_HANDLED;
}

/**
*  @brief       ak_rtc_open
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @return      int
*/
static int ak_rtc_open(struct device *dev)
{
	PDEBUG("Entering %s\n", __FUNCTION__);
	return 0;
}

/**
*  @brief       ak_rtc_release
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @return      void
*/
static void ak_rtc_release(struct device *dev)
{
	PDEBUG("Entering %s\n", __FUNCTION__);

}

static const struct rtc_class_ops ak_rtc_ops = {
	.open		= ak_rtc_open,
	.release	= ak_rtc_release,
	.ioctl		= ak_rtc_ioctl,
	.read_time	= ak_rtc_gettime,
	.set_time	= ak_rtc_settime,
	.read_alarm	= ak_rtc_getalarm,
	.set_alarm	= ak_rtc_setalarm,
	.proc	        = ak_rtc_proc,
};

/**
*  @brief       ak_rtc_remove
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *dev
*  @return      int
*/
static int ak_rtc_remove(struct platform_device *dev)
{
	struct rtc_device *rtc = platform_get_drvdata(dev);
	PDEBUG("Entering %s\n", __FUNCTION__);
	free_irq(IRQ_RTC_ALARM, rtc);

	platform_set_drvdata(dev, NULL);
	rtc_device_unregister(rtc);

	return 0;
}

/**
*  @brief       ak_rtc_probe
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *pdev
*  @return      int
*/
static int ak_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	/* struct resource *res; */
	int ret;

	rtc = rtc_device_register("ak-rtc", &pdev->dev, &ak_rtc_ops,
			THIS_MODULE);

	if (IS_ERR(rtc)) {
		dev_err(&pdev->dev, "cannot attach rtc\n");
		ret = PTR_ERR(rtc);

		return -1;
	}

	rtc->max_user_freq = 128;

	platform_set_drvdata(pdev, rtc);

	ak_rtc_wakeup_enable(0);

	ret = request_irq(IRQ_RTC_ALARM, ak_rtc_alarmirq,
			IRQF_DISABLED,  "ak-rtc alarm", rtc);
	if (ret) {
		printk("IRQ %d error %d\n", IRQ_RTC_ALARM, ret);
		return ret;
	}

	return 0;

}

#ifdef CONFIG_PM


//static struct timespec ak_rtc_delta;
/* RTC Power management control */
/**
*  @brief       ak_rtc_suspend
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *pdev
*  @param[in]   state
*  @return      int
*/
static int ak_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct rtc_time tm;
	struct timespec time;

	time.tv_nsec = 0;

	/* calculate time delta for suspend */

	ak_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);
//	save_time_delta(&ak_rtc_delta, &time);

	return 0;
}

/**
*  @brief       ak_rtc_resume
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[out]  *pdev
*  @return      int
*/
static int ak_rtc_resume(struct platform_device *pdev)
{
	struct rtc_time tm;
	struct timespec time;

	time.tv_nsec = 0;
	ak_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);
//	restore_time_delta(&ak_rtc_delta, &time);
	
	return 0;
}
#else
#define ak_rtc_suspend NULL
#define ak_rtc_resume  NULL
#endif

static struct platform_driver ak_rtcdrv = {
	.probe		= ak_rtc_probe,
	.remove		= ak_rtc_remove,
	.suspend	= ak_rtc_suspend,
	.resume		= ak_rtc_resume,
	.driver		= {
		.name	= "ak-rtc",
		.owner	= THIS_MODULE,
	},
};

/**
*  @brief       ak_rtc_init
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   void
*  @return      int __init
*/
static int __init ak_rtc_init(void)
{
	PDEBUG("RTC Init...\n");
	
	ak_rtc_power(RTC_ON);
	
	if (test_rtc_inter_reg(AK_RTC_REAL_TIME1)<0) {
		printk("Board has not RTC support. exit %s\n", __func__);
		ak_rtc_power(RTC_OFF);
		return -ENODEV;	
	}

	//printk("AK RTC, (c) 2010 ANYKA \n");
	return platform_driver_register(&ak_rtcdrv);
}

/**
*  @brief       ak_rtc_exit
*  @author      zhou wenyong
*  @date        2011-08-29
*  @param[in]   void
*  @return      void __exit
*/
static void __exit ak_rtc_exit(void)
{	
	/*
		When RTC is powered off, this bit(AK_RTC_CONF [24] ) should be set to 0
	*/
	ak_rtc_power(RTC_OFF);
	platform_driver_unregister(&ak_rtcdrv);
}

module_init(ak_rtc_init);
module_exit(ak_rtc_exit);

MODULE_DESCRIPTION("ANYKA AK RTC Driver");
MODULE_AUTHOR("anyka");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ak-rtc");
