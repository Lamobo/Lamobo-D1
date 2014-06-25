/**
 * @filename    include/linux/akuio_driver.h
 * @brief       This file provide the ioctl definitions of akuio driver for hw codec.
 *              Copyright (C) 2011 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
 * @author      Jacky Lau
 * @date        2011-07-05
 * @version     0.1
 * @ref         Please refer to uio_ak98_vcodec.c
 */

#ifndef _AKUIO_DRIVER_H
#define _AKUIO_DRIVER_H

/* struct used by AKUIO_SYSREG_WRITE */
struct akuio_sysreg_write_t
{
	unsigned int paddr;
	unsigned int val;
	unsigned int mask;
};

#define DBG(fmt...)		//printk(fmt)

/* write system register */
#define AKUIO_SYSREG_WRITE       _IOW('U', 100, struct akuio_sysreg_write_t)

/* wait for a interrupt occur */
#define AKUIO_WAIT_IRQ           _IOR('U', 101, int)

/* invalidate the l2 cache in ak98 */
#define AKUIO_INVALIDATE_L2CACHE      _IOR('U', 102, int)

/* incalidate the l1 cache in ak98 */
#define AKUIO_INVALIDATE_L1CACHE      _IOR('U', 103, int)


#endif
