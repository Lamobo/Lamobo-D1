/* linux/arch/arm/mach-ak39/include/mach/irqs.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_IRQS_H_
#define __ASM_ARCH_IRQS_H_

#define	AK39_IRQ(x)				(x)

/*
 * Main CPU Interrupts
 */
#define	IRQ_MEM					AK39_IRQ(0)
#define	IRQ_CAMERA				AK39_IRQ(1)
#define	IRQ_VIDEO_ENCODER		AK39_IRQ(2)
#define	IRQ_SYSCTRL				AK39_IRQ(3)
#define	IRQ_MCI					AK39_IRQ(4)
#define	IRQ_SDIO				AK39_IRQ(5)
#define	IRQ_ADC2				AK39_IRQ(6)
#define	IRQ_DAC					AK39_IRQ(7)
#define	IRQ_SPI1				AK39_IRQ(8)
#define	IRQ_SPI2				AK39_IRQ(9)
#define	IRQ_UART0				AK39_IRQ(10)
#define	IRQ_UART1				AK39_IRQ(11)
#define	IRQ_L2MEM				AK39_IRQ(12)
#define	IRQ_I2C					AK39_IRQ(13)
#define	IRQ_IRDA				AK39_IRQ(14)
#define	IRQ_GPIO				AK39_IRQ(15)
#define	IRQ_MAC					AK39_IRQ(16)
#define	IRQ_ENCRYTION			AK39_IRQ(17)
#define	IRQ_USBOTG_MCU			AK39_IRQ(18)
#define	IRQ_USBOTG_DMA			AK39_IRQ(19)

/*
 * System Control Module Sub-IRQs
 */
#define IRQ_SYSCTRL_START		(IRQ_USBOTG_DMA + 1)
#define	AK39_SYSCTRL_IRQ(x)		(IRQ_SYSCTRL_START + (x))

#define IRQ_SARADC				AK39_SYSCTRL_IRQ(0)
#define	IRQ_TIMER5				AK39_SYSCTRL_IRQ(1)
#define	IRQ_TIMER4				AK39_SYSCTRL_IRQ(2)
#define	IRQ_TIMER3				AK39_SYSCTRL_IRQ(3)
#define	IRQ_TIMER2				AK39_SYSCTRL_IRQ(4)
#define	IRQ_TIMER1				AK39_SYSCTRL_IRQ(5)
#define IRQ_WGPIO				AK39_SYSCTRL_IRQ(6)
#define	IRQ_RTC_RDY				AK39_SYSCTRL_IRQ(7)
#define	IRQ_RTC_ALARM			AK39_SYSCTRL_IRQ(8)
#define IRQ_RTC_TIMER			AK39_SYSCTRL_IRQ(9)
#define IRQ_RTC_WATCHDOG		AK39_SYSCTRL_IRQ(10)

/* 
 * GPIO IRQs
 */
#define IRQ_GPIO_START			(IRQ_RTC_WATCHDOG + 1)
#define AK39_GPIO_IRQ(x)		(IRQ_GPIO_START + (x))

