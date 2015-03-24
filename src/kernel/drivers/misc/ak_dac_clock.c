/*
 * ak_dac_clock  -- driver for reading the dac clock;
 * Features
 * AUTHOR Wudaochao
 * 12-04-13
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/delay.h>
#include <asm/io.h>

extern unsigned long long getPlaybackEclapseTime(void);


static struct miscdevice ak_dacclk_dev;

static int ak_dacclk_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int ak_dacclk_release(struct inode *inode, struct file *file)
{
	return 0;
}

long ak_dacclk_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long long *p;

	p = (unsigned long long *)arg;

	*p = getPlaybackEclapseTime();

	return 0;
}

struct file_operations ak_dacclk_fops = {
	.release = ak_dacclk_release,
	.open = ak_dacclk_open,
	.unlocked_ioctl = ak_dacclk_ioctl
};

static int __init ak_dacclk_init(void)
{
	int err;
	ak_dacclk_dev.name = "dac_clock";
	ak_dacclk_dev.minor = MISC_DYNAMIC_MINOR;
	ak_dacclk_dev.fops = &ak_dacclk_fops;
	printk(KERN_INFO "ak_dacclk_init==>%s\n", ak_dacclk_dev.name);
	
	err = misc_register(&ak_dacclk_dev);
	if (err) {
		printk(KERN_INFO "ak_dacclk_init==>misc_register failed\n");
		return -1;
	} else {
		printk(KERN_INFO "ak_dacclk_init==>misc_register succeedded\n");
	}	
	
	return 0;
}

static void __exit ak_dacclk_exit(void)
{
	misc_deregister(&ak_dacclk_dev);
}

module_init(ak_dacclk_init);
module_exit(ak_dacclk_exit);

MODULE_DESCRIPTION("Anyka Dac Clock Driver");
MODULE_AUTHOR("Anyka");
MODULE_LICENSE("GPL");

