/*
 *  @file      /driver/misc/ak_motor.c
 *  @brief     AK On-chip motor driver
 *   Copyright C 2013 Anyka CO.,LTD
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *  @author    lixinhai
 *  @date      2013-05-11
 *  @note      2013-05-11  created
 *  @note      2013-05-14 add more comments
 */

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/poll.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <mach/gpio.h>
#include <mach/reset.h>
#include <mach/clock.h>

#include <plat-anyka/ak_motor.h>

#define AK_MOTOR_DEVNAME "ak-motor"

//#define MOTOR_DEBUG

#ifdef MOTOR_DEBUG
#define PDEBUG(fmt, args...)  		printk(KERN_INFO "ak-motor:" fmt, ##args)
#else
#define PDEBUG(fmt, args...) 		
#endif


#define MOTOR_TURN_CLKWISE  	(0) /*turn by clock wise*/
#define MOTOR_TURN_ANTICLKWISE 	(1) /*turn by anti clock wise*/
#define MOTOR_STEP_PERIOD 		(64)
#define MOTOR_STEP_ANGLE 		(360/MOTOR_STEP_PERIOD)
#define MOTOR_STEP_REMAIN 		(360%MOTOR_STEP_PERIOD)
#define MOTOR_DEFAULT_DELAY_MS 	(2)

/*motor device list*/
static LIST_HEAD(motor_list);
static DEFINE_MUTEX(list_lock);

struct ak_motor_runtime
{
	int cw;
	int angle;
	int remain_angle;
	int count;
	int delay_jiffies;
	int hit_flags;
	int running;
#define MOTOR_STATUS_RUNNING 	(1)
#define MOTOR_STATUS_STOPING 	(2)
#define MOTOR_STATUS_STOPED 	(3)
};

struct ak_motor {
	struct miscdevice 		miscdev;
	struct ak_motor_plat_data *pdata;
	int 				irq_hit[AK_MOTOR_HIT_NUM]; /*hit feedback irq descriptor code*/	
	int 				hit_pin[AK_MOTOR_HIT_NUM]; /*hit feedback irq pin.*/
	int 				trigger_level[AK_MOTOR_HIT_NUM]; /*irq trigger level*/

	int 				irq_hit_type[AK_MOTOR_HIT_NUM]; /*current trigger level*/
	int 				trigger_irq; /*in course of trigger irq index.*/	
	int 				phase_pin[AK_MOTOR_PHASE_NUM]; /*phase pin*/
	spinlock_t 			lock;
	u32 		 		angular_speed;
	u32 				delay;
	struct timer_list	detect_timer;
	struct timer_list 	work_timer;

	struct ak_motor_runtime runtime;
	struct list_head 	list;
	int 				index;

	struct ak_motor_dev *curr_dev;
};


struct ak_motor_dev {
	struct ak_motor 	*motor;
	spinlock_t 			lock;
	int 				rd_flags;
	struct notify_data  data;  /*notify data to user.*/
	int 				is_open;
	wait_queue_head_t 	event;
};

/*clockwise*/
static unsigned char ctrl_tbl_cw[] = {0x03, 0x06, 0x0c, 0x09};

/*anticlockwise*/
static unsigned char ctrl_tbl_acw[] = {0x03, 0x09, 0x0c, 0x06};

static void ak_motor_stop(struct ak_motor_dev *motor_dev);
static int ak_motor_wait_for_stop(struct ak_motor_dev *motor_dev);

static inline int motor_step_count(int angle)
{
	int count;
	
/*
	int frag;

	count = angle/MOTOR_STEP_ANGLE;
	frag = angle%MOTOR_STEP_ANGLE;

	if((frag*2 >= MOTOR_STEP_ANGLE) || ((count == 0)&& angle)) {
		count++;	
	}
*/
	count = angle*MOTOR_STEP_ANGLE;
	count += (angle*MOTOR_STEP_REMAIN)/MOTOR_STEP_PERIOD;
	return count;
}

#define ms_unit 	(1000)
static inline int get_delay_by_speed(int speed)
{
	int time;

	time = (1*ms_unit)/speed;
	/* time *= MOTOR_STEP_ANGLE; */
	time /= MOTOR_STEP_ANGLE;
	if(time<=0)
		time= MOTOR_DEFAULT_DELAY_MS;
	return time;
}


/**
 * * @brief  ak motor open
 * * 
 * * open.
 * * @author lixinhai
 * * @date 2013-03-20
 * * @param[in] inode pointer.
 * * @param[in] file pointer.
 * * @return int return exec success or failed
 * * @retval returns zero on success
 * * @retval return a non-zero error code if failed
 * */
