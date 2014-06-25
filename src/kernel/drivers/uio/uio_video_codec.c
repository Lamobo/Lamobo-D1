/**
 * drivers/uio/uio_video_codec.c
 *
 * Userspace I/O driver for anyka soc video hardware codec.
 * Based on uio_pdrv.c by Uwe Kleine-Koenig,
 *
 * Jacky Lau
 * 2011-07-05
 *
 * Copyright (C) 2011 by Anyka Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/uio_driver.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>

#include <linux/akuio_driver.h>
#include <mach/reg.h>
#include <mach/l2cache.h>

#define DRIVER_NAME "uio_vcodec"

#define init_MUTEX(sem)		sema_init(sem, 1)
#define init_MUTEX_LOCKED(sem)	sema_init(sem, 0)

/* IRQs of hw codec */
const unsigned VIDEO_IRQ_MASK = (1 << IRQ_VIDEO_ENCODER);
const int MASK_BITS_NUM = sizeof(VIDEO_IRQ_MASK) * 8;

/* platform data of this driver */
struct uio_platdata {
	struct uio_info *uioinfo;
	struct semaphore vcodec_sem;
    unsigned int open_count;
};

/**
 * @brief     Handle hw codec irq
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  irq :  irq id
 * @param     [in]  dev_id :  platform data
 * @return    irqreturn_t :  return handle result
 * @retval    IRQ_HANDLE :  irq handled successful.
 */
static irqreturn_t uio_vcodec_irq_handler(int irq, void *dev_id)
{
	struct uio_platdata *pdata = dev_id;
	int i;

	for (i = 0; i < MASK_BITS_NUM; i++)
	{
		if ((1 << i) & VIDEO_IRQ_MASK)
			disable_irq_nosync(i);
	}

	up (&(pdata->vcodec_sem));

	return IRQ_HANDLED;
}

