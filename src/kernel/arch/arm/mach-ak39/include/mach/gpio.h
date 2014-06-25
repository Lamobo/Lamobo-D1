/*************************************************************************
*   Filename: arch/arm/mach-ak39/include/mach/gpio.h
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
**************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__ 

#include <linux/gpio.h>
#include <asm/io.h>
#include "map.h"


#define AK_WAKEUP_ENABLE		1
#define AK_WAKEUP_DISABLE		0
#define AK_FALLING_TRIGGERED	1
#define AK_RISING_TRIGGERED		0

#define	AK_GPIO_DIR_OUTPUT		1
#define	AK_GPIO_DIR_INPUT		0
#define	AK_GPIO_INT_DISABLE		0
#define	AK_GPIO_INT_ENABLE		1
#define	AK_GPIO_INT_LOWLEVEL	0
#define	AK_GPIO_INT_HIGHLEVEL	1

#define	AK_GPIO_OUT_LOW			0
#define	AK_GPIO_OUT_HIGH		1

#define	AK_PULLUP_DISABLE		0
#define	AK_PULLUP_ENABLE		1
#define	AK_PULLDOWN_DISABLE		0
#define	AK_PULLDOWN_ENABLE		1

#define	GPIO_PIN_MODE_GPIO		0
#define	GPIO_PIN_MODE_INT		1

#define	ATTR_FIXED_1			1
#define	ATTR_FIXED_0			0
#define	PIN_ATTE_LINE			6

#undef END_FLAG
#define END_FLAG                0xff
#define INVALID_GPIO			(-1)


#define AK_FALSE		0
#define AK_TRUE			1
#undef AK_NULL
#define AK_NULL			((void *)(0))


/**************** gpio offsets ************************/
#define AK_GPIO_GROUP1		(32*0)
#define AK_GPIO_GROUP2		(32*1)

#define AK_GPIO_GROUP1_NO(offset)		( AK_GPIO_GROUP1 + (offset))
#define AK_GPIO_GROUP2_NO(offset)		( AK_GPIO_GROUP2 + (offset))
		
#define AK_GPIO_0			AK_GPIO_GROUP1_NO(0)
#define AK_GPIO_1			AK_GPIO_GROUP1_NO(1)
#define AK_GPIO_2			AK_GPIO_GROUP1_NO(2)
#define AK_GPIO_3			AK_GPIO_GROUP1_NO(3)
#define AK_GPIO_4			AK_GPIO_GROUP1_NO(4)
#define AK_GPIO_5			AK_GPIO_GROUP1_NO(5)
#define AK_GPIO_6			AK_GPIO_GROUP1_NO(6)
#define AK_GPIO_7			AK_GPIO_GROUP1_NO(7)
#define AK_GPIO_8			AK_GPIO_GROUP1_NO(8)
#define AK_GPIO_9			AK_GPIO_GROUP1_NO(9)
#define AK_GPIO_10			AK_GPIO_GROUP1_NO(10)
#define AK_GPIO_11			AK_GPIO_GROUP1_NO(11)
#define AK_GPIO_12			AK_GPIO_GROUP1_NO(12)
#define AK_GPIO_13			AK_GPIO_GROUP1_NO(13)
#define AK_GPIO_14			AK_GPIO_GROUP1_NO(14)
#define AK_GPIO_15			AK_GPIO_GROUP1_NO(15)
#define AK_GPIO_16			AK_GPIO_GROUP1_NO(16)
#define AK_GPIO_17			AK_GPIO_GROUP1_NO(17)
#define AK_GPIO_18			AK_GPIO_GROUP1_NO(18)
#define AK_GPIO_19			AK_GPIO_GROUP1_NO(19)
#define AK_GPIO_20			AK_GPIO_GROUP1_NO(20)
#define AK_GPIO_21			AK_GPIO_GROUP1_NO(21)
#define AK_GPIO_22			AK_GPIO_GROUP1_NO(22)
#define AK_GPIO_23			AK_GPIO_GROUP1_NO(23)
#define AK_GPIO_24			AK_GPIO_GROUP1_NO(24)
#define AK_GPIO_25			AK_GPIO_GROUP1_NO(25)
#define AK_GPIO_26			AK_GPIO_GROUP1_NO(26)
#define AK_GPIO_27			AK_GPIO_GROUP1_NO(27)
#define AK_GPIO_28			AK_GPIO_GROUP1_NO(28)
#define AK_GPIO_29			AK_GPIO_GROUP1_NO(29)
#define AK_GPIO_30			AK_GPIO_GROUP1_NO(30)
#define AK_GPIO_31			AK_GPIO_GROUP1_NO(31)