static int ak_motor_open(struct inode *inode, struct file *file)
{
	int minor;	
	bool found = false;
	struct ak_motor *motor;
	struct ak_motor_dev *motor_dev;

	PDEBUG("open the motor device.\n");

	minor = iminor(file->f_path.dentry->d_inode);
	list_for_each_entry(motor, &motor_list, list) {
		if(minor == motor->miscdev.minor) {
			found = true;
			break;
		}
	}

	if((found == false) || (motor->curr_dev != NULL)) {
		printk("ak-motor open fail, device busy now.\n");	
		return -EBUSY;
	}

	motor_dev = kzalloc(sizeof *motor_dev, GFP_KERNEL);
	if(!motor_dev) {
		printk("ak-motor: alloc the motor dev err, open fail.\n");
	}

	init_waitqueue_head(&motor_dev->event);
	motor_dev->motor = motor;
	motor->curr_dev = motor_dev;
	motor_dev->data.event = AK_MOTOR_EVENT_UNHIT;
	motor_dev->is_open = 1;
	spin_lock_init(&motor_dev->lock);

	file->private_data = motor_dev;
	printk("open ak motor device success.\n");
	return 0;
}

/**
 * @brief  ak motor close
 * 
 * close.
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] inode pointer.
 * @param[in] file pointer.
 * @return int return exec success or failed
 * @retval returns zero on success
 * @retval return a non-zero error code if failed
 */
static int ak_motor_close(struct inode *inode, struct file *file)
{
	int ret;
	struct ak_motor_dev *motor_dev = file->private_data;
	struct ak_motor *motor = motor_dev->motor;

	BUG_ON(motor==NULL);

	motor_dev->is_open = 0;
	ak_motor_stop(motor_dev);
	ret = del_timer_sync(&motor->work_timer);
	if(ret) { /*timer is pending, need wait for timer stop.*/
		ret = ak_motor_wait_for_stop(motor_dev);
		if(ret)
			printk("warning: ak motor stop fail.\n");
	}

	motor_dev->motor->curr_dev = NULL;
	kfree(motor_dev);
	printk("close ak motor device success.\n");
	return 0;
}


/**
 * @brief  ak motor read
 * 
 * read.
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] file pointer.
 * @param[in] data buf.
 * @param[in] data count.
 * @param[in] data offset.
 * @return int return read data count. 
 */
static ssize_t ak_motor_read(struct file *file, char __user *data, size_t len, loff_t *ofs)
{
	unsigned long flags;
	int ret = 0;
	struct ak_motor_dev *motor_dev = file->private_data;

	PDEBUG("read the motor data, len:%d\n", len);
	wait_event_interruptible(motor_dev->event, motor_dev->rd_flags != 0);
	
	spin_lock_irqsave(&motor_dev->lock, flags);

	ret = copy_to_user(data, &motor_dev->data, sizeof(struct notify_data));

	motor_dev->rd_flags = 0;
	PDEBUG("copy to user data, ret:%d.\n", ret);
	spin_unlock_irqrestore(&motor_dev->lock, flags);

	return ret;
}
	
	
static unsigned int ak_motor_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	struct ak_motor_dev *motor_dev = file->private_data;

	PDEBUG("motor poll.\n");

	poll_wait(file, &motor_dev->event, wait);

	if(motor_dev->rd_flags != 0)
		mask |= POLLIN;

	return mask;
}

/**
 * @brief control the motor turn.
 * 
 * read.
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] ak_motor_dev.
 * @param[in] is clockwise.
 * @param[in] turn angle..
 * @return int return exec success or failed
 * @retval returns zero on success
 * @retval return a non-zero error code if failed
 */
static int ak_motor_turn(struct ak_motor_dev *motor_dev, int cw, int angle)
{
	struct ak_motor *motor = motor_dev->motor;
	unsigned long flags;
	
	spin_lock_irqsave(&motor_dev->lock, flags);
	motor->runtime.cw = cw;
	motor->runtime.angle = angle;
	motor->runtime.remain_angle = angle;
	motor->runtime.count = motor_step_count(angle);
	motor->runtime.delay_jiffies = (motor->delay*HZ)/1000;
	motor->runtime.running = MOTOR_STATUS_RUNNING;
	motor->runtime.hit_flags = 0;

	PDEBUG("ak motor turn request, %sclockwise, angle is %d.\n", cw ?"":"anti", angle);
	mod_timer(&motor->work_timer, jiffies);
	spin_unlock_irqrestore(&motor_dev->lock, flags);
	return 0;
}

