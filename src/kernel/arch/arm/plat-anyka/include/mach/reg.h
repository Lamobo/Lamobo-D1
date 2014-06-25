/*
 * reg.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _REG_H_
#define _REG_H_

void sys_ctrl_reg_set(unsigned long reg_phy_addr, unsigned long reg_mask, unsigned long reg_val);

#endif /* _REG_H_ */

