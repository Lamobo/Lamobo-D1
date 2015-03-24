/* 
 *	mach/reset.h
 */
#ifndef _AK39_RESET_H_
#define _AK39_RESET_H_	__FILE__

#include <linux/types.h>

extern void (*ak39_arch_reset) (void);

#define MODULE_RESET_CON1       (AK_VA_SYSCTRL + 0x20)
#define MODULE_WDT_CFG1	(AK_VA_SYSCTRL + 0xe4)
#define MODULE_WDT_CFG2	(AK_VA_SYSCTRL + 0Xe8)

#define AK39_SRESET_MMCSD		(1)
#define AK39_SRESET_SDIO		(2)
#define AK39_SRESET_ADC			(3)
#define AK39_SRESET_DAC			(4)
#define AK39_SRESET_SPI1		(5)
#define AK39_SRESET_SPI2		(6)
#define AK39_SRESET_UART1		(7)
#define AK39_SRESET_UART2		(8)
#define AK39_SRESET_L2MEM		(9)
#define AK39_SRESET_I2C			(10)
#define AK39_SRESET_IRDA		(11)
#define AK39_SRESET_GPIO		(12)
#define AK39_SRESET_MAC			(13)
#define AK39_SRESET_ENCRY		(14)
#define AK39_SRESET_USBHS		(15)
#define AK39_SRESET_CAMERA		(19)
#define AK39_SRESET_VIDEO		(20)
#define AK39_SRESET_DRAM		(24)

int ak39_soft_reset(u32 module);

/***** extern call for comm drivers compatible *****/
#define AK_SRESET_MMCSD		AK39_SRESET_MMCSD
#define AK_SRESET_SDIO		AK39_SRESET_SDIO
#define AK_SRESET_ADC		AK39_SRESET_ADC
#define AK_SRESET_DAC		AK39_SRESET_DAC
#define AK_SRESET_SPI1		AK39_SRESET_SPI1
#define AK_SRESET_SPI2		AK39_SRESET_SPI2
#define AK_SRESET_UART1		AK39_SRESET_UART1
#define AK_SRESET_UART2		AK39_SRESET_UART2
#define AK_SRESET_L2MEM		AK39_SRESET_L2MEM
#define AK_SRESET_I2C		AK39_SRESET_I2C
#define AK_SRESET_IRDA		AK39_SRESET_IRDA
#define AK_SRESET_GPIO		AK39_SRESET_GPIO
#define AK_SRESET_MAC		AK39_SRESET_MAC
#define AK_SRESET_ENCRY		AK39_SRESET_ENCRY
#define AK_SRESET_USBHS		AK39_SRESET_USBHS
#define AK_SRESET_CAMERA	AK39_SRESET_CAMERA
#define AK_SRESET_VIDEO		AK39_SRESET_VIDEO
#define AK_SRESET_DRAM		AK39_SRESET_DRAM


int ak_soft_reset(u32 module);
/*** end extern call for comm drivers compatible ***/

#endif
