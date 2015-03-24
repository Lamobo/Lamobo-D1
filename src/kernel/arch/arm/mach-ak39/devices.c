/* linux/arch/arm/mach-ak39/devices.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <plat-anyka/ak_camera.h>

#include <asm/irq.h>
#include <asm/gpio.h>
#include <mach/i2c.h>
#include <linux/ion.h>
#include <linux/uio_driver.h>
#include <linux/akuio_driver.h>

/**
 * @brief: uart0 device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_uart0_device = {
    .name   = "ak39-uart",
    .id     = 0,
};
EXPORT_SYMBOL(ak39_uart0_device);

/**
 * @brief: uart1 device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_uart1_device = {
    .name   = "ak39-uart",
    .id     = 1,
};
EXPORT_SYMBOL(ak39_uart1_device);

/**
 * @brief: gpio uart device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_gpio_uart_device = {
    .name   = "gpio-uart",
    .id     = 0,
};
EXPORT_SYMBOL(ak39_gpio_uart_device);


/**
 * @brief: MCI device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource ak39_mmc_resource[] = {
	[0] = {
		.start = 0x20100000,
		.end = 0x20100000 + 0x43,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_MCI,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device ak39_mmc_device = {
	.name = "ak_mci",
	.id = -1,
	.num_resources = ARRAY_SIZE(ak39_mmc_resource),
	.resource = ak39_mmc_resource,
};
EXPORT_SYMBOL(ak39_mmc_device);

/**
 * @brief: SDIO device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource ak39_sdio_resource[] = {
	[0] = {
		.start = 0x20108000,
		.end = 0x20108000 + 0x43,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SDIO,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device ak39_sdio_device = {
	.name = "ak_sdio",
	.id = -1,
	.num_resources = ARRAY_SIZE(ak39_sdio_resource),
	.resource = ak39_sdio_resource,
};
EXPORT_SYMBOL(ak39_sdio_device);


/**
 * @brief: I2C device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
#if defined(CONFIG_I2C_AK39_HW)
struct gpio_info i2c_gpios[] = {
	{
		.pin 		= AK_GPIO_27,
		.pulldown 	= -1,
		.pullup 	= AK_PULLUP_DISABLE,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.value 		= AK_GPIO_OUT_HIGH,
		.int_pol	= -1,
	},
	{
		.pin 		= AK_GPIO_28,
		.pulldown 	= -1,
		.pullup 	= AK_PULLUP_DISABLE,
		.dir		= AK_GPIO_DIR_OUTPUT,
		.value 		= AK_GPIO_OUT_HIGH,
		.int_pol	= -1,

	},
};

static struct ak39_platform_i2c ak39_default_i2c_data = {
	.flags		= 0,
	.bus_num	= 0,
	.slave_addr	= 0x10,
	.frequency	= 100*1000,
	.sda_delay	= 100,
	.gpios		= i2c_gpios,
	.npins		= ARRAY_SIZE(i2c_gpios),
};

static struct resource ak39_i2c_resource[] = {
	[0] = {
		.start = 0x20150000,
		.end   = 0x20150000+SZ_256,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_I2C,
		.end   = IRQ_I2C,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device ak39_i2c_device = {
	.name	= "i2c-ak39",
	.id		= -1,
	.dev	= {
		.platform_data = &ak39_default_i2c_data,
	},
	.num_resources	= ARRAY_SIZE(ak39_i2c_resource),
	.resource		= ak39_i2c_resource,
};
EXPORT_SYMBOL(ak39_i2c_device);

#elif defined(CONFIG_I2C_GPIO_SOFT)
struct i2c_gpio_platform_data ak39_i2c_data={
	.sda_pin = INVALID_GPIO,
	.scl_pin = INVALID_GPIO,
	.udelay = 10,
	.timeout = 200
};

struct platform_device ak39_i2c_device = {
	.name	= "i2c-gpio",
	.id		= -1,
	.dev	= {
		.platform_data = &ak39_i2c_data,
	},
};
EXPORT_SYMBOL(ak39_i2c_device);
#else
struct platform_device ak39_i2c_device = {
	.name   = "i2c",
	.id     = -1,
};
EXPORT_SYMBOL(ak39_i2c_device);
#endif


/**
 * @brief: USB udc device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource usb_otg_udc_resource[] = {
	[0] = {
		.start	= 0x20200000,
		.end	= 0x202003ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "usb mcu irq",
		.start	= IRQ_USBOTG_MCU,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "usb dma irq",
		.start	= IRQ_USBOTG_DMA,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device ak39_usb_udc_device = {
	.name = "ak-hsudc",
	.id = -1,
	.num_resources = ARRAY_SIZE(usb_otg_udc_resource),
	.resource = usb_otg_udc_resource,
};
EXPORT_SYMBOL(ak39_usb_udc_device);


/**
 * @brief: USB otg host device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource usb_otg_hcd_resource[] = {
	[0] = {
		.start	= 0x20200000,
		.end	= 0x202003ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "usb mcu irq",
		.start	= IRQ_USBOTG_MCU,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "usb dma irq",
		.start	= IRQ_USBOTG_DMA,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device ak39_usb_otg_hcd_device = {
	.name = "usb-host",
	.id = -1,
	.num_resources = ARRAY_SIZE(usb_otg_hcd_resource),
	.resource = usb_otg_hcd_resource,
};
EXPORT_SYMBOL(ak39_usb_otg_hcd_device);

/**
 * @brief: MAC device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource ak39_mac_resource[] = {
	[0] = {
	   .start = 0x20300000,
	   .end = 0x20301fff,
	   .flags = IORESOURCE_MEM,
	},
	[1] = {
	   .name = "mac irq",
	   .start = IRQ_MAC,
	   .flags = IORESOURCE_IRQ,
	},
};

struct platform_device ak39_mac_device = {
	.name = "ak_ethernet",
	.id = 0,
	.num_resources = ARRAY_SIZE(ak39_mac_resource),
	.resource = ak39_mac_resource,
};
EXPORT_SYMBOL(ak39_mac_device);

/**
 * @brief: SPI device info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource ak39_spi1_resource[] = {
	[0] = {
		.start = 0x20120000,
		.end = 0x20120027,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_SPI1,
		.end = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ak39_spi1_device = {
	.name = "ak-spi",
	.id = -1,
	.num_resources = ARRAY_SIZE(ak39_spi1_resource),
	.resource = ak39_spi1_resource,
};
EXPORT_SYMBOL(ak39_spi1_device);


/**
 * @brief: Camera interface resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct resource ak39_camera_resource[] = {
	[0] = {
		.start = 0x20000000,
		.end = 0x20000030,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "camera if irq",
		.start = IRQ_CAMERA,
		.flags = IORESOURCE_IRQ,
	},
};

/* camera interface */
struct platform_device ak39_camera_interface = {
	.name = "ak_camera",
	.id   = 39,
	.num_resources	= ARRAY_SIZE(ak39_camera_resource),	
	.resource = ak39_camera_resource,	
};

