/*
  * @file battery.c
  * @battery driver for ak chip
  * @Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co
  * @author gao_wangsheng
  * @date 2011-04
  * @version 2.0
*/
  
/*
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  *
  */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <plat-anyka/bat.h>
#include <plat-anyka/adkey.h>
#include <mach/adc.h>

//#define BATTERY_GET_VOLTAGE

/* BATTERY DEFINE*/
#define UPDATE_DISCHARGE_DELAY			(HZ * 6)		/* delay 6s */
#define UPDATE_CHARGE_DELAY				(HZ * 60)		/* delay 60s */
#define UPDATE_VOLTAGE_DELAY			(HZ * 1)		/* delay 1s */
#define CHECK_PDOWN_DELAY				50				/* delay 50ms */
#define CHECK_READ_CNT					10
#define CHECK_PDOWN_CNT					5
#define CHARGE_FULL						1
#define CHARGE_NOT_FULL					0

#define PK(fmt...) 		  		//printk(fmt)  // debug  and read ad4 voltage
#define PK_SAMPLE(fmt...) 	 	//printk(fmt)  //debug read voltage sample
#define PK_CHARGE(fmt...)  		//printk(fmt)  //debug charge ac and usb in
#define PK_DISCHARGE(fmt...)  	//printk(fmt) //debug discharge
#define PK_PDOWN(fmt...)  		//printk(fmt) //debug check power down
#define get_bat_platform_data(x)	((x)->bat_ps.dev->parent->platform_data)
	
struct ak_bat{
	struct power_supply bat_ps;
	struct power_supply ac_ps;
	struct power_supply usb_ps;
	struct read_voltage_sample rd_voltage;
	struct mutex bat_lock;
	struct timer_list	timer;
	int status;
	int ac_online;
	int usb_online;
	int voltage;
	int capacity;
	int pdown_count;
	int read_count;
	atomic_t full_flag;
};

static struct delayed_work charge_work;
static struct delayed_work discharge_work;
static struct delayed_work voltage_work;
static struct delayed_work usbirq_work;
static struct delayed_work acirq_work;
static struct delayed_work pdown_work;
static struct delayed_work resume_work;
static struct ak_bat bat_device;

static struct notifier_block ac_detect_nb;


