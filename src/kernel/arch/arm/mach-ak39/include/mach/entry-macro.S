/*
 * include/asm-arm/arch-ak39/entry-macro.S
 *
 * Low-level IRQ helper macros for AK39-based platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
*/

#include <mach/hardware.h>
#include <asm/irq.h>

#define	AK39_IRQ_INTMASK		(AK_VA_SYSCTRL + 0x24)
#define	AK39_FIQ_INTMASK		(AK_VA_SYSCTRL + 0x28)
#define	AK39_INT_STATUS			(AK_VA_SYSCTRL + 0x4C)

	.macro  get_irqnr_preamble, base, tmp
	.endm

	.macro  arch_ret_to_user, tmp1, tmp2
	.endm

	.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
	
		ldr	\base, =AK39_INT_STATUS
		ldr	\irqstat, [\base]		@ get interrupts status
		teq	\irqstat, #0x0
		beq	1002f

		ldr	\base, =AK39_IRQ_INTMASK	@ get interrupts mask
		ldr	\base, [\base]
		and	\irqstat, \irqstat, \base

		tst	\irqstat, #(1<<IRQ_MEM)
		bicne	\irqstat, \irqstat, #(1<<IRQ_MEM)
		movne	\irqnr, #IRQ_MEM
		bne	1001f

		tst	\irqstat, #(1<<IRQ_CAMERA)
		bicne	\irqstat, \irqstat, #(1<<IRQ_CAMERA)
		movne	\irqnr, #IRQ_CAMERA
		bne	1001f

		tst	\irqstat, #(1<<IRQ_VIDEO_ENCODER)
		bicne	\irqstat, \irqstat, #(1<<IRQ_VIDEO_ENCODER)
		movne	\irqnr, #IRQ_VIDEO_ENCODER
		bne	1001f

		tst	\irqstat, #(1<<IRQ_SYSCTRL)
		bicne	\irqstat, \irqstat, #(1<<IRQ_SYSCTRL)
		movne	\irqnr, #IRQ_SYSCTRL
		bne	1001f

		tst	\irqstat, #(1<<IRQ_MCI)
		bicne	\irqstat, \irqstat, #(1<<IRQ_MCI)
		movne	\irqnr, #IRQ_MCI
		bne	1001f

		tst	\irqstat, #(1<<IRQ_SDIO)
		bicne	\irqstat, \irqstat, #(1<<IRQ_SDIO)
		movne	\irqnr, #IRQ_SDIO
		bne	1001f

		tst	\irqstat, #(1<<IRQ_ADC2)
		bicne	\irqstat, \irqstat, #(1<<IRQ_ADC2)
		movne	\irqnr, #IRQ_ADC2
		bne	1001f

		tst	\irqstat, #(1<<IRQ_DAC)
		bicne	\irqstat, \irqstat, #(1<<IRQ_DAC)
		movne	\irqnr, #IRQ_DAC
		bne	1001f

		tst	\irqstat, #(1<<IRQ_SPI1)
		bicne	\irqstat, \irqstat, #(1<<IRQ_SPI1)
		movne	\irqnr, #IRQ_SPI1
		bne	1001f

		tst	\irqstat, #(1<<IRQ_SPI2)
		bicne	\irqstat, \irqstat, #(1<<IRQ_SPI2)
		movne	\irqnr, #IRQ_SPI2
		bne	1001f

		tst	\irqstat, #(1<<IRQ_UART0)
		bicne	\irqstat, \irqstat, #(1<<IRQ_UART0)
		movne	\irqnr, #IRQ_UART0
		bne	1001f

		tst	\irqstat, #(1<<IRQ_UART1)
		bicne	\irqstat, \irqstat, #(1<<IRQ_UART1)
		movne	\irqnr, #IRQ_UART1
		bne	1001f

		tst	\irqstat, #(1<<IRQ_L2MEM)
		bicne	\irqstat, \irqstat, #(1<<IRQ_L2MEM)
		movne	\irqnr, #IRQ_L2MEM
		bne	1001f

		tst	\irqstat, #(1<<IRQ_I2C)
		bicne	\irqstat, \irqstat, #(1<<IRQ_I2C)
		movne	\irqnr, #IRQ_I2C
		bne	1001f

		tst	\irqstat, #(1<<IRQ_IRDA)
		bicne	\irqstat, \irqstat, #(1<<IRQ_IRDA)
		movne	\irqnr, #IRQ_IRDA
		bne	1001f

		tst	\irqstat, #(1<<IRQ_GPIO)
		bicne	\irqstat, \irqstat, #(1<<IRQ_GPIO)
		movne	\irqnr, #IRQ_GPIO
		bne	1001f

		tst	\irqstat, #(1<<IRQ_MAC)
		bicne	\irqstat, \irqstat, #(1<<IRQ_MAC)
		movne	\irqnr, #IRQ_MAC
		bne	1001f

		tst	\irqstat, #(1<<IRQ_ENCRYTION)
		bicne	\irqstat, \irqstat, #(1<<IRQ_ENCRYTION)
		movne	\irqnr, #IRQ_ENCRYTION
		bne	1001f

		tst	\irqstat, #(1<<IRQ_USBOTG_MCU)
		bicne	\irqstat, \irqstat, #(1<<IRQ_USBOTG_MCU)
		movne	\irqnr, #IRQ_USBOTG_MCU
		bne	1001f

		tst	\irqstat, #(1<<IRQ_USBOTG_DMA)
		bicne	\irqstat, \irqstat, #(1<<IRQ_USBOTG_DMA)
		movne	\irqnr, #IRQ_USBOTG_DMA
		bne	1001f

	1001:
		@ got irqnr
	1002:
		@ exit here
	.endm

	/* currently don't need an disable_fiq macro */
	.macro	disable_fiq
	.endm
	
