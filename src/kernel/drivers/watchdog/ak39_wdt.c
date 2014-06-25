/*
 * drivers/char/watchdog/ak98_wdt.c
 *
 * Watchdog driver for ANYKA ak98 processors
 *
 * Author: Wenyong Zhou
 *
 * Adapted from the IXP2000 watchdog driver by Lennert Buytenhek.
 * The original version carries these notices:
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright 2004 (c) MontaVista, Software, Inc.
 * Based on sa1100 driver, Copyright (C) 2000 Oleg Drokin <green@crimea.edu>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <linux/reboot.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/rtc.h>

#define SELECT_WTC	1
#define SELECT_RTC	0

#define WDT_DEBUG
#undef PDEBUG
#ifdef WDT_DEBUG
#define PDEBUG(fmt, args...) printk(KERN_INFO fmt,## args)
#else
#define PDEBUG(fmt, args...) 
#endif

static int nowayout = WATCHDOG_NOWAYOUT;
static unsigned int def_heartbeat = (0x1FFF);	/* Default is 8 seconds */
static unsigned int now_heartbeat = (0x1FFF);

static unsigned long in_use;
static atomic_t in_write = ATOMIC_INIT(0);
static DEFINE_SPINLOCK(wdt_lock);

static int ak98_wdt_disable_nb(struct notifier_block *n, unsigned long state,void *cmd);
static struct notifier_block ak98_wdt_nb = {
	.notifier_call = ak98_wdt_disable_nb,
};

module_param(def_heartbeat, int, 0);
MODULE_PARM_DESC(def_heartbeat, "Watchdog heartbeat in seconds (default 8s)");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");

static void select_wdt_rtc(int which)
{
	unsigned long val;

	val = ak_rtc_read(AK_RTC_SETTING);
	if (which == 0)
		val &= ~(1 << 10);		
	else
		val |= (1 << 10);
	ak_rtc_write(AK_RTC_SETTING, val);
}

void wdt_enable(void)
{
	unsigned long val;

	spin_lock(&wdt_lock);	
	select_wdt_rtc(SELECT_WTC);

	//enable watchdog timer
	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val |= (1<<13);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);

	//set timer
	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val &= (1<<13);
	val |= (def_heartbeat & 0x1FFF);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);

	//open watchdog and watchdog output
	val = ak_rtc_read(AK_RTC_SETTING);
	val |= ((1<<5) | (1<<2));
	val &= ~(1<<11);
	ak_rtc_write(AK_RTC_SETTING, val);	

	select_wdt_rtc(SELECT_RTC);
	spin_unlock(&wdt_lock);
}

static void wdt_disable(void)
{
	unsigned long val;
	
	spin_lock(&wdt_lock);
	select_wdt_rtc(SELECT_WTC);
	
	//clear watchdog timer
	val = ak_rtc_read(AK_RTC_SETTING);
	val |= (1<<6);
	ak_rtc_write(AK_RTC_SETTING, val);

	//disable watchdog timer
	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val &= ~(1<<13);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);

	//close watchdog and watchdog output
	val = ak_rtc_read(AK_RTC_SETTING);
	val &= ~((1<<2) | (1<<5));
	ak_rtc_write(AK_RTC_SETTING, val);
	
	select_wdt_rtc(SELECT_RTC);
	spin_unlock(&wdt_lock);
}

void wdt_keepalive(unsigned int heartbeat)
{
	unsigned long val;

	PDEBUG("heartbeat = %x\n", heartbeat);
	spin_lock(&wdt_lock);
	select_wdt_rtc(SELECT_WTC);

	//clear watchdog timer
	val = ak_rtc_read(AK_RTC_SETTING);
	val |= (1<<6);
	ak_rtc_write(AK_RTC_SETTING, val);

	val = ak_rtc_read(AK_WDT_RTC_TIMER_CONF);
	val &= (1<<13);
	val |= (heartbeat & 0x1FFF);
	ak_rtc_write(AK_WDT_RTC_TIMER_CONF, val);

	select_wdt_rtc(SELECT_RTC);
	spin_unlock(&wdt_lock);
}

/**
 * @brief:      ak98_wdt_disable_nb
 * @author:     zhongjunchao
 * @date:       2011-10-11
 *
 * @note:       Sometimes, system reboot and doesn`t disable watchdog. we disable it
 *              for safe.
 */
static int ak98_wdt_disable_nb(struct notifier_block *n, unsigned long state,void *cmd)
{
        wdt_disable();
        return NOTIFY_DONE;
}

static struct watchdog_info ident = {
	.options	= WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity	= "ANYKA ak98 Watchdog",
};

