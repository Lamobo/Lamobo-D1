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
#include <mach/adc.h>

#include "cpu.h"
#include "irq.h"

#include <mach/reboot.h>

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

static struct flash_platform_data ak39_spiflash_info= {
	.bus_width = FLASH_BUS_WIDTH_4WIRE | FLASH_BUS_WIDTH_2WIRE | FLASH_BUS_WIDTH_1WIRE,
	.type 	   = NULL,
};

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


/* SDIO  platform data*/
struct ak_mci_platform_data sdio_plat_data = {
	.irq_cd_type = IRQ_TYPE_LEVEL_LOW,
	.detect_mode = AKMCI_PLUGIN_ALWAY,
	.xfer_mode = AKMCI_XFER_L2DMA,
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

/* MMC/SD  platform data*/
struct ak_mci_platform_data mmc_plat_data = {
	.irq_cd_type = IRQ_TYPE_LEVEL_LOW,
	.detect_mode = AKMCI_DETECT_MODE_GPIO,	
	.xfer_mode = AKMCI_XFER_L2DMA,
	.mci_mode = MCI_MODE_MMC_SD,
    .gpio_init = ak_gpio_set,
	.max_speed_hz = 25*1000*1000,
    .gpio_cd = {
        .pin = AK_GPIO_30,
        .pulldown = -1,
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

/* akwifi platform data */
struct platform_device anyka_wifi_device = {
	.name = "anyka-wifi",
	.id = -1,
};

static struct akotghc_usb_platform_data akotghc_plat_data = {
	.gpio_init = ak_gpio_set,
	.gpio_pwr_on = {
		.pin = AK_GPIO_6,
		.pulldown = -1,
		.pullup	= AK_PULLUP_DISABLE,
		.value	= AK_GPIO_OUT_HIGH,
		.dir	= AK_GPIO_DIR_OUTPUT,
		.int_pol = -1,
	},	
	.gpio_pwr_off = {
		.pin = AK_GPIO_6,
		.pulldown = -1,
		.pullup	= AK_PULLUP_DISABLE,
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

/* MAC platform data */
static struct ak_mac_data ak39_mac_pdata = {
	.gpio_init = ak_gpio_set,
	.pwr_gpio = {
		.pin		= INVALID_GPIO,
		.pulldown	= AK_PULLDOWN_DISABLE,
		.pullup		= -1,
		.value		= AK_GPIO_OUT_HIGH,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
	},
	.phy_rst_gpio = {
		.pin		= INVALID_GPIO,
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
		.pin        = AK_GPIO_48,
		.dir		= AK_GPIO_DIR_INPUT,
		.pullup		= -1/*AK_PULLUP_DISABLE*/,
		.pulldown	= AK_PULLDOWN_ENABLE,
		.value      = -1,
		.int_pol	= -1,
	},
	.spk_down_gpio =
	{
		.pin        = INVALID_GPIO, //AK_GPIO_16,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.pullup		= AK_PULLUP_ENABLE,
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
	.hpdet_irq            = IRQ_GPIO_48,
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


/* camera platform data */
static struct i2c_board_info ak_camara_devices[] = {
	{
		I2C_BOARD_INFO("aksensor", 0x1),
	},
};

static struct aksensor_camera_info ak_soc_camera_info = {
	.buswidth = SOCAM_DATAWIDTH_8,
	.pin_avdd = INVALID_GPIO,
	.pin_power = AK_GPIO_3,		//initialize GPIO for the power of camera.
	.pin_reset = AK_GPIO_0,		//initialize GPIO for reset of camera.
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

/* Set LED parameter and initialis status */
static struct ak_led_data leds[] = {
#if 0	
	{
	.name		= "wifi_status",
	.def_trigger	= "none",
	.gpio		= {
		.pin		= AK_GPIO_0,
		.pulldown	= -1,
//		.pullup = AK_PULLUP_DISABLE,
		.pullup	= -1,
		.value	= AK_GPIO_OUT_LOW,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.int_pol	= -1,
		}
	}
#endif
};

static struct ak_led_pdata led_pdata = {
	.leds		= leds,
	.nr_led		= ARRAY_SIZE(leds),
};

/**
 * GPIO buttons
 */
static struct gpio_keys_button gpio_keys_button[] = {
#if 0
	{
		.code			= KEY_0,
		.type			= EV_KEY,
		.gpio			= AK_GPIO_0,
		.active_low		= 1,
		.wakeup			= 1,
		.debounce_interval	= 30, /* ms */
		.desc			= "boot0",
		.pullup			= AK_PULLUP_ENABLE,
		.pulldown			= -1,
		.dir				= AK_GPIO_DIR_INPUT,
		.int_pol			= AK_GPIO_INT_LOWLEVEL,
	},
#endif
};

static struct akgpio_keys_platform_data gpio_keys_platform_data = {
	.buttons	= gpio_keys_button,
	.nbuttons	= ARRAY_SIZE(gpio_keys_button),
	.rep		= 0,
};

/**
* @brief		ad-key platform device struct
			we should initialize the correct voltage for each key.
*/
struct adgpio_key adkey[][6] = {
	// = only ad key
	{{ .code = KEY_LEFT,	.min = 0,	 .max = 80},	//20+/-40
	{ .code = KEY_RIGHT,	.min = 140,	 .max = 200},	//174+/-30
	{ .code = KEY_UP,		.min = 400, .max = 480}, 	//444+/-40
	{ .code = KEY_DOWN,		.min = 620, .max = 720},  	//673+/-50
	{ .code = KEY_OK,		.min = 940, .max = 1080},  	//980+/-40
	{ .code = KEY_MENU,		.min = 1230, .max = 1350}},	//1308+/-40
#if 0
	// = key + sd
	{{ .code = KEY_LEFT,	.min = 0,	 .max = 80},	//20+/-40
	{ .code = KEY_RIGHT,	.min = 140,	 .max = 200},	//174+/-30
	{ .code = KEY_UP,		.min = 400, .max = 480}, 	//444+/-40
	{ .code = KEY_DOWN,		.min = 620, .max = 720},  	//673+/-50
	{ .code = KEY_OK,		.min = 940, .max = 1080},  	//980+/-40
	{ .code = KEY_MENU,		.min = 1230, .max = 1350}},	//1308+/-40
#endif
};

struct multi_addetect multi_det[] = {
	{.unpress_min = 3220, .unpress_max = 3300, .fixkeys = adkey[0], .plugdev = PLUGIN_NODEV},	// = key 3296+/-80	
//	{.unpress_min = 1411, .unpress_max = 1591, .fixkeys = adkey[1], .plugdev = PLUGIN_MMC},		// = key+ sddet	1511+/-80)
};

struct analog_gpio_key ak39_adkey_data = {
	.desc = "adkey",
	.interval = 300,
	.debounce_interval = 20,
	.addet = multi_det,
	.naddet = ARRAY_SIZE(multi_det),	/*initialize the number of device*/
	.nkey = ARRAY_SIZE(adkey[0]),
	.wakeup = 1,						/* initialize key have function of wake up chip. */
};

static struct platform_device ak39_adkey_device = {
	.name = "ad-keys",
	.id = -1,
	.dev = {
		.platform_data = &ak39_adkey_data,
	}
};

static struct platform_device *ak3910_platform_devices[] __initdata = {
	&akfha_char_device,
	&ak39_uart0_device,
	&ak39_uart1_device,
	&ak39_spi1_device,
	&ak39_mmc_device,
	//&ak39_sdio_device,
	&ak39_i2c_device,
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
};
void wdt_enable(void);
void wdt_keepalive(unsigned int heartbeat);

static void ak39_restart(char str, const char *cmd)
{
	//ak39_reboot_sys_by_soft();
#if defined CONFIG_AK39_WATCHDOG || defined CONFIG_AK39_WATCHDOG_TOP
	wdt_enable();
	wdt_keepalive(2);
#endif
}

static void __init ak3910_init_machine(void)
{
	adc1_init();
	
	spi_register_board_info(ak39_spi_board_dev, ARRAY_SIZE(ak39_spi_board_dev));	
		
	ak39_spi1_device.dev.platform_data = &ak39_spi1_info;
	
	ak39_mmc_device.dev.platform_data = &mmc_plat_data;
	//ak39_sdio_device.dev.platform_data = &sdio_plat_data;

	ak39_usb_otg_hcd_device.dev.platform_data = &akotghc_plat_data;
	ak39_mac_device.dev.platform_data = &ak39_mac_pdata;

	ak39_led_pdev.dev.platform_data = &led_pdata;
	ak39_gpio_keys_device.dev.platform_data = &gpio_keys_platform_data;	
	
	platform_add_devices(ak3910_platform_devices, 
				ARRAY_SIZE(ak3910_platform_devices));
	
	l2_init();
	
	return;
}


MACHINE_START(AK39XX, "SDK3910 Board")
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

