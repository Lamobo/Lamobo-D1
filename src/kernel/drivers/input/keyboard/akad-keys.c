/*
 * drivers/input/keyboard/akad-keys.c
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input-polldev.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <plat-anyka/adkey.h>
#include <plat-anyka/notify.h>

#include <mach/gpio.h>
#include <mach/adc.h>

//#define ADKEY_DEBUG

#ifdef ADKEY_DEBUG
#define pr_dbg(fmt...)		printk(fmt)
#else
#define pr_dbg(fmt...)
#endif

#define DRV_NAME	"ad-keys"

struct adgpio_key_data {
	struct input_dev *input;
	struct adgpio_key *keys;
	struct timer_list timer;
	/* private: */
	struct delayed_work work;
	
	struct semaphore sem;

	struct multi_addetect *addet;
	int naddet;
	
	int poll_interval;
	int debounce_interval;
	int keycode;
	int nkey;
	
	unsigned char loop;
	unsigned char press;
	unsigned char sync_me;
};

static void adkeys_poll(struct adgpio_key_data *adkey);

static void adkey_input_queue_work(struct adgpio_key_data *adkey)
{
	unsigned long delay;

	delay = msecs_to_jiffies(adkey->poll_interval);
	if (delay >= HZ)
		delay = round_jiffies_relative(delay);
	
	queue_delayed_work(system_freezable_wq, &adkey->work, delay);
}

static void adkey_input_device_work(struct work_struct *work)
{
	struct adgpio_key_data *adkey =
		container_of(work, struct adgpio_key_data, work.work);
	
	adkeys_poll(adkey);
	adkey_input_queue_work(adkey);
}

static void addetect_plugin_notify(T_ad_check state)
{
	static bool dev1_flag = false;
	static bool dev2_flag = false;
	static bool dev3_flag = false;
	static bool dev4_flag = false;
	
	pr_dbg("adkey: plugin device state: 0x%0x\n", state);
	
	if ((state & PLUGIN_DEV1) && (!dev1_flag)) {
		addetect_notifier_call_chain(ADDEDECT_DEV1_PLUGIN, NULL);
		dev1_flag = true;
	} else if ((!(state & PLUGIN_DEV1)) && dev1_flag){
		addetect_notifier_call_chain(ADDEDECT_DEV1_PLUGOUT, NULL);
		dev1_flag = false;
	}
	
	if ((state & PLUGIN_DEV2) && (!dev2_flag)) {
		addetect_notifier_call_chain(ADDEDECT_DEV2_PLUGIN, NULL);
		dev2_flag = true;
	} else if ((!(state & PLUGIN_DEV2)) && dev2_flag){
		addetect_notifier_call_chain(ADDEDECT_DEV2_PLUGOUT, NULL);
		dev2_flag = false;
	}
	
	if ((state & PLUGIN_DEV3) && (!dev3_flag)) {
		addetect_notifier_call_chain(ADDEDECT_DEV3_PLUGIN, NULL);
		dev3_flag = true;
	} else if ((!(state & PLUGIN_DEV3)) && dev3_flag) {
		addetect_notifier_call_chain(ADDEDECT_DEV3_PLUGOUT, NULL);
		dev3_flag = false;
	}
	
	if ((state & PLUGIN_DEV4) && (!dev4_flag)) {
		addetect_notifier_call_chain(ADDEDECT_DEV4_PLUGIN, NULL);
		dev4_flag = true;
	} else if ((!(state & PLUGIN_DEV4)) && dev4_flag) {
		addetect_notifier_call_chain(ADDEDECT_DEV4_PLUGOUT, NULL);
		dev4_flag = false;
	}
}

static void adkeys_timer(unsigned long _data)
{
	struct adgpio_key_data *adkey = (struct adgpio_key_data *)_data;
	struct input_dev *input_dev = adkey->input;
	int advol, i, code;
	
	if (!adkey->press) {
		adkey->sync_me = 0;
	} else {
		advol = adc1_read_ad5();
		pr_dbg("timer: volatge: %d\n", advol);
		
		i = 0;
		do {
			if ((advol > adkey->keys[i].min)&&(advol < adkey->keys[i].max)) {
				pr_dbg("timer: volatge: %d\n", advol);
				code = adkey->keys[i].code;
				if ((code == adkey->keycode)&&(i == adkey->loop)) 
					break;
			} 
			i++;
		} while(i < adkey->nkey);
	} 
	
	/* report the key */
	input_event(input_dev, EV_KEY, adkey->keycode, adkey->press);
	input_sync(input_dev);	
}