#define IRQ_GPIO_0				AK39_GPIO_IRQ(0)
#define IRQ_GPIO_1				AK39_GPIO_IRQ(1)
#define IRQ_GPIO_2				AK39_GPIO_IRQ(2)
#define IRQ_GPIO_3				AK39_GPIO_IRQ(3)
#define IRQ_GPIO_4				AK39_GPIO_IRQ(4)
#define IRQ_GPIO_5				AK39_GPIO_IRQ(5)
#define IRQ_GPIO_6				AK39_GPIO_IRQ(6)
#define IRQ_GPIO_7				AK39_GPIO_IRQ(7)
#define IRQ_GPIO_8				AK39_GPIO_IRQ(8)
#define IRQ_GPIO_9				AK39_GPIO_IRQ(9)
#define IRQ_GPIO_10				AK39_GPIO_IRQ(10)
#define IRQ_GPIO_11				AK39_GPIO_IRQ(11)
#define IRQ_GPIO_12				AK39_GPIO_IRQ(12)
#define IRQ_GPIO_13				AK39_GPIO_IRQ(13)
#define IRQ_GPIO_14				AK39_GPIO_IRQ(14)
#define IRQ_GPIO_15				AK39_GPIO_IRQ(15)
#define IRQ_GPIO_16				AK39_GPIO_IRQ(16)
#define IRQ_GPIO_17				AK39_GPIO_IRQ(17)
#define IRQ_GPIO_18				AK39_GPIO_IRQ(18)
#define IRQ_GPIO_19				AK39_GPIO_IRQ(19)
#define IRQ_GPIO_20				AK39_GPIO_IRQ(20)
#define IRQ_GPIO_21				AK39_GPIO_IRQ(21)
#define IRQ_GPIO_22				AK39_GPIO_IRQ(22)
#define IRQ_GPIO_23				AK39_GPIO_IRQ(23)
#define IRQ_GPIO_24				AK39_GPIO_IRQ(24)
#define IRQ_GPIO_25				AK39_GPIO_IRQ(25)
#define IRQ_GPIO_26				AK39_GPIO_IRQ(26)
#define IRQ_GPIO_27				AK39_GPIO_IRQ(27)
#define IRQ_GPIO_28				AK39_GPIO_IRQ(28)
#define IRQ_GPIO_29				AK39_GPIO_IRQ(29)
#define IRQ_GPIO_30				AK39_GPIO_IRQ(30)
#define IRQ_GPIO_31				AK39_GPIO_IRQ(31)

#define IRQ_GPIO_32				AK39_GPIO_IRQ(32)
#define IRQ_GPIO_33				AK39_GPIO_IRQ(33)
#define IRQ_GPIO_34				AK39_GPIO_IRQ(34)
#define IRQ_GPIO_35				AK39_GPIO_IRQ(35)
#define IRQ_GPIO_36				AK39_GPIO_IRQ(36)
#define IRQ_GPIO_37				AK39_GPIO_IRQ(37)
#define IRQ_GPIO_38				AK39_GPIO_IRQ(38)
#define IRQ_GPIO_39				AK39_GPIO_IRQ(39)
#define IRQ_GPIO_40				AK39_GPIO_IRQ(40)
#define IRQ_GPIO_41				AK39_GPIO_IRQ(41)
#define IRQ_GPIO_42				AK39_GPIO_IRQ(42)
#define IRQ_GPIO_43				AK39_GPIO_IRQ(43)
#define IRQ_GPIO_44				AK39_GPIO_IRQ(44)
#define IRQ_GPIO_45				AK39_GPIO_IRQ(45)
#define IRQ_GPIO_46				AK39_GPIO_IRQ(46)
#define IRQ_GPIO_47				AK39_GPIO_IRQ(47)
#define IRQ_GPIO_48				AK39_GPIO_IRQ(48)
#define IRQ_GPIO_49				AK39_GPIO_IRQ(49)
#define IRQ_GPIO_50				AK39_GPIO_IRQ(50)
#define IRQ_GPIO_51				AK39_GPIO_IRQ(51)
#define IRQ_GPIO_52				AK39_GPIO_IRQ(52)
#define IRQ_GPIO_53				AK39_GPIO_IRQ(53)
#define IRQ_GPIO_54				AK39_GPIO_IRQ(54)
#define IRQ_GPIO_55				AK39_GPIO_IRQ(55)
#define IRQ_GPIO_56				AK39_GPIO_IRQ(56)
#define IRQ_GPIO_57				AK39_GPIO_IRQ(57)
#define IRQ_GPIO_58				AK39_GPIO_IRQ(58)
#define IRQ_GPIO_59				AK39_GPIO_IRQ(59)
#define IRQ_GPIO_60				AK39_GPIO_IRQ(60)
#define IRQ_GPIO_61				AK39_GPIO_IRQ(61)
#define IRQ_GPIO_62				AK39_GPIO_IRQ(62)
#define IRQ_GPIO_63				AK39_GPIO_IRQ(63)


/* total irq number */
#define NR_IRQS     	(IRQ_GPIO_63 + 1)

#endif  /* __ASM_ARCH_IRQS_H_ */

