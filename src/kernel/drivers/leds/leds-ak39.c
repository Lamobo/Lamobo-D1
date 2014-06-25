/* drivers/leds/leds-ak98-factory.c
 *
 * Copyright (c) 2012 anyka
 *
 * AK98- LEDs GPIO driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/leds-gpio.h>

struct ak_led_instance {
	struct led_classdev	cdev;
	struct ak_led_data	*led;
};

struct ak_led_class {
	struct ak_led_instance *instance;
	int nr_instance;
};

static void ak_led_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	unsigned int active_level = 0;
	
	struct ak_led_instance *instance = container_of(led_cdev, struct ak_led_instance, cdev);
	struct ak_led_data	*led = instance->led;

	active_level = (value ? (led->gpio.value == AK_GPIO_OUT_HIGH ? AK_GPIO_OUT_LOW : AK_GPIO_OUT_HIGH)
												: led->gpio.value);
	ak_gpio_setpin(led->gpio.pin, active_level);

}

static int ak_led_remove(struct platform_device *dev)
{
	int i;
	struct ak_led_class *class = platform_get_drvdata(dev);
	struct ak_led_instance *instance = class->instance;

	for (i = 0; i < class->nr_instance; i++) 
		led_classdev_unregister(&(instance + i)->cdev);

	kfree(instance);
	kfree(class);

	return 0;
}

static int ak_led_probe(struct platform_device *dev)
{
	int i, ret;
	struct ak_led_pdata *pdata = dev->dev.platform_data;
	struct ak_led_data *leds = pdata->leds;
	struct ak_led_instance *instance;
	struct ak_led_class *class;

	class = kzalloc(sizeof(struct ak_led_class), GFP_KERNEL);
	instance = kzalloc(sizeof(struct ak_led_instance)*(pdata->nr_led), GFP_KERNEL);
	if (!class || !instance) {
		dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
	}

	class->instance = instance;
	class->nr_instance = pdata->nr_led;
	platform_set_drvdata(dev, class);

	for (i = 0; i < class->nr_instance; i++) {
		/* Default GPIO configure according to board defined */
		ak_gpio_set(&((leds + i)->gpio));
		(instance + i)->cdev.name = (leds + i)->name;
		(instance + i)->cdev.default_trigger = (leds + i)->def_trigger;
		(instance + i)->cdev.brightness_set = ak_led_brightness_set;
		(instance + i)->cdev.flags |= LED_CORE_SUSPENDRESUME;
		(instance + i)->led = (leds + i);

		ret = led_classdev_register(&dev->dev, &(instance + i)->cdev);
		if (ret < 0) {
			dev_err(&dev->dev, "led_classdev_register failed\n");
			kfree(instance);
			kfree(class);
			return ret;
		}
	}

	return 0;
}

static struct platform_driver ak_led_driver = {
	.probe		= ak_led_probe,
	.remove		= ak_led_remove,
	.driver		= {
		.name		= "ak_led",
		.owner		= THIS_MODULE,
	},
};

static int __init ak_led_init(void)
{
	return platform_driver_register(&ak_led_driver);
}

static void __exit ak_led_exit(void)
{
	platform_driver_unregister(&ak_led_driver);
}

module_init(ak_led_init);
module_exit(ak_led_exit);

MODULE_AUTHOR("Hongguang Du <anyka>");
MODULE_DESCRIPTION("AK LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ak_led");
