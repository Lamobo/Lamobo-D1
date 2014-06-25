/**
*  @file      arch/arm/mach-ak39/gpio.c
*  @brief     dispatch the call of GPIO API
*   Copyright C 2011 Anyka CO.,LTD
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*  @author    zhou wenyong
*  @date      2011-08-11
*  @note      2010-10-21 created by caolianming
*  @note      2011-06-14 move this file of old version to the ak39-gpio.c
*                        -- unify the GPIO API
*  @note      2010-08-11 add more comments and change some identifiers' name
*/
#include <linux/module.h>
#include <mach/irqs.h>
#include <mach/gpio.h>

/*
	Look up table in which we look up the implementation function of GPIO API by
	gpio pin
*/
static struct gpio_api_lut ak39_gpio_api_lut[] =
{
	{
		.pin_start 		= AK_GPIO_MIN,
		.pin_end 		= AK_GPIO_MAX,
		.setpin_as_gpio	= g_ak39_setpin_as_gpio,
		.gpio_pullup	= g_ak39_gpio_pullup,
		.gpio_pulldown	= g_ak39_gpio_pulldown,
		.gpio_dircfg	= g_ak39_gpio_cfgpin,
		.gpio_intcfg	= g_ak39_gpio_inten,
		.gpio_set_intpol= g_ak39_gpio_intpol,
		.gpio_setpin	= g_ak39_gpio_setpin,
		.gpio_getpin	= g_ak39_gpio_getpin,
		.gpio_to_irq	= g_ak39_gpio_to_irq,
		.irq_to_gpio	= g_ak39_irq_to_gpio,			
	},
};

#define GPIO_API_LUT ak39_gpio_api_lut
/*
    Look up the table to get the implementation function of corresponding 
    GPIO API
*/
#define GET_GPIO_FUNC(pfun, pin, name)\
	do{\
	int _i_name;\
	pfun=NULL;\
	for (_i_name=0; _i_name<ARRAY_SIZE(GPIO_API_LUT); _i_name++)\
	if (pin>= GPIO_API_LUT[_i_name].pin_start && pin <= GPIO_API_LUT[_i_name].pin_end)\
	{ pfun = GPIO_API_LUT[_i_name].name; break;}\
	}while(0);
										


/**
*  @brief       set pin as GPIO port
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @return      int
*/
int ak_setpin_as_gpio(unsigned int pin)
{
	int (*pfunc)(unsigned int);
	GET_GPIO_FUNC(pfunc, pin, setpin_as_gpio);
	if (pfunc)
		return pfunc(pin);
	return 0;
}
EXPORT_SYMBOL(ak_setpin_as_gpio);


/**
*  @brief       configure GPIO PULLUP function
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   enable
*  @return      int
*/
int ak_gpio_pullup(unsigned int pin, unsigned char enable)
{
	int (*pfunc)(unsigned int, unsigned char);
	GET_GPIO_FUNC(pfunc, pin, gpio_pullup);
	if (pfunc)
		return pfunc(pin, enable);	

	return 0;
}
EXPORT_SYMBOL(ak_gpio_pullup);

/**
*  @brief       configure GPIO PULLDOWN function
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   enable
*  @return      int
*/
int ak_gpio_pulldown(unsigned int pin, unsigned char enable)
{
	int (*pfunc)(unsigned int, unsigned char);
	GET_GPIO_FUNC(pfunc, pin, gpio_pulldown);
	if (pfunc)
		return pfunc(pin, enable);	

	return 0;
}
EXPORT_SYMBOL(ak_gpio_pulldown);


/**
*  @brief       configure GPIO direction 
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   direction
*  @return      int
*/
int ak_gpio_dircfg(unsigned int pin, unsigned int direction)
{
	int (*pfunc)(unsigned int, unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_dircfg);
	if (pfunc)
		return pfunc(pin, direction);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_dircfg);
/**
*  @brief       old version of ak_gpio_dircfg
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   to
*  @return      int
*/
int ak_gpio_cfgpin(unsigned int pin, unsigned int to)
{
	return ak_gpio_dircfg(pin, to);
}
EXPORT_SYMBOL(ak_gpio_cfgpin);