static void ak_motor_stop(struct ak_motor_dev *motor_dev)
{
	unsigned long flags;
	struct ak_motor *motor = motor_dev->motor;

	spin_lock_irqsave(&motor_dev->lock, flags);
	motor->runtime.running = MOTOR_STATUS_STOPING;
	spin_unlock_irqrestore(&motor_dev->lock, flags);
}

static int ak_motor_wait_for_stop(struct ak_motor_dev *motor_dev)
{
	int timeout = 200;
	struct ak_motor *motor = motor_dev->motor;

	while(timeout--) {
		if(motor->runtime.running == MOTOR_STATUS_STOPED)
			return 0;
		msleep(1);
	}
	return 1;
}


static long ak_motor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int val;
	int err = 0, ret = 0;
	struct ak_motor_dev *motor_dev = file->private_data;
	struct ak_motor *motor = motor_dev->motor;

	PDEBUG("exec the motor ioctl.\n");
	/* Check type and command number */
	if (_IOC_TYPE(cmd) != AK_MOTOR_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
				(void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch(cmd) {
		case AK_MOTOR_SET_ANG_SPEED:
			__get_user(val, (int __user*)arg);
			PDEBUG("ak motor set angular speed %d.\n", val);
			if((val<AK_MOTOR_MIN_SPEED) || val > AK_MOTOR_MAX_SPEED)
				return -EINVAL;

			motor->angular_speed = val;
			motor->delay = get_delay_by_speed(motor->angular_speed);
			break;
		case AK_MOTOR_GET_ANG_SPEED:
			PDEBUG("ak motor get angular speed.\n");
			__put_user(motor->angular_speed, (int __user*)arg);
			break;
		case AK_MOTOR_TURN_CLKWISE:
			__get_user(val, (int __user*)arg);
			PDEBUG("ak motor turn clk wise, angle:%d\n", val);
			if(val < 0)
				return -EINVAL;

			ak_motor_turn(motor_dev, MOTOR_TURN_CLKWISE, val);
			break;
		case AK_MOTOR_TURN_ANTICLKWISE:
			__get_user(val, (int __user*)arg);
			PDEBUG("ak motor turn anti clkwise, angle:%d\n", val);
			if(val < 0)
				return -EINVAL;

			ak_motor_turn(motor_dev, MOTOR_TURN_ANTICLKWISE, val);
			break;
		case AK_MOTOR_GET_HIT_STATUS:
			val = 0;
			ret = ak_gpio_getpin(motor->hit_pin[AK_MOTOR_HIT_LEFT]);
			if(ret == (motor->trigger_level[AK_MOTOR_HIT_LEFT] == IRQ_TYPE_LEVEL_HIGH))
				val |= AK_MOTOR_HITTING_LEFT;
			
			ret = ak_gpio_getpin(motor->hit_pin[AK_MOTOR_HIT_RIGHT]);
			if(ret == (motor->trigger_level[AK_MOTOR_HIT_RIGHT] == IRQ_TYPE_LEVEL_HIGH))
				val |= AK_MOTOR_HITTING_RIGHT;

			PDEBUG("ak motor get status, val:%d\n", val);
			__put_user(val, (int __user*)arg);
		case AK_MOTOR_TURN_STOP:
			PDEBUG("ak motor stop request.\n");
			ak_motor_stop(motor_dev);
			break;
		default:
			break;	
	}

	return ret;
}

static struct file_operations ak_motor_fops = {
	.owner = THIS_MODULE,
	.open = ak_motor_open,
	.release = ak_motor_close,
	.read = ak_motor_read,
	.poll = ak_motor_poll,
	.unlocked_ioctl = ak_motor_ioctl,
};

/*********************************************************************/

static void ak_motor_event_notify(struct ak_motor *motor, int num, int event)
{
	struct ak_motor_dev *motor_dev = motor->curr_dev;

	if(!motor_dev)
		return;

	motor_dev->data.hit_num = num;
	motor_dev->data.event = event;
	motor_dev->data.remain_angle = motor->runtime.remain_angle;
	motor_dev->rd_flags = 1;
	PDEBUG("notify to hit: hit number:%d, event:%d, remain angle:%d.\n", 
			num, event, motor->runtime.remain_angle);

	if(event == AK_MOTOR_EVENT_HIT)
		motor->runtime.hit_flags = 1;

	wake_up_interruptible(&motor_dev->event);
}

/**
 * @brief detect the motor hit the boundary or not.
 * 
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] timer data pointer.
 */
static void ak_motor_hit_detect(unsigned long data)
{
	int i;
	int hit_event;
	unsigned long flags;
	struct ak_motor *motor = (struct ak_motor*)data;
	struct ak_motor_dev *motor_dev = motor->curr_dev;

	PDEBUG("ak motor hit detect.\n");
	spin_lock_irqsave(&motor_dev->lock, flags);
	for(i=0; i<AK_MOTOR_HIT_NUM; i++) {
		if(motor->trigger_irq == motor->irq_hit[i]) {
			hit_event = (motor->trigger_level[i] == motor->irq_hit_type[i]) ? 
				AK_MOTOR_EVENT_HIT : AK_MOTOR_EVENT_UNHIT;

			if(motor->irq_hit_type[i] == IRQ_TYPE_LEVEL_LOW) {
				motor->irq_hit_type[i] = IRQ_TYPE_LEVEL_HIGH;
			}else {
				motor->irq_hit_type[i] = IRQ_TYPE_LEVEL_LOW;
			} 
			irq_set_irq_type(motor->irq_hit[i], motor->irq_hit_type[i]);
			enable_irq(motor->irq_hit[i]);
			ak_motor_event_notify(motor, i, hit_event);
			break;
		}
	}
	spin_unlock_irqrestore(&motor_dev->lock, flags);
}

/**
 * @brief hitting the boundary interrupt.
 * 
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] irq description.
 * @param[in] dev_id.
 */
static irqreturn_t ak_motor_hit_irq(int irq, void *dev_id)
{
	struct ak_motor *motor = dev_id;
	
	PDEBUG("receive the motor hit irq.\n");
	disable_irq_nosync(irq);
	motor->trigger_irq = irq;
	mod_timer(&motor->detect_timer, jiffies + msecs_to_jiffies(50));

	return IRQ_HANDLED;
}

static void ak_motor_timer_hander(unsigned long data)
{
	unsigned long flags;
	unsigned char *ctrl_tbl;
	int i, j, idx;
	struct ak_motor_dev *motor_dev;
	int ret = 0;
	struct ak_motor_runtime *runtime;
	struct ak_motor *motor = (struct ak_motor*)data;
	int count;
	char *err_desc = "";

	motor_dev = motor->curr_dev;
	if(!motor_dev)
		return;

	spin_lock_irqsave(&motor_dev->lock, flags);
	runtime = &motor->runtime;
	count = motor_step_count(runtime->angle);
	i = count - runtime->count;
	ctrl_tbl = runtime->cw ? ctrl_tbl_cw : ctrl_tbl_acw;
	//printk("[%d:%d]",i, runtime->count);
/*
	PDEBUG("ak motor turn: %s, angle:%d, delay:%d, count:%d, runtime count:%d\n",
		   	runtime->cw ?"cw":"acw", runtime->angle, motor->delay, count, runtime->count);
*/
	if(unlikely(!motor_dev->is_open)) {
		err_desc = "not open";
		goto out;
	}

	if(unlikely(motor->runtime.hit_flags)) {
		motor->runtime.hit_flags = 0;
		err_desc = "hitting the boundary";
		goto out;
	}

	if(unlikely(runtime->running != MOTOR_STATUS_RUNNING)) {
		ak_motor_event_notify(motor, 0, AK_MOTOR_EVENT_STOP);
		err_desc = "stop running";
		goto out;
	}

	idx = i % AK_MOTOR_PHASE_NUM;
	for(j=0; j<AK_MOTOR_PHASE_NUM; j++) {
		ak_gpio_setpin(motor->phase_pin[j], !!((1<<j) & ctrl_tbl[idx]));
	}

	runtime->remain_angle = runtime->angle - (i*MOTOR_STEP_PERIOD)/360;

	if(--runtime->count > 0 &&
		   	runtime->running == MOTOR_STATUS_RUNNING) {
		ret = mod_timer(&motor->work_timer, jiffies + runtime->delay_jiffies);	
		if(ret) {
			printk(KERN_ERR "ak motor: mod timer fail.\n");
		}
	} else {
		runtime->running = MOTOR_STATUS_STOPED;
		for(j=0; j<AK_MOTOR_PHASE_NUM; j++) {
			ak_gpio_setpin(motor->phase_pin[j], 0);
		}
		ak_motor_event_notify(motor, 0, AK_MOTOR_EVENT_STOP);
	}

	spin_unlock_irqrestore(&motor_dev->lock, flags);
	return;
out:
	runtime->running = MOTOR_STATUS_STOPED;
	spin_unlock_irqrestore(&motor_dev->lock, flags);
	PDEBUG("ak motor running stop, out reason:%s\n", err_desc);
	return;
}


/**
 * @brief initilize the motor gpio.
 * 
 * @author lixinhai
 * @date 2013-03-20
 * @param[in] ak_motor.
 */
static int ak_motor_init_cfg(struct ak_motor *motor)
{
	int i, ret = 0;
	bool flags = false;
	struct ak_motor_plat_data *plat = motor->pdata;
	
	for(i=0; i<AK_MOTOR_PHASE_NUM; i++) {
		ak_gpio_set(&plat->gpio_phase[i]);	
		motor->phase_pin[i] = plat->gpio_phase[i].pin;
		ak_setpin_as_gpio(motor->phase_pin[i]);
	}

	for(i=0; i<AK_MOTOR_HIT_NUM; i++) {
		motor->irq_hit[i] = 0;
		if(plat->gpio_hit[i].pin >= 0) {
			motor->hit_pin[i] = plat->gpio_hit[i].pin;
			ak_gpio_set(&plat->gpio_hit[i]);
		
			if(!flags) {
				setup_timer(&motor->detect_timer, ak_motor_hit_detect, 
					(unsigned long)motor);
				flags = true;
			}
			motor->irq_hit[i] = ak_gpio_to_irq(motor->hit_pin[i]);    

			/*request the hit boundary irq*/
			ret = request_irq(motor->irq_hit[i], ak_motor_hit_irq, 
					IRQF_DISABLED|IRQF_TRIGGER_LOW, "motor", motor);
			if(ret)
				goto out;

			printk("ak_motor:request gpio irq ret = %d, irq=%d\n",
				ret, motor->irq_hit[i]);

			motor->trigger_level[i] = motor->irq_hit_type[i] = plat->irq_hit_type[i]; 
		}
	}

out:
	return ret;
}

static int __devinit ak_motor_probe(struct platform_device *pdev)
{
	int ret;
	struct ak_motor *motor;
	struct ak_motor_plat_data *pdata;
	char name[64];

	PDEBUG("ak motor driver probe enter.\n");
	pdata = pdev->dev.platform_data;
	if(!pdata)
		return -ENODEV;

	motor = kzalloc(sizeof *motor, GFP_KERNEL);
	if(!motor) {
		printk("alloc the motor device fail.\n");
		return -ENOMEM;
	}
	
	motor->pdata = pdata;	
	platform_set_drvdata(pdev, motor);

	sprintf(name, "%s%d", pdev->name, pdev->id);

	motor->angular_speed = pdata->angular_speed;
	motor->delay = get_delay_by_speed(motor->angular_speed);
	motor->miscdev.minor = MISC_DYNAMIC_MINOR;
	motor->miscdev.name =  name;
	motor->miscdev.fops = &ak_motor_fops;
	motor->miscdev.mode = S_IRWXO;
	spin_lock_init(&motor->lock);

	setup_timer(&motor->work_timer, ak_motor_timer_hander, (unsigned long)motor);

	ak_motor_init_cfg(motor);	
	list_add_tail(&motor->list, &motor_list);

	/*register to miscdevice subsystem*/
	ret = misc_register(&motor->miscdev);
	if(ret) {
		dev_err(&pdev->dev, "register misc device fail.\n");
		ret = -ENOENT;
		goto err_misc_dev;
	}
	PDEBUG("minor:%d\n", motor->miscdev.minor);
	printk("init the ak-motor device success.\n");
	return 0;

err_misc_dev:
	kfree(motor);

	return ret;
}

static int __devexit ak_motor_remove(struct platform_device *pdev)
{
	struct ak_motor *motor = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "remove the ak motor driver.\n");
	misc_deregister(&motor->miscdev);

	kfree(motor);
	return 0;
}

static struct platform_driver ak_motor_driver = {
	.driver = {
		.name = "ak-motor",
		.owner = THIS_MODULE,
	},
	.probe = ak_motor_probe,
	.remove = __devexit_p(ak_motor_remove),
};


static int __init ak_motor_init(void)
{
	printk("AK Motor Driver (c) 2013 ANYKA\n");
	return platform_driver_register(&ak_motor_driver);
}

static void __exit ak_motor_exit(void)
{
	platform_driver_unregister(&ak_motor_driver);
}

module_init(ak_motor_init);
module_exit(ak_motor_exit);

MODULE_AUTHOR("Anyka");
MODULE_DESCRIPTION("Anyka Motor Device Driver");
MODULE_ALIAS("platform:ak-motor");
MODULE_LICENSE("GPL");