static void ak_bat_update(struct ak_bat *battery);

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		set battery full flag
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_charge_full_timer(unsigned long bat_dev)
{
	struct ak_bat *battery = (void *)bat_dev;

	if (POWER_SUPPLY_STATUS_CHARGING == battery->status)
	{
		atomic_set(&battery->full_flag, CHARGE_FULL);
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		check capacity
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_check_charge_capacity(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);
	unsigned int delay = info->bat_mach_info.full_delay * 60 * 1000; //minute to ms
	
	if (POWER_SUPPLY_STATUS_CHARGING != battery->status)
	{
		return;
	}
	
	if (atomic_read(&battery->full_flag) == CHARGE_FULL
		&& battery->capacity != info->bat_mach_info.full_capacity)
	{
		battery->capacity = info->bat_mach_info.full_capacity;
	}			
	
	if (atomic_read(&battery->full_flag) == CHARGE_NOT_FULL
		&& battery->capacity == info->bat_mach_info.full_capacity)
	{
		battery->capacity = info->bat_mach_info.full_capacity - 1;
		
		if (!timer_pending(&battery->timer))
		{
			mod_timer(&battery->timer,jiffies + msecs_to_jiffies(delay));
		}
	}			
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		read battery voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*info
*  @return      	voltage
*/
int ak_bat_read_voltage(struct ak_bat_mach_info *info)
{
	int voltage;

	PK("\n###############%s; \n",__func__);
	
	// read voltage sample
	PK("############adc1_read_bat:\n");
	voltage = (int)adc1_read_bat();
	PK("ad4_voltage = %d:\n",voltage);

	voltage = voltage 
			* (info->bat_adc.up_resistance + info->bat_adc.dw_resistance)
			/ info->bat_adc.dw_resistance;
	PK("bat_voltage = %d:\n",voltage);
		
	voltage += info->bat_adc.voltage_correct;				// correct battery voltage
	PK("correct_voltage = %d:\n",voltage);

	return voltage;

}
EXPORT_SYMBOL(ak_bat_read_voltage);

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		read and set gpio interrupt
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	gpio pin
*  @return      	void
*/
static void bat_set_int_inverse(unsigned int pin)
{
	if (ak_gpio_getpin(pin) == AK_GPIO_OUT_HIGH)
	{
		ak_gpio_intpol(pin, AK_GPIO_INT_LOWLEVEL);	
	}
	else
	{
		ak_gpio_intpol(pin, AK_GPIO_INT_HIGHLEVEL);
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		notify ac and usb state who interest in
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_notify_ac_usb_plug(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	PK_CHARGE("##################%s:\n", __func__);
	
	if (info->usb_gpio.pindata.pin >= 0)
	{
		// usb and ac pin all exist
		if (battery->usb_online)
		{
			power_notifier_call_chain(POWER_EVENT_AC_PLUGIN, NULL);
		}
		else
		{
			power_notifier_call_chain(POWER_EVENT_AC_PLUGOUT, NULL);
		}
	}
	
	else
	{
		// only ac exist
		if (battery->ac_online)
		{
			power_notifier_call_chain(POWER_EVENT_AC_PLUGIN, NULL);
		}
		else
		{
			power_notifier_call_chain(POWER_EVENT_AC_PLUGOUT, NULL);
		}
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		poweroff system when voltage down to setting level
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_check_voltage_poweroff(struct ak_bat *battery )
{
	int voltage;
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	voltage = ak_bat_read_voltage(info);
	if (voltage <= info->bat_mach_info.min_voltage)
	{
		PK_PDOWN("voltage=%d <= pdown_voltage=%d\n",voltage,
			info->bat_mach_info.min_voltage);
		schedule_delayed_work(&pdown_work,0);
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		switch work depend on status
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void switch_charge_discharge_work(struct ak_bat *battery)
{
	if (POWER_SUPPLY_STATUS_CHARGING == battery->status)
	{
		cancel_delayed_work_sync(&discharge_work);
		schedule_delayed_work(&charge_work,UPDATE_CHARGE_DELAY);
	}
	else
	{
		cancel_delayed_work_sync(&charge_work);
		del_timer_sync(&battery->timer);
		schedule_delayed_work(&discharge_work,UPDATE_DISCHARGE_DELAY);
	}
}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		return battery property to user space
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*psy...
*  @return      	int
*/
static int bat_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct ak_bat   *battery = container_of(psy, struct ak_bat, bat_ps);
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = battery->status;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = battery->voltage;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = info->bat_mach_info.max_voltage;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = info->bat_mach_info.min_voltage;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = 25;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = battery->capacity;	// real capacity
		break;
#if 0		
	case POWER_SUPPLY_PROP_POWEROFF_CAP:
		val->intval = info->bat_mach_info.poweroff_cap;	//power off capacity value
		break;
	case POWER_SUPPLY_PROP_LOW_CAP:
		val->intval = info->bat_mach_info.low_cap;	//low warring capacity
		break;
	case POWER_SUPPLY_PROP_RECOVER_CAP:
		val->intval = info->bat_mach_info.recover_cap;	//recover capacity limit
		break;
	case POWER_SUPPLY_PROP_POWER_ON_VOLTAGE:
		val->intval = info->bat_mach_info.power_on_voltage;	//power on voltage
		break;
	case POWER_SUPPLY_PROP_CPOWER_ON_VOLTAGE:
		val->intval = info->bat_mach_info.cpower_on_voltage;	//low warring capacity
		break;
#endif		
	default:
		return -EINVAL;
	}
	return 0;
}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		return usb property
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*psy...
*  @return      	void
*/
static int usb_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret = 0;
	struct ak_bat   *battery = container_of(psy, struct ak_bat, usb_ps);
	
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = battery->usb_online;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		return ac property
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*psy...
*  @return      	void
*/
static int ac_get_property(struct power_supply *psy,
			enum power_supply_property psp,
			union power_supply_propval *val)
{
	int ret = 0;
	struct ak_bat *battery = container_of(psy, struct ak_bat, ac_ps);
	
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = battery->ac_online;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}


static void ak_battery_external_power_changed(struct power_supply *bat_ps)
{
	// NULL
}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		power manage function
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*dev
*  @return      	int
*/
#ifdef CONFIG_PM
static int ak_battery_suspend(struct platform_device *dev, pm_message_t state)
{
	flush_scheduled_work();
	del_timer_sync(&bat_device.timer);
	atomic_set(&bat_device.full_flag, CHARGE_NOT_FULL);
	cancel_delayed_work_sync(&charge_work);
	cancel_delayed_work_sync(&discharge_work);
	cancel_delayed_work_sync(&voltage_work);
	cancel_delayed_work_sync(&usbirq_work);
	cancel_delayed_work_sync(&acirq_work);
	cancel_delayed_work_sync(&pdown_work);
	cancel_delayed_work_sync(&resume_work);
	
	return 0;
}

static int ak_battery_resume(struct platform_device *dev)
{

	schedule_delayed_work(&resume_work,msecs_to_jiffies(1000));
	return 0;
}
#else
#define ak_battery_suspend NULL
#define ak_battery_resume NULL
#endif

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		caculate capacity from discharge voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery...
*  @return      	void
*/
static void get_capacity_from_voltage(struct ak_bat *battery,
				struct ak_bat_mach_info *info,int voltage)
{
	int vtg = voltage;
	int min_voltage;
	int max_voltage;

	min_voltage = (POWER_SUPPLY_STATUS_CHARGING == battery->status)
				   ? info->bat_mach_info.charge_min_voltage
				   : info->bat_mach_info.min_voltage;
	max_voltage = (POWER_SUPPLY_STATUS_DISCHARGING == battery->status
					&& atomic_read(&battery->full_flag) == CHARGE_FULL)
				   ? info->bat_mach_info.full_voltage
				   : info->bat_mach_info.max_voltage;
					   
	if (vtg < min_voltage)
	{
		vtg = min_voltage;
	}

	if (vtg > max_voltage)
	{
		vtg = max_voltage;
	}

	
	battery->capacity = (vtg - min_voltage) * info->bat_mach_info.full_capacity
			/ (max_voltage - min_voltage);
			
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		init sample value
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void ak_init_sample_value(struct ak_bat *battery)
{
	struct read_voltage_sample *rd_voltage = &(battery->rd_voltage);

	rd_voltage->index	= 0;		
	rd_voltage->sum		= 0;
	rd_voltage->max	  	= 0;
	rd_voltage->min	  	= rd_voltage->design_max * 2;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		caculate voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery,*info
*  @return      	void
*/
static void ak_update_voltage(struct ak_bat *battery,
				struct ak_bat_mach_info *info)
{
	int temp;
	struct read_voltage_sample *rd_voltage = &(battery->rd_voltage);
	
	PK_SAMPLE("\n###############%s; \n",__func__);

	// if voltage_index in correct range
	if ((rd_voltage->index >= rd_voltage->sample) 
		|| (rd_voltage->index < 0))		
	{
		PK_SAMPLE("%s:error voltage sample index\n",__func__);
		ak_init_sample_value(battery);
	}
	
	// read voltage sample
	temp = ak_bat_read_voltage(info);

	// save voltage max and min
	if (temp > rd_voltage->max)
	{
		rd_voltage->max = temp;	
	}

	if (temp < rd_voltage->min)
	{
		rd_voltage->min = temp;
	}
	
	PK_SAMPLE("read_voltage=%d\n",temp);
	PK_SAMPLE("voltage_max=%d;	voltage_min=%d;\n",rd_voltage->max,rd_voltage->min);
	PK_SAMPLE("voltage_index=%d; voltage_sum=%d;\n",rd_voltage->index,rd_voltage->sum);
	
	rd_voltage->sum += temp;
	
	if (++rd_voltage->index == rd_voltage->sample)
	{
		rd_voltage->sum		-= rd_voltage->min + rd_voltage->max;
		rd_voltage->index 	-= 2;
		battery->voltage	=rd_voltage->sum / rd_voltage->index;

		PK_SAMPLE("=========bat_data.voltage = %d;=======\n",battery->voltage);
		ak_init_sample_value(battery);
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		wrap for caculating voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_update_voltage(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);
	
	mutex_lock(&battery->bat_lock);
	ak_update_voltage(battery,info);
	mutex_unlock(&battery->bat_lock);
}

static void update_voltage_immediately(struct ak_bat *battery,
				struct ak_bat_mach_info *info)
{
	int read_cnt;
	
	ak_init_sample_value(battery);

	for (read_cnt = 0; read_cnt < battery->rd_voltage.sample; read_cnt++)
	{
		ak_update_voltage(battery,info);
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		debug message
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_print_battery_info(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	printk("battery:cap=%d; vol=%d; status=%s; full=%d; poweroff_cap=%d; low_cap=%d.\n",
		battery->capacity,
		battery->voltage,
		(battery->status == POWER_SUPPLY_STATUS_CHARGING) ? "charge" : "discharge",
		ak_gpio_getpin(info->full_gpio.pindata.pin),
		info->bat_mach_info.poweroff_cap,
		info->bat_mach_info.low_cap);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		check capacity flag
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void check_cap_full_flag(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	if (atomic_read(&battery->full_flag) == CHARGE_FULL)
	{
		if (battery->voltage < info->bat_mach_info.full_voltage)
		{
			atomic_set(&battery->full_flag, CHARGE_NOT_FULL);
		}
	}
	else
	{
		if (battery->capacity == info->bat_mach_info.full_capacity)
		{
			atomic_set(&battery->full_flag, CHARGE_FULL);
		}
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		caculate discharge capacity from discharge voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void update_discharge_capacity(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);
	
	int old_capacity = battery->capacity;		

	PK_DISCHARGE("###########%s:\n",__func__);
	get_capacity_from_voltage(battery,info,battery->voltage);
	check_cap_full_flag(battery);

	if (old_capacity != battery->capacity)
	{
		power_supply_changed(&battery->bat_ps);
	}

	PK_DISCHARGE("voltage = %d;capacity = %d;\n", battery->voltage, battery->capacity);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		caculate capacity from charge voltage
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void update_charge_capacity(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	int old_capacity = battery->capacity;

	get_capacity_from_voltage(battery,info,battery->voltage);
	bat_check_charge_capacity(battery);
		
	if (battery->capacity != old_capacity)
	{
		power_supply_changed(&battery->bat_ps);
	}
					
	PK_CHARGE("voltage = %d; capacity = %d;\n",
		  battery->voltage,battery->capacity);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		caculate ac and usb status
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery, *info
*  @return      	void
*/
static void ak_get_bat_status(struct ak_bat *battery
									  ,struct ak_bat_mach_info *info)
{
	if (info->ac_gpio.is_detect_mode == BAT_CHARGE_GPIO_DETECT) {
		if (info->ac_gpio.pindata.pin >= 0)
		{
			battery->ac_online =  
		      (ak_gpio_getpin(info->ac_gpio.pindata.pin) == info->ac_gpio.active)? 1 : 0;
		}
		else
		{
			battery->ac_online = 0;
		}
	}

	if (info->usb_gpio.pindata.pin >= 0)
	{
		battery->usb_online = 
		  (ak_gpio_getpin(info->usb_gpio.pindata.pin)== info->usb_gpio.active)? 1 : 0;
	}
	else
	{
		battery->usb_online = 0;
	}

	//	if (battery->ac_online || battery->usb_online)
	if (battery->ac_online)
	{
		battery->status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else 
	{
		battery->status = POWER_SUPPLY_STATUS_DISCHARGING;
	}

}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		quickly power off system
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void check_machine_power_off(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	if ((poweroff_enable == info->bat_mach_info.power_off)
		&& (POWER_SUPPLY_STATUS_DISCHARGING == battery->status)
		&& (battery->voltage <= info->bat_mach_info.min_voltage))
	{
		printk("battery:voltage=%d,machine power off immediately!\n",battery->voltage);
		machine_power_off();
	}
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		ac irq work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_acirq_work(struct work_struct *work)
{
	struct ak_bat *battery = &bat_device;
	
	PK_CHARGE("##################%s:\n", __func__);
	ak_bat_update(battery);
	check_machine_power_off(battery);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		ac irq handler
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	irq, *battery
*  @return      	irqreturn_t
*/
static irqreturn_t akbat_ac_irqhandler(int irq, void *handle)
{
	struct ak_bat *battery = (struct ak_bat *)handle;
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);
	
	PK_CHARGE("##################%s:\n", __func__);
	disable_irq_nosync(irq);
	bat_set_int_inverse(ak_irq_to_gpio(irq));	
	schedule_delayed_work(&acirq_work,msecs_to_jiffies(info->ac_gpio.delay));
	enable_irq(irq);
	return IRQ_HANDLED;	
}

static int ac_detect_plugin(struct notifier_block *nb,
	unsigned long val, void *data)
{
	struct ak_bat *battery = &bat_device;

	if (val == ADDETECT_AC_PLUGIN) {
		battery->ac_online = 1;
	}
	else if (val == ADDETECT_AC_PLUGOUT) {
		battery->ac_online = 0;
	}

	ak_bat_update(battery);
	return 0;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		usb irq handler
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery, irq
*  @return      	irqreturn_t
*/
static irqreturn_t akbat_usb_irqhandler(int irq, void *handle) 
{
	struct ak_bat *battery = (struct ak_bat *)handle;
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	PK_CHARGE("##################%s:\n", __func__);
	disable_irq_nosync(irq);
	bat_set_int_inverse(ak_irq_to_gpio(irq));	
	schedule_delayed_work(&usbirq_work,msecs_to_jiffies(info->usb_gpio.delay));
	enable_irq(irq);
	return IRQ_HANDLED;		
}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		update battery info
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void ak_bat_update(struct ak_bat *battery)
{
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);
	
	PK_CHARGE("##################%s:\n", __func__);
	mutex_lock(&battery->bat_lock);
	
	ak_get_bat_status(battery,info);
	update_voltage_immediately(battery,info);
	get_capacity_from_voltage(battery,info,battery->voltage);
	bat_check_charge_capacity(battery);	
	power_supply_changed(&battery->bat_ps);
	power_supply_changed(&battery->ac_ps);
	power_supply_changed(&battery->usb_ps);
	
	switch_charge_discharge_work(battery);
	bat_notify_ac_usb_plug(battery);	
	
	mutex_unlock(&battery->bat_lock);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		update charge info
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void ak_bat_update_charge(struct ak_bat *battery)
{
	PK_CHARGE("##################%s:\n", __func__);
	mutex_lock(&battery->bat_lock);
	update_charge_capacity(battery);
	mutex_unlock(&battery->bat_lock);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		update discharge info
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void ak_bat_update_discharge(struct ak_bat *battery)
{
	PK_DISCHARGE("##################%s:\n", __func__);
	mutex_lock(&battery->bat_lock);
	update_discharge_capacity(battery);
	mutex_unlock(&battery->bat_lock);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		charge work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_charge_work(struct work_struct *work)
{
	ak_bat_update_charge(&bat_device);
	schedule_delayed_work(&charge_work,UPDATE_CHARGE_DELAY);
//	schedule_delayed_work(&charge_work,HZ*6);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		discharge work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*battery
*  @return      	void
*/
static void bat_discharge_work(struct work_struct *work)
{
	ak_bat_update_discharge(&bat_device);
	schedule_delayed_work(&discharge_work,UPDATE_DISCHARGE_DELAY);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		voltage work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_voltage_work(struct work_struct *work)
{
#ifdef BATTERY_GET_VOLTAGE
	// 5 seconds to print battery info
	static int print_time = 10;
	if (--print_time < 0) {
		print_time = 5;
		bat_print_battery_info(&bat_device);
	}
#endif	
	bat_check_voltage_poweroff(&bat_device);
	bat_update_voltage(&bat_device);
	schedule_delayed_work(&voltage_work,UPDATE_VOLTAGE_DELAY);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		usb irq work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_usbirq_work(struct work_struct *work)
{
	struct ak_bat *battery = &bat_device;
	
	PK_CHARGE("##################%s:\n", __func__);
	ak_bat_update	(battery);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		power down work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_pdown_work(struct work_struct *work)
{
	int voltage;
	struct ak_bat *battery = &bat_device;
	struct ak_bat_mach_info *info = get_bat_platform_data(battery);

	PK_PDOWN("#############%s:\n",__func__);
	if (battery->status != POWER_SUPPLY_STATUS_DISCHARGING)
	{
		return;
	}

	if (battery->read_count++ < CHECK_READ_CNT)
	{
		PK_PDOWN("read_count=%d\n",battery->read_count);
		voltage = ak_bat_read_voltage(info);
		PK_PDOWN("voltage=%d\n",voltage);
		
		if (voltage <= info->bat_mach_info.min_voltage)
		{
			battery->pdown_count++;
		}

		schedule_delayed_work(&pdown_work,msecs_to_jiffies(CHECK_PDOWN_DELAY));
	}
	else
	{
		PK_PDOWN("pdown_count=%d\n",battery->pdown_count);
	
		if (battery->pdown_count >= CHECK_PDOWN_CNT)
		{
			mutex_lock(&battery->bat_lock);
			printk(KERN_WARNING "Ak_battery:capacity=%d => 0!\n",battery->capacity);
			battery->capacity = 0;
			power_supply_changed(&battery->bat_ps);
			cancel_delayed_work_sync(&voltage_work);
			cancel_delayed_work_sync(&charge_work);
			cancel_delayed_work_sync(&discharge_work);
			mutex_unlock(&battery->bat_lock);
		}

		battery->read_count = 0;
		battery->pdown_count = 0;
	}

}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		resume work
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*work
*  @return      	void
*/
static void bat_resume_work(struct work_struct *work)
{
	ak_bat_update(&bat_device);
	schedule_delayed_work(&voltage_work,UPDATE_VOLTAGE_DELAY);
}

static int battery_reboot_notify(struct notifier_block *nb, unsigned long code,
				void *unused)
{
	switch (code) {
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		printk("%s:code=%ld, cap=%d, vol=%d, status=%s.\n",
			__func__,
			code,
			bat_device.capacity,
			bat_device.voltage,
			bat_device.status==POWER_SUPPLY_STATUS_CHARGING ? "charging":"discharging");
		break;
	}

	return NOTIFY_DONE;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		power supply property
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	void
*  @return      	void
*/
static enum power_supply_property bat_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
#if 0	
	POWER_SUPPLY_PROP_POWEROFF_CAP,
	POWER_SUPPLY_PROP_LOW_CAP,
	POWER_SUPPLY_PROP_RECOVER_CAP,
	POWER_SUPPLY_PROP_POWER_ON_VOLTAGE,
	POWER_SUPPLY_PROP_CPOWER_ON_VOLTAGE,
#endif	
};

static enum power_supply_property usb_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property ac_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		battery device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	void
*  @return      	void
*/
static struct ak_bat bat_device = {
	.bat_ps= {
		.name					= "battery",
		.type					= POWER_SUPPLY_TYPE_BATTERY,
		.properties 			= bat_power_props,
		.num_properties 		= ARRAY_SIZE(bat_power_props),
		.get_property			= bat_get_property,
		.external_power_changed = ak_battery_external_power_changed,
		.use_for_apm			= 1,
	},

	.ac_ps = {
		.name					= "ac",
		.type					= POWER_SUPPLY_TYPE_MAINS,
		.properties 			= ac_power_props,
		.num_properties 		= ARRAY_SIZE(ac_power_props),
		.get_property			= ac_get_property,
	},

 	.usb_ps = {
		.name					= "usb",
		.type					= POWER_SUPPLY_TYPE_USB,
		.properties 			= usb_power_props,
		.num_properties 		= ARRAY_SIZE(usb_power_props),
		.get_property			= usb_get_property,
	},
	
	.status			= POWER_SUPPLY_STATUS_CHARGING,
	.ac_online		= 0,
	.usb_online		= 0,
	.read_count		= 0,
	.pdown_count 	= 0,
	.full_flag		= ATOMIC_INIT(CHARGE_NOT_FULL),
};


static struct notifier_block battery_reboot_nb = {
	.notifier_call = battery_reboot_notify,
	.next = NULL,
	.priority = INT_MIN
};


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		probe battery
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*dev
*  @return      	fail or not
*/
static int __devinit ak_battery_probe(struct platform_device *dev)
{
	int ret = 0;
	struct ak_bat_mach_info *info;

	info = (struct ak_bat_mach_info *)dev->dev.platform_data;
	if (!info) 
	{
		printk(KERN_ERR "%s:no platform data for battery\n",__func__);
		ret = -EINVAL;
		goto nodata_out;
	}	

	/* initialize hardware */
	mutex_init(&bat_device.bat_lock);
	bat_device.rd_voltage.sample = info->bat_mach_info.voltage_sample;
	bat_device.rd_voltage.design_max = info->bat_mach_info.max_voltage;
	init_timer(&bat_device.timer);
	bat_device.timer.function = bat_charge_full_timer;
	bat_device.timer.data = (unsigned long) &bat_device;
	
	INIT_DELAYED_WORK(&charge_work,bat_charge_work);
	INIT_DELAYED_WORK(&discharge_work,bat_discharge_work);
	INIT_DELAYED_WORK(&voltage_work,bat_voltage_work);
	INIT_DELAYED_WORK(&usbirq_work,bat_usbirq_work);
	INIT_DELAYED_WORK(&pdown_work,bat_pdown_work);
	INIT_DELAYED_WORK(&resume_work,bat_resume_work);
		
	ret = power_supply_register(&dev->dev, &bat_device.bat_ps);
	if (ret != 0)
	{
		goto cancel_out;
	}
	
	ret = power_supply_register(&dev->dev,&bat_device.usb_ps);
	if (ret != 0)
	{
		goto free_bat_ps_out;
	}
	
	ret = power_supply_register(&dev->dev,&bat_device.ac_ps);
	if (ret != 0)
	{
		goto free_usb_ps_out;
	}

	// use for charge full state 
	if (info->full_gpio.pindata.pin >= 0)
	{
		info->gpio_init(&info->full_gpio.pindata);
	}

	if (info->ac_gpio.is_detect_mode == BAT_CHARGE_GPIO_DETECT) {
		INIT_DELAYED_WORK(&acirq_work,bat_acirq_work);
		// use for ac charge in irq
		if (info->ac_gpio.irq >= 0)
		{
			info->gpio_init(&info->ac_gpio.pindata);
			bat_set_int_inverse(info->ac_gpio.pindata.pin);	
			if (request_irq(info->ac_gpio.irq,akbat_ac_irqhandler,0,"ac_charge",&bat_device))
			{
				printk(KERN_ERR "%s:Could not allocate IRQ %d\n", __func__,info->ac_gpio.irq);
				ret = -EIO;
				goto free_ac_ps_out;
			}
		}
	} else if (info->ac_gpio.is_detect_mode == BAT_CHARGE_ADC_DETECT) {
		memset(&ac_detect_nb, 0, sizeof(ac_detect_nb));
		ac_detect_nb.notifier_call = ac_detect_plugin;
		addetect_register_client(&ac_detect_nb);
	}

	// use for usb charge in irq
	if (info->usb_gpio.irq >= 0)
	{
		info->gpio_init(&info->usb_gpio.pindata);
		mdelay(100);
		bat_set_int_inverse(info->usb_gpio.pindata.pin);	
		if (request_irq(info->usb_gpio.irq,akbat_usb_irqhandler,0,"usb_charge",&bat_device))
		{
			printk(KERN_ERR "%s:Could not allocate IRQ %d\n", __func__,info->usb_gpio.irq);
			ret = -EIO;
			goto free_acirq_out;
		}
	}

	ak_bat_update(&bat_device);
	schedule_delayed_work(&voltage_work,UPDATE_VOLTAGE_DELAY);
	register_reboot_notifier(&battery_reboot_nb);
	
	bat_print_battery_info(&bat_device);
	printk("AK Battery initialized\n");
	return ret;
	
free_acirq_out:
	if (info->ac_gpio.irq > 0)
	{
		disable_irq(info->ac_gpio.irq);
		free_irq(info->ac_gpio.irq, dev);
	}
free_ac_ps_out:
	power_supply_unregister(&bat_device.ac_ps);
free_usb_ps_out:
	power_supply_unregister(&bat_device.usb_ps);
free_bat_ps_out:
	power_supply_unregister(&bat_device.bat_ps);
cancel_out:
	del_timer_sync(&bat_device.timer);
	cancel_delayed_work_sync(&charge_work);
	cancel_delayed_work_sync(&discharge_work);
	cancel_delayed_work_sync(&voltage_work);
	cancel_delayed_work_sync(&usbirq_work);
	cancel_delayed_work_sync(&acirq_work);
	cancel_delayed_work_sync(&pdown_work);
	cancel_delayed_work_sync(&resume_work);
	
nodata_out:
	printk(KERN_ERR "###########%s:ERR out##########\n",__func__);
	return ret;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		remove battery
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	*dev
*  @return      	fail or not
*/
static int __devexit ak_battery_remove(struct platform_device *dev)
{	
	struct ak_bat_mach_info *info;
	
	info = (struct ak_bat_mach_info *)dev->dev.platform_data;
	if (!info) {
		printk(KERN_ERR "no platform data for battery\n");
		return -EINVAL;
	}

#ifdef CONFIG_AC_GPIO_DETECT
	if (info->ac_gpio.irq > 0)
	{
		disable_irq(info->ac_gpio.irq);
		free_irq(info->ac_gpio.irq, &bat_device);
	}
#elif defined(CONFIG_AC_AD_DETECT)
	addetect_unregister_client(&ac_detect_nb);
#endif

	if (info->usb_gpio.irq > 0)
	{
		disable_irq(info->usb_gpio.irq);
		free_irq(info->usb_gpio.irq, &bat_device);
	}

	del_timer_sync(&bat_device.timer);
	cancel_delayed_work_sync(&charge_work);
	cancel_delayed_work_sync(&discharge_work);
	cancel_delayed_work_sync(&voltage_work);
	cancel_delayed_work_sync(&usbirq_work);
	cancel_delayed_work_sync(&acirq_work);
	cancel_delayed_work_sync(&pdown_work);
	cancel_delayed_work_sync(&resume_work);
	
	power_supply_unregister(&bat_device.bat_ps);
	power_supply_unregister(&bat_device.ac_ps);
	power_supply_unregister(&bat_device.usb_ps);

	unregister_reboot_notifier(&battery_reboot_nb);
	return 0;
}

static struct platform_driver ak_battery_driver = {
	.driver		= {
		.name	= "battery",
	},
	.probe		= ak_battery_probe,
	.remove		= __devexit_p(ak_battery_remove),
	.suspend	= ak_battery_suspend,
	.resume		= ak_battery_resume,
};

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		init battery
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	void
*  @return      	fail or not
*/
static int __init ak_battery_init(void)
{
	return platform_driver_register(&ak_battery_driver);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		exit battery
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-11-2
*  @param[out]  	void
*  @param[in]   	void
*  @return      	void
*/
static void __exit ak_battery_exit(void)
{
	platform_driver_unregister(&ak_battery_driver);
}

module_init(ak_battery_init);
module_exit(ak_battery_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("gao_wangsheng <gao_wangsheng@anyka.oa>");
MODULE_DESCRIPTION("ak battery driver");