/**
 * @brief     Handle hw codec ioctl
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  uioinfo :  information of uio driver
 * @param     [in]  cmd :  ioctl request code
 * @param     [in]  arg :  argument
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int uio_vcodec_ioctl(struct uio_info *uioinfo, unsigned int cmd, unsigned long arg)
{
	struct uio_platdata *pdata = uioinfo->priv;
	int err;

	switch (cmd) {
	case AKUIO_SYSREG_WRITE:
	{
		struct akuio_sysreg_write_t reg_write;

		if (copy_from_user(&reg_write, (void __user *)arg, sizeof(struct akuio_sysreg_write_t)))
			return -EFAULT;

		sys_ctrl_reg_set(reg_write.paddr, reg_write.mask, reg_write.val);

		err = 0;
	}
	break;

	case AKUIO_WAIT_IRQ:
	{
		int i;

		for (i = 0; i < MASK_BITS_NUM; i++)
		{
			if ((1 << i) & VIDEO_IRQ_MASK)
				enable_irq(i);
		}
		
		down (&pdata->vcodec_sem);
		err = 0;
	}
	break;

	case AKUIO_INVALIDATE_L2CACHE:
		l2cache_invalidate ();
		flush_cache_all();
		err = 0;
		break;

	case AKUIO_INVALIDATE_L1CACHE:
//		flush_cache_all();
		err = 0;
		break;

	default:
		err = -EINVAL;
		break;
	};

	return err;
}

/**
 * @brief     When application open the akuio device, request all irq of the hw codec and disable the irq immediately
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  uioinfo :  information of uio driver
 * @param     [in]  inode :  inode of the device
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int uio_vcodec_open(struct uio_info *uioinfo, struct inode *inode)
{
	struct uio_platdata *pdata = uioinfo->priv;
	int i;
	int ret = 0;

    //Add code here to make sure uio0 can be multi-opened.
    if( pdata->open_count++ > 0 ) {
        DBG("uio0 has opened! open_count=%d\n", pdata->open_count );
        return 0;
    }    
    
	init_MUTEX_LOCKED(&(pdata->vcodec_sem));

	for (i = 0; i < MASK_BITS_NUM; i++)
	{
		if ((1 << i) & VIDEO_IRQ_MASK) {
			ret = request_irq(i, uio_vcodec_irq_handler, IRQF_DISABLED, "VIDEO HW CODEC", pdata);
			disable_irq_nosync(i);
		}
	}
	
	return 0;
}

/**
 * @brief     When application close the akuio device, free all irq of the hw codec
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  uioinfo :  information of uio driver
 * @param     [in]  inode :  inode of the device
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int uio_vcodec_release(struct uio_info *uioinfo, struct inode *inode)
{
	struct uio_platdata *pdata = uioinfo->priv;
	int i;

    //Add code here to make sure uio0 can be multi-opened.
	if(--pdata->open_count != 0) {
	    DBG("uio0 does's closed to 0! open_count=%d\n", pdata->open_count );
	    return 0;
	}	

	for (i = 0; i < MASK_BITS_NUM; i++)
	{
		if ((1 << i) & VIDEO_IRQ_MASK)
			free_irq(i, pdata);
	}

	return 0;
}

/**
 * @brief     Init the device which was probed
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  pdev :  the device definition
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int uio_vcodec_probe(struct platform_device *pdev)
{
	struct uio_info *uioinfo = pdev->dev.platform_data;
	struct uio_platdata *pdata;
	struct uio_mem *uiomem;
	int ret = -ENODEV;
	int i;

	if (!uioinfo || !uioinfo->name || !uioinfo->version) {
		dev_dbg(&pdev->dev, "%s: err_uioinfo\n", __func__);
		goto err_uioinfo;
	}

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		ret = -ENOMEM;
		dev_dbg(&pdev->dev, "%s: err_alloc_pdata\n", __func__);
		goto err_alloc_pdata;
	}

	pdata->uioinfo = uioinfo;

	uiomem = &uioinfo->mem[0];

	for (i = 0; i < pdev->num_resources; ++i) {
		struct resource *r = &pdev->resource[i];
		if (r->flags != IORESOURCE_MEM)
			continue;

		if (uiomem >= &uioinfo->mem[MAX_UIO_MAPS]) {
			dev_warn(&pdev->dev, "device has more than "
					__stringify(MAX_UIO_MAPS)
					" I/O memory resources.\n");
			break;
		}

		uiomem->memtype = UIO_MEM_PHYS;
		uiomem->addr = r->start;
		uiomem->size = r->end - r->start + 1;
		++uiomem;
	}

	while (uiomem < &uioinfo->mem[MAX_UIO_MAPS]) {
		uiomem->size = 0;
		++uiomem;
	}

    /* open count */
    pdata->open_count = 0;
        
	/* irq */
	pdata->uioinfo->irq = UIO_IRQ_CUSTOM;

	/* file handle */
	pdata->uioinfo->open = uio_vcodec_open;
	pdata->uioinfo->release = uio_vcodec_release;
	pdata->uioinfo->ioctl = uio_vcodec_ioctl;

	pdata->uioinfo->priv = pdata;

	ret = uio_register_device(&pdev->dev, pdata->uioinfo);

	if (ret) {
		kfree(pdata);
err_alloc_pdata:
err_uioinfo:
		return ret;
	}

	platform_set_drvdata(pdev, pdata);

	return 0;
}

/**
 * @brief     De-init the device which will be removed
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param     [in]  pdev :  the device definition
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int uio_vcodec_remove(struct platform_device *pdev)
{
	struct uio_platdata *pdata = platform_get_drvdata(pdev);

	uio_unregister_device(pdata->uioinfo);

	kfree(pdata);

	return 0;
}

/* driver definition */
static struct platform_driver uio_vcodec = {
	.probe = uio_vcodec_probe,
	.remove = uio_vcodec_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

/**
 * @brief     kernel module init function, register the driver to kernel
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param
 * @return    int :  return 0 when handle successful, otherwise return negative
 * @retval     0 :  handled successful.
 * @retval    <0 :  handle failed.
 */
static int __init uio_vcodec_init(void)
{
	return platform_driver_register(&uio_vcodec);
}

/**
 * @brief     kernel module finally function, unregister the driver from kernel
 * @author    Jacky Lau
 * @date      2011-07-05
 * @param
 * @return    void
 * @retval
 */
static void __exit uio_vcodec_exit(void)
{
	platform_driver_unregister(&uio_vcodec);
}
module_init(uio_vcodec_init);
module_exit(uio_vcodec_exit);

MODULE_AUTHOR("Jacky Lau");
MODULE_DESCRIPTION("Userspace driver for anyka video hw codec");
MODULE_LICENSE("GPL v2");

