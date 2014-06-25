#include <linux/module.h>
#include <linux/platform_device.h>
#include <plat-anyka/wifi.h>
#include <linux/delay.h>

#define WIFI_DEV_ENABLE 1


static int __devinit akplat_wifi_probe(struct platform_device *pdev)
{
	int ret;
	struct akwifi_platform_data *pdata = pdev->dev.platform_data;

	printk("%s entered.\n", __func__);
	if(!pdata)
	{
		ret = -EINVAL;
		return ret;
	}
	
    /* akwifi power on */
	if (pdata->gpio_on.pin > 0) {
		pdata->gpio_init(&pdata->gpio_on);

		msleep(pdata->power_on_delay);
		printk("wifi power on\n");

	}
//	data->power_off();
//	data->power_on();
	return 0;
}

static int __devexit akplat_wifi_remove(struct platform_device *pdev)
{
	struct akwifi_platform_data *pdata = pdev->dev.platform_data;

	printk("%s entered.\n", __func__);

	if (pdata->gpio_off.pin > 0) {
		pdata->gpio_init(&pdata->gpio_off);

		msleep(pdata->power_off_delay);
		printk("wifi power off\n");
	}

//	data->power_off();	
	return 0;
}

static int akplat_wifi_suspend(struct platform_device *pdev, pm_message_t state)
{
//	struct wifi_power_platform_data *data = pdev->dev.platform_data;

	printk("%s entered.\n", __func__);

	//data->power_off();
	
	return 0;
}

static int akplat_wifi_resume(struct platform_device *pdev)
{
//	struct wifi_power_platform_data *data = pdev->dev.platform_data;

	printk("%s entered.\n", __func__);

	//data->power_on();
	
	return 0;
}

struct platform_device_id sdio_wifi_ids[] ={
	{
		.name = "sdio_wifi_ar6302",
		.driver_data = 0,
	}, { },
};

static struct platform_driver akplat_wifi_driver = {
	.driver = {
		.name = "anyka-wifi",
	},
//	.id_table	= sdio_wifi_ids,
	.probe = akplat_wifi_probe,
	.remove = akplat_wifi_remove,
	.suspend = akplat_wifi_suspend,
	.resume = akplat_wifi_resume,
};


static int __init sdio_wifi_init(void)
{
	return platform_driver_register(&akplat_wifi_driver);
}

static void __exit sdio_wifi_exit(void)
{
	platform_driver_unregister(&akplat_wifi_driver);
}

module_init(sdio_wifi_init);
module_exit(sdio_wifi_exit);