EXPORT_SYMBOL(ak39_camera_interface);


/**
 * @brief: PCM device resource info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static u64 snd_dma_mask = DMA_BIT_MASK(32);

struct platform_device ak39_pcm_device = {
	.name = "snd_akpcm",
	.id = 0,
	.dev = {
		.dma_mask	   = &snd_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};
EXPORT_SYMBOL(ak39_pcm_device);


/**
 * @brief: ION device resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
static struct ion_platform_data ak39_ion_pdata = {
	.nr = 1,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = 1,
			.name = "Reserved phys Memory",
			.base = CONFIG_RAM_BASE,
			.size = CONFIG_VIDEO_RESERVED_MEM_SIZE, /* the first reserved size */
			.align	= PAGE_SIZE,
		},
	}
};
		
struct platform_device ak39_ion_device = {
	.name = "ion-ak",
	.id = -1,
	.dev = {
		.platform_data = &ak39_ion_pdata,
	},
};
EXPORT_SYMBOL(ak39_ion_device);

/**
 * @brief:  LDE device resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_led_pdev = {
	.name		= "ak_led",
	.id		= -1,
};
EXPORT_SYMBOL(ak39_led_pdev);

/**
 * @brief:  fha device resource info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
struct platform_device akfha_char_device = {
	.name = "ak-fhachar",
	.id = -1,
	.dev = {
		.platform_data = NULL,
	},
};
EXPORT_SYMBOL(akfha_char_device);

/**
 * @brief:  gpio buttons device resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_gpio_keys_device = {
	.name	= "akgpio-keys",
	.id	= -1,
};
EXPORT_SYMBOL(ak39_gpio_keys_device);

/**
 * @brief:  battery device resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_battery_power = {
	.name = "battery",
	.id   = -1,
};
EXPORT_SYMBOL(ak39_battery_power);

static struct resource ak39mmx_resources[] = {
	{
		.name   = "video-base",
		.start	= 0x20020000,
		.end	= 0x2002042f,
		.flags	= IORESOURCE_MEM,
	},
};

static struct uio_info akmmx_uioinfo = {
	.name    = "video_codec",
	.version = "0.1.0",
#ifdef CONFIG_UIODMA
	.use_dma = true,
#endif
	.irq     = UIO_IRQ_CUSTOM,
};

struct platform_device ak39_mmx_device = {
	.name		= "uio_vcodec",
	.id		= 0,
	.dev		= {
		.platform_data = &akmmx_uioinfo,
	},
	.num_resources	= ARRAY_SIZE(ak39mmx_resources),
	.resource	= ak39mmx_resources,
};
EXPORT_SYMBOL(ak39_mmx_device);

/**
 * @brief:  rtc device resource info
 * 
 * @author: caolianming
 * @date: 2014-01-09
 */
struct platform_device ak39_rtc_device = {
	.name = "ak-rtc",
	.id = -1,
};
EXPORT_SYMBOL(ak39_rtc_device);

/**
 * @brief: motor0  device resource info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
struct platform_device ak39_motor0_device = {
	.name = "ak-motor",
	.id = 0,
};
EXPORT_SYMBOL(ak39_motor0_device);

/**
 * @brief:  motor1 device resource info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
struct platform_device ak39_motor1_device = {
	.name = "ak-motor",
	.id = 1,
};
EXPORT_SYMBOL(ak39_motor1_device);

/**
 * @brief:  Crypto device resource info
 * 
 * @author: lixinhai
 * @date: 2014-01-09
 */
static struct resource ak39_crypto_resource[] = {
	[0] = {
	       .start = 0x20180000,
	       .end = 0x20180067,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = IRQ_ENCRYTION,
	       .end = IRQ_ENCRYTION,
	       .flags = IORESOURCE_IRQ,
	       }
};

struct platform_device ak39_crypto_device = {
	.name = "ak-crypto",
	.id = -1,
	.num_resources = ARRAY_SIZE(ak39_crypto_resource),
	.resource = ak39_crypto_resource,
};
EXPORT_SYMBOL(ak39_crypto_device);

