/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>

#include <plat-anyka/wifi.h>
#include <plat-anyka/otg-hshcd.h>
#include <plat-anyka/ak_camera.h>
#include <plat-anyka/aksensor.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <plat/l2.h>
#include <mach/devices.h>
#include <mach/gpio.h>
#include <mach-anyka/mac.h>
#include <mach/ak_codec.h>

#include <mach/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <plat-anyka/akmci.h>
#include <plat-anyka/adkey.h>

#include <mach/leds-gpio.h>
#include <linux/input.h>
#include <plat-anyka/gpio_keys.h>
#include <plat-anyka/bat.h>

#include <plat-anyka/ak_motor.h>
#include "cpu.h"
#include "irq.h"
#include <mach/adc.h>
#include <plat-anyka/akgpios.h>
#include <mach/reboot.h>
#include <plat-anyka/ak_crypto.h>


/**
 * @brief: spi device info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
#define SPI_ONCHIP_CS 	(0)		/*means not need gpio*/
static unsigned long ak39_spidev_cs[AKSPI_CS_NUM] = {
	[AKSPI_ONCHIP_CS] = SPI_ONCHIP_CS,	/*gpio 25, spidev0: ak-spiflash*/
};

struct ak_spi_info ak39_spi1_info = {
	.pin_cs = ak39_spidev_cs,
	.num_cs = ARRAY_SIZE(ak39_spidev_cs),
	.bus_num = AKSPI_BUS_NUM1,
	.clk_name = "spi1",
	.mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH,
	.xfer_mode = AKSPI_XFER_MODE_DMA,
};

/**
 * @brief: spiflash device info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static struct flash_platform_data ak39_spiflash_info= {
	.bus_width = FLASH_BUS_WIDTH_4WIRE | FLASH_BUS_WIDTH_2WIRE | FLASH_BUS_WIDTH_1WIRE,
	.type 	   = NULL,
};

/**
 * @brief: spi bus device info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static struct spi_board_info ak39_spi_board_dev[] = {
	{ 		
		.modalias = "ak-spiflash",	
		.bus_num = AKSPI_BUS_NUM1,		
		.chip_select = AKSPI_ONCHIP_CS,
		.mode = SPI_MODE_0,
		.max_speed_hz = 20*1000*1000,
		.platform_data = &ak39_spiflash_info,
	},
};

/**
 * @brief: motor0 device info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static struct ak_motor_plat_data ak39_motor0_pdata = {
		.gpio_phase[0] = {
			.pin = AK_GPIO_37,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[1] = {
			.pin = AK_GPIO_38,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[2] = {
			.pin = AK_GPIO_39,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[3] = {
			.pin = AK_GPIO_40,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},

		.gpio_hit[0] = {
			.pin = AK_GPIO_62,
			.pulldown = -1,
			.pullup	= -1,
			.value	= -1,
			.dir	= AK_GPIO_DIR_INPUT,
			.int_pol = -1,
		},
		.gpio_hit[1] ={
			.pin = AK_GPIO_63,
			.pulldown = -1,
			.pullup	= -1,
			.value	= -1,
			.dir	= AK_GPIO_DIR_INPUT,
			.int_pol = -1,
		},
	.irq_hit_type[0] = IRQ_TYPE_LEVEL_LOW,
	.irq_hit_type[1] = IRQ_TYPE_LEVEL_LOW,

	.angular_speed = 100,	/* angle/s */
};


/**
 * @brief: motor1 device info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static struct ak_motor_plat_data ak39_motor1_pdata = {
		.gpio_phase[0] = {
			.pin = AK_GPIO_56,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[1] = {
			.pin = AK_GPIO_58,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[2] = {
			.pin = AK_GPIO_59,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},
		.gpio_phase[3] = {
			.pin = AK_GPIO_60,
			.pulldown = -1,
			.pullup	= -1,
			.value	= AK_GPIO_OUT_LOW,
			.dir	= AK_GPIO_DIR_OUTPUT,
			.int_pol = -1,
		},

		.gpio_hit[0] = {
			.pin = AK_GPIO_61,
			.pulldown = -1,
			.pullup	= -1,
			.value	= -1,
			.dir	= AK_GPIO_DIR_INPUT,
			.int_pol = -1,
		},
		.gpio_hit[1] ={
			.pin = AK_GPIO_49,
			.pulldown = -1,
			.pullup	= -1,
			.value	= -1,
			.dir	= AK_GPIO_DIR_INPUT,
			.int_pol = -1,
		},
	.irq_hit_type[0] = IRQ_TYPE_LEVEL_LOW,
	.irq_hit_type[1] = IRQ_TYPE_LEVEL_LOW,

	.angular_speed = 100,	/* angle/s */
};


