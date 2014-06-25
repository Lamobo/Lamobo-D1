/* arch/arm/arch-ak39/include/mach/map.h
 *
 * AK39 - Memory map definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_MAP_H
#define __ASM_ARCH_MAP_H

#ifndef __ASSEMBLY__
#define AK39_ADDR(x)		((void __iomem *)0xF0000000 + (x))
#else
#define AK39_ADDR(x)		(0xF0000000 + (x))
#endif

#define AK_VA_OCROM			AK39_ADDR(0x00000000)
#define AK_PA_OCROM			0x00000000
#define AK_SZ_OCROM			SZ_32K		/* 32KB */

#define AK_VA_SYSCTRL		AK39_ADDR(0x00008000)
#define AK_PA_SYSCTRL		(0x08000000)
#define AK_SZ_SYSCTRL		SZ_32K		/* 32KB */

#define AK_VA_CAMERA		AK39_ADDR(0x00010000)
#define AK_PA_CAMERA		(0x20000000)
#define AK_SZ_CAMERA		SZ_64K		/* 64KB */

#define AK_VA_VENCODE		AK39_ADDR(0x00020000)
#define AK_PA_VENCODE		(0x20020000)
#define AK_SZ_VENCODE		SZ_64K		/* 64KB */

/* some sub system control register */
#define AK_VA_SUBCTRL		AK39_ADDR(0x00030000)
#define AK_PA_SUBCTRL		(0x20100000)
#define AK_SZ_SUBCTRL		SZ_2M		/* 2MB */

#define AK_VA_MAC			AK39_ADDR(0x00230000)
#define AK_PA_MAC			(0x20300000)
#define AK_SZ_MAC			SZ_8K		/* 8KB */

#define AK_VA_REGRAM		AK39_ADDR(0x00232000)
#define AK_PA_REGRAM		(0x21000000)
#define AK_SZ_REGRAM		SZ_8K		/* 8KB */

#define AK_VA_L2MEM			AK39_ADDR(0x00234000)
#define AK_PA_L2MEM			(0x4800C000)
#define AK_SZ_L2MEM			SZ_8K 		/* 8KB */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#define AK_VA_MCI			(AK_VA_SUBCTRL + 0x0000)
#define AK_PA_MCI			(AK_PA_SUBCTRL + 0x0000)

#define AK_VA_SDIO			(AK_VA_SUBCTRL + 0x8000)
#define AK_PA_SDIO			(AK_PA_SUBCTRL + 0x8000)

#define AK_VA_DAC			(AK_VA_SUBCTRL + 0x10000)
#define AK_PA_DAC			(AK_PA_SUBCTRL + 0x10000)

#define AK_VA_ADC			(AK_VA_SUBCTRL + 0x18000)
#define AK_PA_ADC			(AK_PA_SUBCTRL + 0x18000)

#define AK_VA_SPI1			(AK_VA_SUBCTRL + 0x20000)
#define AK_PA_SPI1			(AK_PA_SUBCTRL + 0x20000)

#define AK_VA_SPI2			(AK_VA_SUBCTRL + 0x28000)
#define AK_PA_SPI2			(AK_PA_SUBCTRL + 0x28000)

#define AK_VA_UART			(AK_VA_SUBCTRL + 0x30000)
#define AK_PA_UART			(AK_PA_SUBCTRL + 0x30000)

#define AK_VA_L2CTRL		(AK_VA_SUBCTRL + 0x40000)
#define AK_PA_L2CTRL		(AK_PA_SUBCTRL + 0x40000)

#define AK_VA_I2C			(AK_VA_SUBCTRL + 0x50000)
#define AK_PA_I2C			(AK_VA_SUBCTRL + 0x50000)

#define AK_VA_IRDA			(AK_VA_SUBCTRL + 0x60000)
#define AK_PA_IRDA			(AK_PA_SUBCTRL + 0x60000)

#define AK_VA_GPIO			(AK_VA_SUBCTRL + 0x70000)
#define AK_PA_GPIO			(AK_PA_SUBCTRL + 0x70000)

/* encryption register */
#define AK_VA_ENCRY			(AK_VA_SUBCTRL + 0x80000)
#define AK_PA_ENCRY			(AK_PA_SUBCTRL + 0x80000)

/* usb register */
#define AK_VA_USB			(AK_VA_SUBCTRL + 0x100000)
#define AK_PA_USB			(AK_PA_SUBCTRL + 0x100000)


#define	write_ramb(v, p)		(*(volatile unsigned char *)(p) = (v))
#define write_ramw(v, p)		(*(volatile unsigned short *)(p) = (v))
#define write_raml(v, p)		(*(volatile unsigned long *)(p) = (v))

#define read_ramb(p)			(*(volatile unsigned char *)(p))
#define read_ramw(p)			(*(volatile unsigned short *)(p))
#define read_raml(p)			(*(volatile unsigned long *)(p))

#define write_buf(v, p)			(*(volatile unsigned long *)(p) = (v))
#define read_buf(p)				(*(volatile unsigned long *)(p))

#define REG_VA_VAL(base_addr, offset)	(*(volatile unsigned long *)((base_addr) + (offset)))
#define REG_VA_ADDR(base_addr, offset)	((base_addr) + (offset))

#define REG_PA_VAL(base_addr, offset)	(*(volatile unsigned long *)((base_addr) + (offset)))
#define REG_PA_ADDR(base_addr, offset)	((base_addr) + (offset))


#endif  /* __ASM_ARCH_MAP_H */