static void adkeys_poll(struct adgpio_key_data *adkey)
{
	int advol, i;
	static struct adgpio_key *prev_keys = NULL;
	static T_ad_check state = PLUGIN_INVALID;
	static int is_change_flag = 0;
	
	advol = adc1_read_ad5();
	pr_dbg("poll: volatge: %d\n", advol);
	
	for (i = 0; i < adkey->naddet; i++) {
		if ((advol > adkey->addet[i].unpress_min)
			&&(advol < adkey->addet[i].unpress_max)) {
			
			adkey->keys = adkey->addet[i].fixkeys;
			if ((prev_keys != adkey->keys) || (state != adkey->addet[i].plugdev)) {
				prev_keys = adkey->keys;
				state = adkey->addet[i].plugdev;
				is_change_flag = 1;
			} else {
				is_change_flag = 0;
			}
			break;
		}
	}

	if (is_change_flag) {
		addetect_plugin_notify(state);
	}
	
	i = 0;
	do {
		if ((adkey->keys != NULL)
			&& (advol > adkey->keys[i].min) && (advol < adkey->keys[i].max)) {
			adkey->press = 1;
			adkey->sync_me = 1;
			
			adkey->keycode = adkey->keys[i].code;
			adkey->loop = i;
			break;
		} else 
			adkey->press = 0;
		
		i++;
	} while(i < adkey->nkey);
	
	if (adkey->sync_me) {
		mod_timer(&adkey->timer,
			jiffies + msecs_to_jiffies(adkey->debounce_interval));
	}
}
	
static int analog_gpio_probe(struct platform_device *pdev)
{
	struct adgpio_key_data *adkey;
	struct input_dev *input_dev;
	struct analog_gpio_key *pdata = NULL;
	int i, err;

	pdata = pdev->dev.platform_data;
	if (!pdata) 
		return -ENODEV;
	
	adkey = kzalloc(sizeof(struct adgpio_key_data)+
		pdata->naddet*sizeof(struct multi_addetect)+
		pdata->nkey*sizeof(struct adgpio_key), GFP_KERNEL);
	if (!adkey) 
		return -ENOMEM;
	
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		goto fail;
	}
	
	platform_set_drvdata(pdev, adkey);
	
	adkey->input = input_dev;
	if ((pdata->addet) && (pdata->naddet > 0)) {
		adkey->addet = pdata->addet;
		adkey->naddet = pdata->naddet;
		adkey->keys = pdata->addet[0].fixkeys;
	}
	adkey->nkey = pdata->nkey;

	if (pdata->interval <= 0)
		adkey->poll_interval = 500;
	else
		adkey->poll_interval = pdata->interval;
	
	if (pdata->debounce_interval <= 0)
		adkey->debounce_interval = 20;
	else
		adkey->debounce_interval = pdata->debounce_interval;
	
	adkey->press = 0;
	adkey->sync_me = 0;
		
	input_dev->name = pdev->name;
	input_dev->phys = DRV_NAME"/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0010;
	input_dev->id.version = 0x00100;
	input_dev->dev.parent = &pdev->dev;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	input_dev->keycode = adkey->keys;
	input_dev->keycodesize = sizeof(struct adgpio_key);
	input_dev->keycodemax = adkey->nkey;

	for (i = 0; i < adkey->nkey; i++)
		set_bit(adkey->keys[i].code, input_dev->keybit);
	clear_bit(KEY_RESERVED, input_dev->keybit);

	input_set_capability(input_dev, EV_MSC, MSC_SCAN);

	INIT_DELAYED_WORK(&adkey->work, adkey_input_device_work);
	
	/* Only start polling if polling is enabled */
	adkey_input_queue_work(adkey);
	
	setup_timer(&adkey->timer, adkeys_timer, (unsigned long)adkey);
	
	err = input_register_device(adkey->input);
	if (err)
		goto fail;
	
	return 0;
	
fail:
	printk(KERN_ERR "Adkey: failed to register driver, error: %d\n", err);
	platform_set_drvdata(pdev, NULL);
	input_free_device(input_dev);
	kfree(adkey);
	return err;
}

static int analog_gpio_remove(struct platform_device *pdev)
{
	struct adgpio_key_data *adkey = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&adkey->work);
	
	input_unregister_device(adkey->input);

	input_free_device(adkey->input);

	del_timer_sync(&adkey->timer);
	kfree(adkey);	

	return 0;
}


#ifdef CONFIG_PM
static int analog_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct adgpio_key_data *adkey = platform_get_drvdata(pdev);
	struct analog_gpio_key *pdata = pdev->dev.platform_data;
	
	if (pdata->wakeup > 0) 
		adkey_wakeup_enable(true);
	
	del_timer_sync(&adkey->timer);
	
	return 0;
}

static int analog_gpio_resume(struct platform_device *pdev)
{
	//struct adgpio_key_data *adkey = platform_get_drvdata(pdev);
	struct analog_gpio_key *pdata = pdev->dev.platform_data;

	if (pdata->wakeup > 0)
		adkey_wakeup_enable(false);
		
	return 0;
}
#else
#define analog_gpio_suspend		NULL
#define analog_gpio_resume		NULL
#endif

static struct platform_driver analog_ops = {
	.probe = analog_gpio_probe,
	.remove = __devexit_p(analog_gpio_remove),
	.suspend = analog_gpio_suspend,
	.resume = analog_gpio_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = DRV_NAME,
	},
};

static int __init analog_gpio_key_init(void)
{
	printk("Init ADC simulate gpio keys driver.\n");
	return platform_driver_register(&analog_ops);
}

static void __exit analog_gpio_key_exit(void)
{
	platform_driver_unregister(&analog_ops);
}

late_initcall(analog_gpio_key_init);
module_exit(analog_gpio_key_exit);

MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_DESCRIPTION("Anyka ADC simulate gpio keys driver");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_LICENSE("GPL");