/**
 * @brief: SDIO device platform data
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
struct ak_mci_platform_data sdio_plat_data = {
	.irq_cd_type = IRQ_TYPE_LEVEL_LOW,
	.detect_mode = AKMCI_PLUGIN_ALWAY,
	.xfer_mode   = AKMCI_XFER_L2DMA,
	.mci_mode = MCI_MODE_SDIO,
    .gpio_init = ak_gpio_set,
    .max_speed_hz = 25*1000*1000,
    .gpio_cd = {
        .pin = AK_GPIO_18,
        .pulldown = AK_PULLDOWN_DISABLE,
        .pullup = -1,
        .value = -1,
        .dir = AK_GPIO_DIR_INPUT,
        .int_pol = -1,
    },
    .gpio_wp = {
        .pin = -1,
        .pulldown = AK_PULLDOWN_DISABLE,
        .pullup = AK_PULLUP_ENABLE,
        .value = -1,
        .dir = AK_GPIO_DIR_INPUT,
        .int_pol = -1,
    }
};


/**
 * @brief: MMC/SD  platform data
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct ak_mci_platform_data mmc_plat_data = {
	.irq_cd_type = IRQ_TYPE_LEVEL_LOW,
	.detect_mode = AKMCI_DETECT_MODE_AD,
	.xfer_mode	 = AKMCI_XFER_L2DMA,
	.mci_mode = MCI_MODE_MMC_SD,
	.max_speed_hz = 25*1000*1000,
    .gpio_init = ak_gpio_set,
    .gpio_cd = {
        .pin = -1,//AK_GPIO_47,
        .pulldown = AK_PULLDOWN_DISABLE,
        .pullup = -1,
        .value = -1,
        .dir = AK_GPIO_DIR_INPUT,
        .int_pol = -1,
    },
    .gpio_wp = {
        .pin = -1,
        .pulldown = AK_PULLDOWN_DISABLE,
        .pullup = AK_PULLUP_ENABLE,
        .value = -1,
        .dir = AK_GPIO_DIR_INPUT,
        .int_pol = -1,
    }
};


struct ak_crypto_plat_data akcrypto_pdata = {
	.encrypt_mode = CRYPTO_SINGLE_GROUP_MODE,
	/*.encrypt_mode = CRYPTO_MULTI_GROUP_MODE,*/
};

