/*
 *  Copyright (C) anyka 2012
 *  Wangsheng Gao <gao_wangsheng@anyka.oa>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <mach/map.h>
#include <mach/reset.h>

#undef REG32
#define REG32(_reg)		(*(volatile unsigned long *)(_reg))


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		soft reset module
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	which module
*  @param[in]   	delay millisecond second
*  @return      	fail or not
*/
int ak39_soft_reset(u32 module)
{
	BUG_ON(module < AK39_SRESET_MMCSD || module > AK39_SRESET_DRAM);

	REG32(MODULE_RESET_CON1) |= (0x1 << module);
	mdelay(5);
	REG32(MODULE_RESET_CON1) &= ~(0x1 << module);
	return 0;
}
EXPORT_SYMBOL(ak39_soft_reset);

/***** extern call for comm drivers compatible *****/
int ak_soft_reset(u32 module)
{
	return ak39_soft_reset(module);
}
EXPORT_SYMBOL(ak_soft_reset);