/**
*  @brief       enable/disable GPIO interrupt
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   enable
*  @return      int
*/
int ak_gpio_intcfg(unsigned int pin, unsigned int enable)
{
	int (*pfunc)(unsigned int, unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_intcfg);
	if (pfunc)
		return pfunc(pin, enable);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_intcfg);
/**
*  @brief       old version of ak_gpio_intcfg
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   enable
*  @return      int
*/
int ak_gpio_inten(unsigned int pin, unsigned int enable)
{
	return ak_gpio_intcfg(pin, enable);
}
EXPORT_SYMBOL(ak_gpio_inten);


/**
*  @brief       configure GPIO interrupt polarity
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   level
*  @return      int
*/
int ak_gpio_set_intpol(unsigned int pin, unsigned int level)
{
	int (*pfunc)(unsigned int, unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_set_intpol);
	if (pfunc)
		return pfunc(pin, level);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_set_intpol);
/**
*  @brief       old version of ak_gpio_set_intpol
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   level
*  @return      int
*/
int ak_gpio_intpol(unsigned int pin, unsigned int level)
{
	return ak_gpio_set_intpol(pin, level);
}
EXPORT_SYMBOL(ak_gpio_intpol);	


/**
*  @brief       configure GPIO output state
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @param[in]   to
*  @return      int
*/
int ak_gpio_setpin(unsigned int pin, unsigned int to)
{
	int (*pfunc)(unsigned int, unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_setpin);
	if (pfunc)
		return pfunc(pin, to);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_setpin);

/**
*  @brief       read GPIO input state
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @return      int
*/
 int ak_gpio_getpin(unsigned int pin)
{	
	 int (*pfunc)(unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_getpin);
	if (pfunc)
		return pfunc(pin);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_getpin);


/**
*  @brief       get corresponding irq by pin
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   pin
*  @return      int
*/
 int ak_gpio_to_irq(unsigned int pin)
{
	int (*pfunc)(unsigned int);
	GET_GPIO_FUNC(pfunc, pin, gpio_to_irq);
	if (pfunc)
		return pfunc(pin);

	return 0;
}
EXPORT_SYMBOL(ak_gpio_to_irq);

/**
*  @brief       get corresponding gpio pin by irq
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[in]   irq
*  @return      int
*/
 int ak_irq_to_gpio(unsigned int irq)
{
	if ( irq >= IRQ_GPIO_0 && irq<= NR_IRQS - 1)
		return AK_GPIO_0 + (irq-IRQ_GPIO_0);
	else
		panic("wrong irq number %u passed to ak_irq_to_gpio.\n", irq);

	return -1;
}
EXPORT_SYMBOL(ak_irq_to_gpio);


/**
*  @brief       configure GPIO followed properties specified in info 
*  @author      zhou wenyong
*  @date        2011-08-11
*  @param[out]  *info
*  @return      void
*/
void ak_gpio_set(const struct gpio_info *info)
{
	if ( ! (info->pin >= AK_GPIO_MIN && info->pin <= AK_GPIO_MAX ))
		return ;
	
	ak_setpin_as_gpio(info->pin);
	if (info->dir == AK_GPIO_DIR_OUTPUT || info->dir == AK_GPIO_DIR_INPUT)
		ak_gpio_dircfg(info->pin, info->dir);
	if (info->pullup == AK_PULLUP_ENABLE || info->pullup == AK_PULLUP_DISABLE)
		ak_gpio_pullup(info->pin, info->pullup);
	if (info->pulldown == AK_PULLDOWN_ENABLE || info->pulldown == AK_PULLDOWN_DISABLE)
		ak_gpio_pulldown(info->pin, info->pulldown);
	if (info->value == AK_GPIO_OUT_HIGH || info->value == AK_GPIO_OUT_LOW)
		ak_gpio_setpin(info->pin, info->value);
	if (info->int_pol == AK_GPIO_INT_LOWLEVEL || info->int_pol == AK_GPIO_INT_HIGHLEVEL)
		ak_gpio_set_intpol(info->pin, info->int_pol);
}
EXPORT_SYMBOL(ak_gpio_set);

