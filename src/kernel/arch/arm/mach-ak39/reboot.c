/*
 * reboot.c - implement a interface jump to ROM code
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
//#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include <mach/l2cache.h>
#include <asm/io.h>

#define DIS_INTERRUPTS	(0x00001FFF)
#define RESET_DEVICE1	(0xE9A70000)
#define RESET_DEVICE2	(0xBF7F0000)
#define CLOSE_CLOCK1	(0x0000E9A7)
#define CLOSE_CLOCK2	(0x0000BF7F)
#define RESET_MULTI1	(0x003003C8)
#define RESET_MULTI2	(0x24000000)
#define RESET_SHARE1	(0x00000203)
#define RESET_SHARE2	(0x00000001)
/**
 * @brief:	ak39_jump_to_rom
 * @author:	zhongjunchao
 * @date:	2011-10-13
 *
 * @note:	Invalidate  I & D cache and TLBs, and then disable I & D cache and MMU
 * 		At last, we jump to ROM
 */
void ak39_jump_to_rom(unsigned long addr)
{
        __asm__ __volatile__(
        "1: mrc  p15, 0, pc, c7, c14, 3\n\t"   
        "bne 1b\n\t"
        "mov %0, #0x0\n\t"
        "mcr p15, 0, %0, c8, c7, 0\n\t"
        "mcr p15, 0, %0, c7, c5, 0\n\t"
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "bic %0, %0, #0x3000\n\t"
        "bic %0, %0, #0x0005\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        "mov pc, %1\n\t"
        : : "r"(0),"r"(addr));
}
EXPORT_SYMBOL(ak39_jump_to_rom);

/**
 * @brief:      ak39_reboot_sys_by_soft
 * @author:     zhongjunchao
 * @date:       2011-10-11
 *
 * @note:       I jump to 0x0 to reboot our system.
 * 		TODO: Now on, the register config only for ak39xx.
 */
void ak39_reboot_sys_by_soft(void)
{
	/* disable some interrupt */
    REG32(AK_VA_SYSCTRL + 0x4c) &= ~DIS_INTERRUPTS;
	printk("After disable some interrupt\n");

	/* mask all normal interrupt */
    REG32(AK_VA_SYSCTRL + 0x34) = 0x0;
	printk("After mask all normal interrupt\n");

	/* mask all GPIO interrupt */
    REG32(AK_GPIO_INT_MASK1) = 0x0;
    REG32(AK_GPIO_INT_MASK2) = 0x0;
	printk("After disable all GPIO interrupt\n");

	/* reset and close clock except UART1,RAM and L2 FIFO */
	REG32(AK_VA_SYSCTRL + 0x0C) |= RESET_DEVICE1;
	udelay(500);
	REG32(AK_VA_SYSCTRL + 0x0C) &= ~RESET_DEVICE1;

	REG32(AK_VA_SYSCTRL + 0x10) |= RESET_DEVICE2;
	udelay(500);
	REG32(AK_VA_SYSCTRL + 0x10) &= ~RESET_DEVICE2;

	REG32(AK_VA_SYSCTRL + 0x0C) |= CLOSE_CLOCK1;
	REG32(AK_VA_SYSCTRL + 0x10) |= CLOSE_CLOCK2;
	printk("After reset all device and close clock\n");

	/* reset multiple-function control */
	REG32(AK_VA_SYSCTRL + 0x58) = RESET_MULTI1;
	REG32(AK_VA_SYSCTRL + 0x14) = RESET_MULTI2;
	printk("After reset multiple-function\n");
	 
	/* reset share pin control */
	REG32(AK_VA_SYSCTRL + 0x78) = RESET_SHARE1;
	REG32(AK_VA_SYSCTRL + 0x74) = RESET_SHARE2;
	printk("After reset all share pin\n");

	/* reset GPIO interrupt polarity */
	REG32(AK_GPIO_INTP1) = 0x0;
	REG32(AK_GPIO_INTP2) = 0x0;
	printk("After reset all GPIO int pol\n");

	/* reset GPIO direction */
	REG32(AK_GPIO_DIR1) = 0x0;
	REG32(AK_GPIO_DIR2) = 0x0;
	printk("After reset all GPIO dir\n");

	/* reset GPIO pull up/down */
	REG32(AK_VA_SYSCTRL + 0x9C) = 0x0;
	REG32(AK_VA_SYSCTRL + 0xA0) = 0x0;
	REG32(AK_VA_SYSCTRL + 0xA4) = 0x0;
	REG32(AK_VA_SYSCTRL + 0xA8) = 0x0;
	printk("After reset all GPIO PDU\n");

	/* disable l2 cache */
    l2cache_clean_finish();
    l2cache_invalidate();
	printk("After disable l2 cache\n");

	/* jump to 0x0 anddress */
    ak39_jump_to_rom(0x0);
}
EXPORT_SYMBOL(ak39_reboot_sys_by_soft);