/* akwifi platform data */
struct akwifi_platform_data akwifi_pdata = {
	.gpio_init = ak_gpio_set,
#if defined(CONFIG_SDIO_WIFI)
	.gpio_on = {
		.pin= AK_GPIO_52,
		.pulldown= -1,
		.pullup 	= -1,
		.value= AK_GPIO_OUT_HIGH,
		.dir= AK_GPIO_DIR_OUTPUT,
		.int_pol= -1,
	},
	.gpio_off = {
		.pin= AK_GPIO_52,
		.pulldown= -1,
		.pullup 	= -1,
		.value= AK_GPIO_OUT_LOW,
		.dir= AK_GPIO_DIR_OUTPUT,
		.int_pol= -1,
	},
	.power_on_delay   = 10,
	.power_off_delay  = 10,
#else
	.gpio_on = {
		.pin		= AK_GPIO_50,
		.pulldown	= -1,
		.pullup		= AK_PULLUP_ENABLE,
		.value		= AK_GPIO_OUT_HIGH,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.gpio_off = {
		.pin		= AK_GPIO_50,
		.pulldown	= -1,
		.pullup		= AK_PULLUP_ENABLE,
		.value		= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.power_on_delay   = 2000,
	.power_off_delay  = 0,
#endif
};


/**
 * @brief: akwifi device info for wifi driver
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device anyka_wifi_device = {
	.name = "anyka-wifi",
	.id = -1,
	.dev = {
		.platform_data = &akwifi_pdata,
	},
};


/**
 * @brief: usb bus device  platform data
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct akotghc_usb_platform_data akotghc_plat_data = {
	.gpio_init = ak_gpio_set,
	.gpio_pwr_on = {
		.pin = AK_GPIO_55,
		.pulldown = AK_PULLDOWN_DISABLE,
		.pullup	= -1,
		.value	= AK_GPIO_OUT_HIGH,
		.dir	= AK_GPIO_DIR_OUTPUT,
		.int_pol = -1,
	},	
	.gpio_pwr_off = {
		.pin = AK_GPIO_55,
		.pulldown = AK_PULLDOWN_DISABLE,
		.pullup	= -1,
		.value	= AK_GPIO_OUT_LOW,
		.dir	= AK_GPIO_DIR_OUTPUT,
		.int_pol = -1,
	},
	.switch_onboard = {
		.pin = -1,
	},
	.switch_extport = {
		.pin = -1,
	},
};

/**
 * @brief: ethenet device  platform data
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct ak_mac_data ak39_mac_pdata = {
	.gpio_init = ak_gpio_set,
	.pwr_gpio = {
		.pin		= AK_GPIO_54,
		.pulldown	= AK_PULLDOWN_DISABLE,
		.pullup		= -1,
		.value		= AK_GPIO_OUT_HIGH,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.phy_rst_gpio = {
		.pin		= AK_GPIO_53,
		.pulldown	= -1,
		.pullup		= AK_PULLUP_DISABLE,
		.value		= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
};


/**
* @brief		ak pcm device struct
			hp and spk can identify by GPIO or AD.
			wo can initialize it in this struct.
* @author	dengzhou 
* @date	  2012-07-19
*/
struct ak39_codec_platform_data ak39_codec_pdata =
{
	.hpdet_gpio =
	{
		.pin        = AK_GPIO_7,
		.dir		= AK_GPIO_DIR_INPUT,
		.pullup		= AK_PULLUP_DISABLE,
		.pulldown	= -1,
		.value      = -1,
		.int_pol	= -1,
	},
	.spk_down_gpio =
	{
		.pin        = AK_GPIO_3,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.pullup		= -1,
		.pulldown	= -1,
		.value      = AK_GPIO_OUT_LOW,
		.int_pol	= -1,
	},
	.hpmute_gpio =
	{
		.pin        = INVALID_GPIO, //AK_GPIO_29,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.pullup		= -1,
		.pulldown	= -1,
		
		.value      = AK_GPIO_OUT_LOW,
		.int_pol	= -1,
	},

	.hp_on_value          = AK_GPIO_OUT_LOW,
	.hpdet_irq            = IRQ_GPIO_7,
	.bIsHPmuteUsed        = 0,
	.hp_mute_enable_value = AK_GPIO_OUT_HIGH,
	.bIsMetalfixed        = 0,
	.boutput_only		  = 1,
};

struct resource ak39_codec_resources[] = {
	[0] = {
		.start = 0x08000000,
		.end = 0x0800FFFF,
		.flags = (int)IORESOURCE_MEM,
		.name = "akpcm_AnalogCtrlRegs",
	},
	[1] = {
		.start = 0x20110000,
		.end = 0x2011800F,
		.flags = (int)IORESOURCE_MEM,
		.name = "akpcm_ADC2ModeCfgRegs",
	},
};

struct platform_device ak39_codec_device = {
	.name	= "ak39-codec",
	.id	= -1,
	.resource = ak39_codec_resources,
	.num_resources = ARRAY_SIZE(ak39_codec_resources),
	.dev	= {
		.platform_data	= &ak39_codec_pdata,
	},
};


/**
 * @brief: sensor device  platform data, this info is fake.
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct i2c_board_info ak_camara_devices[] = {
	{
		I2C_BOARD_INFO("aksensor", 0x1),
	},
};

static struct aksensor_camera_info ak_soc_camera_info = {
	.buswidth = SOCAM_DATAWIDTH_8,
	.pin_avdd = INVALID_GPIO,
	.pin_power = AK_GPIO_48,		//initialize GPIO for the power of camera.
	.pin_reset = AK_GPIO_51,		//initialize GPIO for reset of camera.
	.link = {
		.bus_id = 39,
		.power = NULL,
		.board_info = &ak_camara_devices[0],
		.i2c_adapter_id = 0,
		.priv = &ak_soc_camera_info,
	}	
};

/* fake device for soc_camera subsystem */
static struct platform_device soc_camera_interface = {
	.name = "soc-camera-pdrv",
	.id   = -1,
	.dev = {
		.platform_data = &ak_soc_camera_info.link,
	}
};

/**
 * @brief: LED platform data and initialis status
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct ak_led_data leds[] = {
	{
	.name		= "wps_led",
	.def_trigger	= "none",
	.gpio		= {
		.pin		= AK_GPIO_57,
		.pulldown	= -1,
		.pullup 	= AK_PULLUP_DISABLE,
		.value	= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
		}
	},
	{
	.name		= "vedio_led",
	.def_trigger	= "none",
	.gpio		= {
		.pin		= AK_GPIO_30,
		.pulldown	= -1,
		.pullup 	= AK_PULLUP_DISABLE,
		.value	= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
		}
	},
};

static struct ak_led_pdata led_pdata = {
	.leds		= leds,
	.nr_led		= ARRAY_SIZE(leds),
};


/* unused GPIO number for the machine board is left*/
static unsigned int ak39_custom_gpiopin[] = {
	AK_GPIO_4,
	AK_GPIO_5,
};

static struct custom_gpio_data ak39_custom_gpios= {
	.gpiopin = ak39_custom_gpiopin,
	.ngpiopin = ARRAY_SIZE(ak39_custom_gpiopin),
};

static struct platform_device ak39_custom_gpio = {
	.name = "akgpio",
	.id = -1,
	.dev = {
		.platform_data = &ak39_custom_gpios,
	},
};


/**
 * @brief: GPIO buttons platform data and initialis status
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct gpio_keys_button gpio_keys_button[] = {
	{
		.code			= KEY_0,
		.type			= EV_KEY,
		.gpio			= AK_GPIO_0,
		.active_low		= 1,
		.wakeup			= 1,
		.debounce_interval	= 30, /* ms */
		.desc			= "boot0",
		.pullup			= AK_PULLUP_ENABLE,
		.pulldown		= -1,
		.dir			= AK_GPIO_DIR_INPUT,
		.int_pol		= AK_GPIO_INT_LOWLEVEL,
	},
};