#define AK_GPIO_32			AK_GPIO_GROUP2_NO(0)
#define AK_GPIO_33			AK_GPIO_GROUP2_NO(1)
#define AK_GPIO_34			AK_GPIO_GROUP2_NO(2)
#define AK_GPIO_35			AK_GPIO_GROUP2_NO(3)
#define AK_GPIO_36			AK_GPIO_GROUP2_NO(4)
#define AK_GPIO_37			AK_GPIO_GROUP2_NO(5)
#define AK_GPIO_38			AK_GPIO_GROUP2_NO(6)
#define AK_GPIO_39			AK_GPIO_GROUP2_NO(7)
#define AK_GPIO_40			AK_GPIO_GROUP2_NO(8)
#define AK_GPIO_41			AK_GPIO_GROUP2_NO(9)
#define AK_GPIO_42			AK_GPIO_GROUP2_NO(10)
#define AK_GPIO_43			AK_GPIO_GROUP2_NO(11)
#define AK_GPIO_44			AK_GPIO_GROUP2_NO(12)
#define AK_GPIO_45			AK_GPIO_GROUP2_NO(13)
#define AK_GPIO_46			AK_GPIO_GROUP2_NO(14)
#define AK_GPIO_47			AK_GPIO_GROUP2_NO(15)
#define AK_GPIO_48			AK_GPIO_GROUP2_NO(16)
#define AK_GPIO_49			AK_GPIO_GROUP2_NO(17)
#define AK_GPIO_50			AK_GPIO_GROUP2_NO(18)
#define AK_GPIO_51			AK_GPIO_GROUP2_NO(19)
#define AK_GPIO_52			AK_GPIO_GROUP2_NO(20)
#define AK_GPIO_53			AK_GPIO_GROUP2_NO(21)
#define AK_GPIO_54			AK_GPIO_GROUP2_NO(22)
#define AK_GPIO_55			AK_GPIO_GROUP2_NO(23)
#define AK_GPIO_56			AK_GPIO_GROUP2_NO(24)
#define AK_GPIO_57			AK_GPIO_GROUP2_NO(25)
#define AK_GPIO_58			AK_GPIO_GROUP2_NO(26)
#define AK_GPIO_59			AK_GPIO_GROUP2_NO(27)
#define AK_GPIO_60			AK_GPIO_GROUP2_NO(28)
#define AK_GPIO_61			AK_GPIO_GROUP2_NO(29)
#define AK_GPIO_62			AK_GPIO_GROUP2_NO(30)
#define AK_GPIO_63			AK_GPIO_GROUP2_NO(31)


#define AK_GPIO_MIN				AK_GPIO_0
#define AK_GPIO_MAX				AK_GPIO_63
#define GPIO_UPLIMIT			AK_GPIO_MAX

/******************  access gpio register addr **********************/
#define AK_GPIO_DIR1			(AK_VA_GPIO + 0x00)
#define AK_GPIO_DIR2			(AK_VA_GPIO + 0x04)

#define AK_GPIO_OUT1			(AK_VA_GPIO + 0x08)  
#define AK_GPIO_OUT2			(AK_VA_GPIO + 0x0C) 

#define AK_GPIO_INPUT1         	(AK_VA_GPIO + 0x10) 
#define AK_GPIO_INPUT2         	(AK_VA_GPIO + 0x14) 

#define AK_GPIO_INT_MASK1      	(AK_VA_GPIO + 0x18) 
#define AK_GPIO_INT_MASK2      	(AK_VA_GPIO + 0x1C) 

#define AK_GPIO_INT_MODE1      	(AK_VA_GPIO + 0x20)
#define AK_GPIO_INT_MODE2      	(AK_VA_GPIO + 0x24) 

#define AK_GPIO_INTP1         	(AK_VA_GPIO + 0x28)
#define AK_GPIO_INTP2          	(AK_VA_GPIO + 0x2C)

#define AK_GPIO_EDGE_STATUS1	(AK_VA_GPIO + 0x30)
#define AK_GPIO_EDGE_STATUS2	(AK_VA_GPIO + 0x34)

#define AK_PPU_PPD1           	(AK_VA_SYSCTRL + 0x80)
#define AK_PPU_PPD2           	(AK_VA_SYSCTRL + 0x84)
#define AK_PPU_PPD3           	(AK_VA_SYSCTRL + 0x88)

#define AK_SHAREPIN_CON1		(AK_VA_SYSCTRL + 0x74)
#define AK_SHAREPIN_CON2		(AK_VA_SYSCTRL + 0x78)
#define AK_SHAREPIN_CON3		(AK_VA_SYSCTRL + 0x7C)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AK_GPIO_DIR_BASE(pin)			(((pin)>>5)*4 + AK_GPIO_DIR1)
#define AK_GPIO_OUT_BASE(pin)			(((pin)>>5)*4 + AK_GPIO_OUT1)

