#ifndef _MACH_DEVICES_H
#define _MACH_DEVICES_H


extern struct platform_device ak39_uart0_device;
extern struct platform_device ak39_uart1_device;
extern struct platform_device ak39_gpio_uart_device;

extern struct platform_device ak39_mmc_device;
extern struct platform_device ak39_sdio_device;

extern struct platform_device ak39_i2c_device;

extern struct platform_device ak39_usb_udc_device;
extern struct platform_device ak39_usb_otg_hcd_device;
extern struct platform_device ak39_mac_device;

extern struct platform_device ak39_spi1_device;
extern struct platform_device ak39_camera_interface;
extern struct platform_device ak39_pcm_device;
extern struct platform_device ak39_mmx_device;

extern struct platform_device ak39_ion_device;
extern struct platform_device ak39_led_pdev;
extern struct platform_device ak39_gpio_keys_device;
extern struct platform_device ak39_battery_power;
extern struct platform_device ak39_rtc_device;

extern struct platform_device akfha_char_device;

extern struct platform_device ak39_motor0_device;
extern struct platform_device ak39_motor1_device;

extern struct platform_device ak39_crypto_device;

#endif /* endif _MACH_DEVICES_H */