static struct akgpio_keys_platform_data gpio_keys_platform_data = {
	.buttons	= gpio_keys_button,
	.nbuttons	= ARRAY_SIZE(gpio_keys_button),
	.rep		= 0,
};


/**
* @brief		ad-key platform device struct
			we should initialize the correct voltage for each key.
* @author: caolianming
* @date: 2014-01-09
*/
struct multi_addetect multi_det[] = {
	{.unpress_min = 3220, .unpress_max = 3258, .fixkeys = NULL, .plugdev = PLUGIN_NODEV},	// = null		3251+/-20	
	{.unpress_min = 3259, .unpress_max = 3300, .fixkeys = NULL, .plugdev = PLUGIN_AC},		// = sddet	3261(+/-30)
	{.unpress_min = 2230, .unpress_max = 2290, .fixkeys = NULL, .plugdev = PLUGIN_MMC},		// = sddet	2262(+/-30)
	{.unpress_min = 1700, .unpress_max = 1780, .fixkeys = NULL, .plugdev = PLUGIN_MMC_AC},	// = sddet	1742(+/-30)
};

struct analog_gpio_key ak39_adkey_data = {
	.desc = "adkey",
	.interval = 300,			/* ms */
	.debounce_interval = 20,	/* ms */
	.addet = multi_det,
	.naddet = ARRAY_SIZE(multi_det),
	.nkey = 0,
	.wakeup = 1,				/* enable ad-key wakeup from standby mode */
};

static struct platform_device ak39_adkey_device = {
	.name = "ad-keys",
	.id = -1,
	.dev = {
		.platform_data = &ak39_adkey_data,
	}
};


