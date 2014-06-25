/*
 * linux/arch/arm/mach-ak39/include/l2cache.h
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_L2CACHE_H
#define __ASM_ARCH_L2CACHE_H

void l2cache_init(void);
void l2cache_clean_finish(void);
void l2cache_invalidate(void);

#endif	/* __ASM_ARCH_L2CACHE_H */
