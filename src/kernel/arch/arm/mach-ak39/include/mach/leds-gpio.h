/* arch/arm/mach-ak98/include/mach/leds-gpio.h
 *
 * Copyright (c) Anyka
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_LEDSGPIO_H
#define __ASM_ARCH_LEDSGPIO_H "leds-gpio.h"

#define AK_LEDF_ACTLOW   	(1<<0)		/* LED is on when GPIO low */
#define AK_LEDF_ACTHIGH   	(1<<1)		/* LED is on when GPIO hight */
#define AK_LEDF_TRISTATE	(1<<2)		/* tristate to turn off */

struct ak_led_data {
	char			*name;
	char			*def_trigger;
	struct gpio_info	gpio;
};

struct ak_led_pdata {
	struct ak_led_data	*leds;
	int			nr_led;
};

#endif /* __ASM_ARCH_LEDSGPIO_H */