/**
 * @brif: ak39 battery mach info
 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct ak_bat_mach_info ak39_bat_info = {
	.gpio_init	= ak_gpio_set,
	.usb_gpio	= {
		.active 	= -1,
		.irq		= -ENOSYS, 
		.delay		= 0,
		.pindata	={
			.pin		= -1,
			.pulldown	= -1,
			.pullup 	= -1,
			.value		= -1,
			.dir		= -1,
			.int_pol	= -1,
		},	
	},
	
	.ac_gpio	= { 
		.is_detect_mode = BAT_CHARGE_ADC_DETECT,
		.active 	= -1,
		.irq		= -1, 
		.delay		= 500,
		.pindata	={
			.pin		= -1,
			.pulldown	= -1,
			.pulldown	= -1,
			.value	= -1,
			.dir		= -1,
			.int_pol	= -1,
		},	
	},

	.full_gpio = {
		.active 	= -1,
		.irq		= -ENOSYS, 
		.delay		= 0,
		.pindata	={
			.pin		= -1,
			.pulldown	= -1,
			.pullup 	= -1,
			.value		= -1,
			.dir		= -1,
			.int_pol	= -1,
		},	
	},
	
	.bat_mach_info	= {
		.voltage_sample 		= 6,			// the sample of read voltage
		.power_on_voltage		= 3650, 		// discharge power on voltage limit
		.power_on_correct		= 64,		// mv		
		.charge_min_voltage	= 3550,		// charge minute voltage (mv)
		.max_voltage			= 4200, 		// max battery voltage	
		.min_voltage			= 3500, 		// min battery voltage
		.power_off			= poweroff_disable,
		.full_capacity			= 100,		// battery full 
		.poweroff_cap			= 0,			// user read value to power off
		.low_cap				= 5,			// user read value to low power warring
		.recover_cap			= 30,
		.cpower_on_voltage	= 3700, 		// charge power on voltage limit
		.full_delay 			= 30,		// unit is minute
		.full_voltage			= 4100,
	},

	.bat_adc	= {
		.up_resistance		= 10,
		.dw_resistance		= 10,
		.voltage_correct		= 32,			// battery correct factor
		.adc_avdd			= 3300,			// avdd voltage
	},

};


/**
 * @brief: camera platform data and initialis status
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct ak_camera_pdata ak39_camera_info = {
	.mclk = 24,
	.gpio_get = ak_gpio_getpin,
	.gpio_set = ak_gpio_set,
	.rf_led = {
		.pin = -1,
		.pulldown = -1,
		.pullup = AK_PULLUP_DISABLE,
		.value = -1,
		.dir = AK_GPIO_DIR_INPUT,
		.int_pol = AK_GPIO_INT_LOWLEVEL,
	}
};

/**
 * @brief: ak39 platform devices table
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct platform_device *ak3910_platform_devices[] __initdata = {
	&akfha_char_device,
	&ak39_uart0_device,
	&ak39_motor0_device,
	&ak39_motor1_device,
	&ak39_spi1_device,
	&ak39_mmc_device,
	&ak39_sdio_device,
	&ak39_i2c_device,
	&ak39_custom_gpio,
	&ak39_usb_udc_device,
	&ak39_usb_otg_hcd_device,
	&anyka_wifi_device,
	&soc_camera_interface,
	&ak39_camera_interface,	
	&ak39_ion_device,
	&ak39_pcm_device,
	&ak39_codec_device,
	&ak39_mmx_device,
	&ak39_mac_device, 
	&ak39_led_pdev,
	&ak39_gpio_keys_device,
	&ak39_adkey_device,
	&ak39_battery_power,
	&ak39_rtc_device,
	&ak39_crypto_device,
	&ak39_gpio_uart_device,
};

void wdt_enable(void);
void wdt_keepalive(unsigned int heartbeat);


/**
 * @brief: restart by "reboot" cmd 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static void ak39_restart(char str, const char *cmd)
{
	//ak39_reboot_sys_by_soft();
#if defined CONFIG_AK39_WATCHDOG || defined CONFIG_AK39_WATCHDOG_TOP
	wdt_enable();
	wdt_keepalive(2);
#endif
}


/**
 * @brief: initial ak3916 machine 
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static void __init ak3910_init_machine(void)
{
	adc1_init();

	spi_register_board_info(ak39_spi_board_dev, ARRAY_SIZE(ak39_spi_board_dev));	
		
	ak39_spi1_device.dev.platform_data = &ak39_spi1_info;

	ak39_motor0_device.dev.platform_data = &ak39_motor0_pdata;
	ak39_motor1_device.dev.platform_data = &ak39_motor1_pdata;
		
	ak39_mmc_device.dev.platform_data = &mmc_plat_data;
	ak39_sdio_device.dev.platform_data = &sdio_plat_data;

	ak39_crypto_device.dev.platform_data = &akcrypto_pdata;

	ak39_usb_otg_hcd_device.dev.platform_data = &akotghc_plat_data;
	ak39_mac_device.dev.platform_data = &ak39_mac_pdata;

	ak39_led_pdev.dev.platform_data = &led_pdata;
	ak39_gpio_keys_device.dev.platform_data = &gpio_keys_platform_data;	
	ak39_battery_power.dev.platform_data = &ak39_bat_info;

	ak39_camera_interface.dev.platform_data = &ak39_camera_info;
	
	platform_add_devices(ak3910_platform_devices, 
				ARRAY_SIZE(ak3910_platform_devices));
	
	l2_init();
	
	return;
}


MACHINE_START(AK39XX, "Aimer39_AK3916_MB_V1.0.0")
/* Maintainer: */
	.atag_offset = 0x100,
	.fixup = NULL,
	.map_io = ak39_map_io,
	.reserve = NULL,
	.init_irq = ak39_init_irq,
	.init_machine = ak3910_init_machine,
	.init_early = NULL,
	.timer = &ak39_timer, 
    .restart = ak39_restart,
    
MACHINE_END

