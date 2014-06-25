/*
 * reg.c - Register Access Routines
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/io.h>
#include <mach/reg.h>
#include <mach/map.h>

static DEFINE_SPINLOCK(sys_ctrl_reg_lock);

/*
 * @brief Anyka system control register setting routine
 * @author Li Xiaoping
 * @date 2011-07-04
 * @param reg_phy_addr [in] Register Physical Address
 * @param reg_mask [in] Bit mask of the bits which need to be set
 * @param reg_val [in] The value of the bits which need to be set
 * @return void
 * @note This routine is created in order to solve the problem of access the sam system control
 *       register from different kernel path (ISR, system call, etc...).
 * @note This routine depends on that corresponding registers are mapped, currently this is
 *       done in akxx_map_io(), so be careful about akxx_map_io() changes.
 *       the akxx_ is instead of ak37_.
 * @sample sys_ctrl_reg_set(0x0800000C, (1 << 29), (1 << 29)) will set bit 29 of 
 *       Clock Control and Soft Reset Control Register to 1
 */
void sys_ctrl_reg_set(unsigned long reg_phy_addr, unsigned long reg_mask, unsigned long reg_val)
{
	unsigned long flags;
	unsigned long val;
	unsigned long reg_virt_addr;

	BUG_ON((reg_phy_addr < AK_PA_SYSCTRL) || (reg_phy_addr > (AK_PA_SYSCTRL + AK_SZ_SYSCTRL)));

	spin_lock_irqsave(&sys_ctrl_reg_lock, flags);

	reg_virt_addr = (unsigned long)AK_VA_SYSCTRL + reg_phy_addr - AK_PA_SYSCTRL;
	val = __raw_readl(reg_virt_addr);
	val = (val & ~reg_mask) | (reg_val & reg_mask);
	__raw_writel(val, reg_virt_addr);

	spin_unlock_irqrestore(&sys_ctrl_reg_lock, flags);	
}
EXPORT_SYMBOL(sys_ctrl_reg_set);