#define AK_GPIO_IN_BASE(pin)			(((pin)>>5)*4 + AK_GPIO_INPUT1)
#define AK_GPIO_INTEN_BASE(pin)			(((pin)>>5)*4 + AK_GPIO_INT_MASK1)
#define AK_GPIO_INTPOL_BASE(pin)		(((pin)>>5)*4 + AK_GPIO_INTP1)
#define AK_PPU_PPD_BASE(pin)			(((pin)>>5)*4 + AK_PPU_PPD1)

/****************** end access gpio register addr **********************/

#define AK_WGPIO_POLARITY			(AK_VA_SYSCTRL + 0x3C)
#define AK_WGPIO_CLEAR				(AK_VA_SYSCTRL + 0x40)
#define AK_WGPIO_ENABLE				(AK_VA_SYSCTRL + 0x44)
#define AK_WGPIO_STATUS				(AK_VA_SYSCTRL + 0x48)

#define AK_PA_WGPIO_POLARITY		(AK_PA_SYSCTRL + 0x3C)
#define AK_PA_WGPIO_CLEAR			(AK_PA_SYSCTRL + 0x40)
#define AK_PA_WGPIO_ENABLE		    (AK_PA_SYSCTRL + 0x44)
#define AK_PA_WGPIO_STATUS		    (AK_PA_SYSCTRL + 0x48)

#undef REG32
#define REG32(_reg)	(*(volatile unsigned long *)(_reg))
#undef REG16 
#define REG16(_reg)	(*(volatile unsigned short *)(_reg))

#define AK_GPIO_UART1_FLOW(x)	REG32(AK_VA_SYSCTRL + 0x74) &= ~(0xa << 20)

/****************** enum defined **********************/
typedef enum {
	 ePIN_AS_GPIO = 0,			 // All pin as gpio
	 ePIN_AS_OPCLK,
	 ePIN_AS_JTAG,				 // share pin as JTAG
	 ePIN_AS_RTCK, 		   	  	 // share pin as watch dog
	 ePIN_AS_I2S,				 // share pin as I2S
	 ePIN_AS_PWM1,				 // share pin as PWM1
	 ePIN_AS_PWM2,				 // share pin as PWM2
 	 ePIN_AS_PWM3,				 // share pin as PWM3
	 ePIN_AS_PWM4,				 // share pin as PWM4
	 ePIN_AS_PWM5,				 // share pin as PWM5
	 ePIN_AS_UART1, 			 // share pin as UART1
	 ePIN_AS_UART2, 			 // share pin as UART2
	 ePIN_AS_CAMERA,			 // share pin as CAMERA
	 ePIN_AS_MCI,				 // share pin as MDAT1, 4 lines
	 ePIN_AS_MCI_8LINE,			 // share pin as MDAT1, 8 lines
	 ePIN_AS_SDIO,				 // share pin as SDIO
	 ePIN_AS_SPI1,				 // share pin as SPI1
	 ePIN_AS_SPI2,				 // share pin as SPI2
	 ePIN_AS_MAC,				 // share pin as Ethernet MAC
	 ePIN_AS_I2C,				 // share pin as I2C
 	 ePIN_AS_IRDA,				 // share png as IrDA
	 ePIN_AS_RAM,				 // share pin as RAM Controller

	 ePIN_AS_DUMMY
 } T_GPIO_SHAREPIN_CFG ;

typedef enum {
	SHARE_CONFG1 = 0,
	SHARE_CONFG2
} T_SHARE_CONFG;

typedef enum  {
	SHARE_CFG1 = 0,   // share cfg1
	SHARE_CFG2,       // share cfg2
	SHARE_CFG3,       // share cfg2
	SHARE_CFG12,	  // share cfg1 and share cfg2 as used
	SHARE_CFG13,	  // share cfg1 and share cfg3 as used
	SHARE_CFG23,	  // share cfg2 and share cfg2 as used
	SHARE_CFG123,	  // share cfg1, share config2 and cfg3 as used
	EXIT_CFG		  
}T_SHARE_CFG;

struct gpio_sharepin_cfg {
    T_GPIO_SHAREPIN_CFG func_module;
	T_SHARE_CFG share_config;
    unsigned long reg1_bit_mask;
    unsigned long reg1_bit_value;
    unsigned long reg2_bit_mask;
    unsigned long reg2_bit_value;
    unsigned long reg3_bit_mask;
    unsigned long reg3_bit_value;
};