/**
 * @brief:	ak98_wdt_ioctl
 * @author:	zhouwenyong
 * @modify:	zhongjunchao
 * @date:	2011-9-26
 *
 * @note:	WDIOC_GETSUPPORT,return struct watchdog_info.
 * 		WDIOC_KEEPALIVE, set a default timeout (8s).
 *		WDIOC_SETTIMEOUT, set a timeout value what you want (max 8s).
 *		WDIOC_GETTIMEOUT, query the current timeout.
 *		More detail, refer to kernel/Documentation/watchdog/watchdog-api.txt
 */
static long ak98_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int ret = -ENOTTY;
	int time;

	switch (cmd) {

	case WDIOC_GETSUPPORT:
		return copy_to_user((struct watchdog_info *)argp, &ident,
				   sizeof(ident)) ? -EFAULT : 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);

	case WDIOC_KEEPALIVE:
		wdt_keepalive(def_heartbeat);
		now_heartbeat = def_heartbeat;
		return 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(time, p))
			return -EFAULT;

		PDEBUG("timeout = %d\n", time);
		if (time <= 0 || time > 8)
			return -EINVAL;

		now_heartbeat = time * 1024 - 1;
		wdt_keepalive(now_heartbeat);
		return 0;

	case WDIOC_GETTIMEOUT:
		return put_user((now_heartbeat + 1)/1024, p);

	default:
		return -ENOTTY;
	}

	return ret;
}

/**
 * @brief:	ak98_wdt_write
 * @author:	zhouwenyong
 * @modify:	zhongjunchao
 * @date:	2011-9-26
 *
 * @note:	We support "Magic Close" that driver will not disable the watchdog unless
 * 		a specific magic character 'V' has been sent to /dev/watchdog just before 
 * 		closing the file.
 */
static ssize_t ak98_wdt_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	if (len) {
		size_t i;

		atomic_set(&in_write, 1);
		for (i = 0; i != len; i++) {
			char c;

			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V') {
				PDEBUG("Detect \"V\" Magic Character\n");
				atomic_set(&in_write, 0);
			}
		}
		wdt_keepalive(def_heartbeat);
	}

	return len;
}

/**
 * @brief:	ak98_wdt_open
 * @author:	zhouwenyong
 * @modify:	zhongjunchao
 * @date:	2011-9-26
 *
 * @note:	We only support one process open /dev/watchdog, to avoid overwrite watchdog
 * 		timeout value.
 */
static int ak98_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(0, &in_use))
		return -EBUSY;
	
	if (nowayout)
		__module_get(THIS_MODULE);

	wdt_enable();

	return nonseekable_open(inode, file);
}

/**
 * @brief:	ak98_wdt_release
 * @author:	zhouwenyong
 * @modify:	zhongjunchao
 * @date:	2011-9-26
 *
 * @note:	When open CONFIG_WATCHDOG_NOWAYOUT option, nowayout = 1.
 * 		When you open and write to /dev/watchdog, please write magic character 'V' 
 * 		before close the file. If not, watchdog will not disable after close 
 * 		/dev/watchdog.
 */
static int ak98_wdt_release(struct inode *inode, struct file *file)
{
	if (nowayout)
		printk(KERN_INFO "WATCHDOG: Driver support nowayout option -"
							"no way to disable watchdog\n");
	else if (atomic_read(&in_write))
		printk(KERN_CRIT "WATCHDOG: Device closed unexpectedly - "
							"timer will not stop\n");
	else
		wdt_disable();
	clear_bit(0, &in_use);

	return 0;
}

static const struct file_operations ak98_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= ak98_wdt_write,
	.unlocked_ioctl	= ak98_wdt_ioctl,
	.open		= ak98_wdt_open,
	.release	= ak98_wdt_release,
};

static struct miscdevice ak98_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &ak98_wdt_fops,
};

static int __init ak98_wdt_init(void)
{		
	spin_lock(&wdt_lock);
	ak_rtc_power(RTC_ON);
	spin_unlock(&wdt_lock);	
	register_reboot_notifier(&ak98_wdt_nb);
	
	return misc_register(&ak98_wdt_miscdev);
}

static void __exit ak98_wdt_exit(void)
{
	unregister_reboot_notifier(&ak98_wdt_nb);
	ak_rtc_power(RTC_OFF);

	misc_deregister(&ak98_wdt_miscdev);
}
module_init(ak98_wdt_init);
module_exit(ak98_wdt_exit);

MODULE_DESCRIPTION("ANYKA ak98 Watchdog");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
