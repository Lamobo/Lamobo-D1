/* 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __AK39_IIC_H
#define __AK39_IIC_H __FILE__

#include <asm/gpio.h>

#define I2C_CLKPIN			(27)
#define I2C_DATPIN			(28)
#define I2C_INTPIN			(24)

#define AK39_I2C_NACKEN		(1 << 17)
#define AK39_I2C_NOSTOP		(1 << 16)
#define AK39_I2C_ACKEN		(1 << 15)
#define AK39_I2C_START		(1 << 14)
#define AK39_I2C_TXRXSEL	(1 << 13)
#define AK39_I2C_TRX_BYTE	(9)
#define AK39_I2C_CLR_DELAY	(0x3 << 7)
#define AK39_I2C_SDA_DELAY	(0x2 << 7)
#define AK39_I2C_TXDIV_512	(1 << 6)
#define AK39_I2C_INTEN		(1 << 5)

#define INT_PEND_FLAG		(1 << 4)
#define AK39_TX_CLK_DIV		(0xf)

#define	AK39_I2C_CMD_EN			(1 << 18)
#define AK39_I2C_START_BIT		(1 << 17)

#define AK39_I2C_READ		1
#define AK39_I2C_WRITE		0
	
#define AK39_I2C_CTRL		REG_VA_ADDR(AK_VA_I2C, 0x00)
#define AK39_I2C_CMD1		REG_VA_ADDR(AK_VA_I2C, 0x10)
#define AK39_I2C_CMD2		REG_VA_ADDR(AK_VA_I2C, 0x14)
#define AK39_I2C_CMD3		REG_VA_ADDR(AK_VA_I2C, 0x18)
#define AK39_I2C_CMD4		REG_VA_ADDR(AK_VA_I2C, 0x1C)
#define AK39_I2C_DATA0		REG_VA_ADDR(AK_VA_I2C, 0x20)
#define AK39_I2C_DATA1		REG_VA_ADDR(AK_VA_I2C, 0x24)
#define AK39_I2C_DATA2		REG_VA_ADDR(AK_VA_I2C, 0x28)
#define AK39_I2C_DATA3		REG_VA_ADDR(AK_VA_I2C, 0x2C)


struct ak39_platform_i2c {
	int		bus_num;
	unsigned int	flags;
	unsigned int	slave_addr;
	unsigned long	frequency;
	unsigned int	sda_delay;
	struct gpio_info *gpios;
	int npins;
};

#endif	/* __AK39_IIC_H */