typedef enum {
	 PULLUP = 0,
	 PULLDOWN,
	 PULLUPDOWN,
	 UNDEFINED
 } T_PUPD_TYPE ;

typedef enum  {
	PUPD_CFG1 = 0,   // share cfg1
	PUPD_CFG2,       // share cfg2
	PUPD_CFG3,       // share cfg2
}T_PUPD_CFG;

typedef enum  {
	AS_GPIO_CFG_BIT1 = 0,   // share cfg1
	AS_GPIO_CFG_BIT2,       // share cfg2
}T_AS_GPIO_CFG;

struct gpio_pupd_cfg {
	int pin;
	int index;
	T_PUPD_CFG pupd_cfg;
	T_PUPD_TYPE pupd_type;
};

struct sharepin_as_gpio {
    unsigned char gpio_start;
    unsigned char gpio_end;
	int index;
	T_AS_GPIO_CFG flag;
};

struct t_gpio_wakeup_cfg {
	unsigned char gpio_start;
	unsigned char gpio_end;
	unsigned char start_bit;
};

struct gpio_info {
	int pin;
	char pulldown;
	char pullup;
	char value;
	char dir;
	char int_pol;	
};

struct gpio_api_lut {
	unsigned int pin_start;
	unsigned int pin_end;

	int (*setpin_as_gpio) (unsigned int pin);
	int (*gpio_pullup)(unsigned int pin, unsigned char enable);
	int (*gpio_pulldown)(unsigned int pin, unsigned char enable);

	int (*gpio_dircfg)(unsigned int pin, unsigned int to);	
	
	int (*gpio_intcfg)(unsigned int pin, unsigned int enable);
	int (*gpio_set_intpol)(unsigned int pin, unsigned int level);

	int (*gpio_setpin)(unsigned int pin, unsigned int to);
	int (*gpio_getpin)(unsigned int pin);

	int (*gpio_to_irq)(unsigned int pin);
	int (*irq_to_gpio)(unsigned int irq);
};

void ak_group_config(T_GPIO_SHAREPIN_CFG mod_name);

/* set gpio's wake up polarity*/
void ak_gpio_wakeup_pol(unsigned int pin, unsigned char pol);
/* enable/disable gpio wake up function*/
int ak_gpio_wakeup(unsigned int pin, unsigned char enable);

int ak_setpin_as_gpio (unsigned int pin);
int ak_gpio_setpin(unsigned int pin, unsigned int to);
int ak_gpio_getpin(unsigned int pin);
int ak_gpio_pullup(unsigned int pin, unsigned char enable);
int ak_gpio_pulldown(unsigned int pin, unsigned char enable);
/* new version of ak_gpio_cfgpin */
int ak_gpio_dircfg(unsigned int pin, unsigned int to);
/* new version of  ak_gpio_inten*/
int ak_gpio_intcfg(unsigned int pin, unsigned int enable);
/* new version of  ak_gpio_intpol*/
int ak_gpio_set_intpol(unsigned int pin, unsigned int level);

/*  to support backward compatibility*/
int ak_gpio_cfgpin(unsigned int pin, unsigned int to);
int ak_gpio_inten(unsigned int pin, unsigned int enable);
int ak_gpio_intpol(unsigned int pin, unsigned int level);
int reg_set_mutli_bit(void __iomem *reg, unsigned int value, int bit, int index);

int ak_gpio_to_irq(unsigned int pin);
int ak_irq_to_gpio(unsigned int irq);

extern int ak_gpio_request(unsigned long gpio, const char *label);
extern void ak_gpio_free(unsigned long gpio);


/*************** wrap gpio interface again ****************/
void ak_gpio_set(const struct gpio_info *info);

int g_ak39_setpin_as_gpio(unsigned int pin);
void g_ak39_setgroup_attribute(T_GPIO_SHAREPIN_CFG mod_name);
int g_ak39_gpio_pullup(unsigned int pin, unsigned char enable);
int g_ak39_gpio_pulldown(unsigned int pin, unsigned char enable);
int g_ak39_gpio_cfgpin(unsigned int pin, unsigned int to);
int g_ak39_gpio_setpin(unsigned int pin, unsigned int to);
int g_ak39_gpio_getpin(unsigned int pin);
int g_ak39_gpio_inten(unsigned int pin, unsigned int enable);
int g_ak39_gpio_intpol(unsigned int pin, unsigned int level);

int g_ak39_gpio_to_irq(unsigned int pin);
int g_ak39_irq_to_gpio(unsigned int irq);

/*************** end wrap gpio interface again ****************/

#endif /* __GPIO_H__ */

