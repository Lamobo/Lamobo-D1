/*
 *  spi_anyka_slave.c - Anyka SPI slave controller driver
 *  based on spi_anyka.c
 *
 *  Copyright (C) Anyka 2012
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

/*
 * Note:
 *
 * Supports interrupt programmed transfers.
 * 
 */

 /*
  * Usage:
  * 	1. set one device as master:
  *		1. register spidev board info in platform file;
  *		2. select spidev and anyka spi controler in menuconfig.
  *	2. set one device as slave: select anyka spi slave controler in menuconfig
  *	3. run make command in Documentation/spi/ directory
  *	4. copy Documentation/spi/spi_master_test and Documentation/spi/spi_slave_test to Your_rootfs_dir
  *	5. firstly run spi_slave_test in slave and then run spi_master_test in master
  */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <mach/regs-comm.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include <plat-anyka/spi-anyka-slave.h>
#include <plat/spi.h>
#include <linux/spi/spidev.h>
#include <asm/uaccess.h>
#include <mach/clock.h>

/**
*  @brief       	slave data: devices, lock, wait_queue, io resource...
*  @author     gao wangsheng
*  @date        2012-10-16
*  @param[out]  void
*  @param[in]   void
*  @param[in]   void
*  @return      void
*/
struct spi_anyka_slave {
	/* Driver model hookup */
	struct platform_device *pdev;
	struct ak_spi_info *pdata;
	struct cdev cdev;
	struct fasync_struct *async_queue; /* asynchronous readers */

	struct clk		*clk;

	/* Lock */
	struct mutex	slave_lock;
	spinlock_t regs_lock;

	/* wait queue */
	wait_queue_head_t readq;
	wait_queue_head_t writeq;

	/* read and write buffer */
	char *rdbuf, *rdend;
	char *recvp, *readp;
	int rdsize;
	char *wrbuf, *wrend;
	char *sentp, *writep;
	int wrsize;
	
	/* Spi controler register addresses */
	void *ioarea;
	void *regs;
	int irq;

	/* flag */
	u8 mode;
	u8 bits_per_word;
	u32 max_speed_hz;
	int openers;
};

static struct class *slave_class;
static int slave_major;
static int slave_minor = 0;
#define SLAVE_MAX_MINOR	(8)
#define DFT_DIV				(255)
#define DFT_CON 			AK_SPICON_EN
#define BUF_LEN				(512)
#define DCNT				(0xffff)
#define SET_CLK				(1 << 0)
#define SET_MODE			(1 << 1)
#define SPI_MODE_MASK (SPI_CPOL | SPI_CPHA )
#define SLAVE_MASK	(AK_SPISTA_RXHFULL|AK_SPIINT_TIMEOUT |AK_SPIINT_RXFULL)

#define sdbug(fmt...)	//printk(fmt)

static int akspi_slave_fasync(int fd, struct file *filp, int mode);


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		How much space is free?
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @return      	free space in buffer
*/
static int akspi_slave_space_free(struct spi_anyka_slave *slave)
{
	if (slave->readp == slave->recvp){
		return slave->rdsize - 1;
	}
	return ((slave->readp + slave->rdsize - slave->recvp) % slave->rdsize) - 1;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		store data into buffer
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	slave->recvp
*  @param[in]   	*slave
*  @param[in]   	*buf
*  @param[in]   	count
*  @return      	How much data has been stored
*/
static int akspi_slave_reveive_data(struct spi_anyka_slave *slave,
			char *buf, size_t count)
{
//	sdbug("%s\n", __func__);
	
	/* ok, space is there, accept something */
	count = min(count, (size_t)akspi_slave_space_free(slave));
	if (slave->recvp >= slave->readp){
		/* to end-of-buf */	
		count = min(count, (size_t)(slave->rdend - slave->recvp)); 
	}
	else {
		/* the write pointer has wrapped, fill up to rp-1 */
		count = min(count, (size_t)(slave->readp - slave->recvp - 1));
	}

	memcpy(slave->recvp, buf, count);
	slave->recvp += count;
	if (slave->recvp == slave->rdend){
		slave->recvp = slave->rdbuf; /* wrapped */
	}

	return count;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		Enable or disable irq
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @param[in]   	is enable?
*  @return      	void
*/
static void akspi_slave_set_irq(struct spi_anyka_slave *slave, int enable)
{
	u32 spiint = SLAVE_MASK; 
	if (enable)
		iowrite32(spiint, slave->regs + AK_SPIINT);
	else
		iowrite32(0, slave->regs + AK_SPIINT);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		receive data int interupt
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @param[in]   	irq
*  @return      	irqreturn_t
*/
static irqreturn_t akspi_slave_int(int irq, void *dev_id)
{
	struct spi_anyka_slave *slave = (struct spi_anyka_slave *)dev_id;
	u32 status, val;
	u16 count;
	int i;
	
	spin_lock(&slave->regs_lock);
	status = ioread32(slave->regs + AK_SPISTA);
	sdbug("%s: status=0x%08x\n", __func__, status);

	if (((status & AK_SPISTA_TIMEOUT) == AK_SPISTA_TIMEOUT)
		&& ((status & AK_SPISTA_RXEMP) != AK_SPISTA_RXEMP))
	{
		char temp;
		u16 cnt =0;
	
		count = ioread32(slave->regs + AK_SPICNT);
		cnt = (DCNT - count)%4;
		val = ioread32(slave->regs + AK_SPIIN);
		sdbug("%s: AK_SPISTA_TIMEOUT, val=0x%08x, cnt=%u\n", __func__, val, cnt);
		
		for (i=0; i<cnt; i++)
		{
			temp = (val >> i*8) & 0xff;
			akspi_slave_reveive_data(slave, &temp, 1);
		}

		iowrite32(DCNT, slave->regs + AK_SPICNT);
	}
	else if ((status & AK_SPISTA_RXFULL) == AK_SPISTA_RXFULL)
	{
		val = ioread32(slave->regs + AK_SPIIN);
		sdbug("%s: AK_SPISTA_RXFULL, val=0x%08x\n", __func__, val);
		akspi_slave_reveive_data(slave, (char *)&val, 4);
		val = ioread32(slave->regs + AK_SPIIN);
		sdbug("%s: AK_SPISTA_RXFULL, val=0x%08x\n", __func__, val);
		akspi_slave_reveive_data(slave, (char *)&val, 4);
	}
	
	else if ((status & AK_SPISTA_RXHFULL) == AK_SPISTA_RXHFULL)
	{
		val = ioread32(slave->regs + AK_SPIIN);
		sdbug("%s: AK_SPISTA_RXHFULL, val=0x%08x\n", __func__, val);
		akspi_slave_reveive_data(slave, (char *)&val, 4);
	}
	
	wake_up_interruptible(&slave->readq);
	if (slave->async_queue){
		kill_fasync(&slave->async_queue, SIGIO, POLL_IN);
	}
	spin_unlock(&slave->regs_lock);
	return IRQ_HANDLED;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		prepare receive data from master
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @return      	void
*/
static  void akspi_slave_prepare_read(struct spi_anyka_slave *slave)
{
	u32 val;
 	unsigned long flags; 

	spin_lock_irqsave(&slave->regs_lock, flags);
	
	val = ioread32(slave->regs + AK_SPICON);
	val &= ~(AK_SPICON_ARRM);
	val |= AK_SPICON_TGDM;
	iowrite32(val, slave->regs + AK_SPICON);
	iowrite32(DCNT, slave->regs + AK_SPICNT);
	akspi_slave_set_irq(slave, 1);

	spin_unlock_irqrestore(&slave->regs_lock, flags);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		open slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*node
*  @param[in]   	*filp
*  @return      	fail or not
*/
static int akspi_slave_open(struct inode *node, struct file *filp)
{
	struct spi_anyka_slave *slave = container_of(node->i_cdev, struct spi_anyka_slave, cdev);

	if (mutex_lock_interruptible(&slave->slave_lock)){
		return -ERESTARTSYS;
	}

	nonseekable_open(node, filp);

	if (!slave->rdbuf){
		/* Alloc memory for receive data  */
		slave->rdbuf = kzalloc(BUF_LEN, GFP_KERNEL);
		if (!slave->rdbuf){
			mutex_unlock(&slave->slave_lock);
			printk("%s: %d\n", __func__, __LINE__);
			return -ENOMEM;
		}
		akspi_slave_prepare_read(slave);
	}

	slave->rdsize = BUF_LEN;
	slave->rdend = slave->rdbuf + slave->rdsize;
	slave->readp = slave->recvp = slave->rdbuf; /* rd and wr from the beginning */
	slave->openers++;
	filp->private_data = slave;
	mutex_unlock(&slave->slave_lock);
	sdbug("%s: openers=%d\n", __func__, slave->openers);
	return 0;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		relese slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*node
*  @param[in]   	*filp
*  @return      	fail or not
*/
static int akspi_slave_release(struct inode *node, struct file *filp)
{
	struct spi_anyka_slave *slave = container_of(node->i_cdev, struct spi_anyka_slave, cdev);

	akspi_slave_fasync(-1, filp, 0);
	mutex_lock(&slave->slave_lock);

	slave->openers--;
	if (!slave->openers){
		/* close spi controler */
		akspi_slave_set_irq(slave, 0);
		kfree(slave->rdbuf);
		slave->rdbuf = NULL;
	}

	filp->private_data = NULL;
	mutex_unlock(&slave->slave_lock);
	sdbug("%s: openers=%d\n", __func__, slave->openers);
	return 0;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		setup slave controler
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @param[in]   	*void
*  @return      	fail or not
*/
static int akspi_slave_setmode(struct spi_anyka_slave *slave)
{
	u16 spicon;

	sdbug("%s: set mode-------------.\n", __func__);
	spin_lock(&slave->regs_lock);
	spicon = ioread32(slave->regs + AK_SPICON);
	
	if (slave->mode & SPI_CPHA)
		spicon |= AK_SPICON_CPHA;
	else
		spicon &= ~AK_SPICON_CPHA;
	if (slave->mode & SPI_CPOL)
		spicon |= AK_SPICON_CPOL;
	else
		spicon &= ~AK_SPICON_CPOL;
		
	iowrite32(spicon, slave->regs + AK_SPICON);
	spin_unlock(&slave->regs_lock);
	return 0;		
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		setup slave controler
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @param[in]   	*void
*  @return      	fail or not
*/
static int akspi_slave_setclk(struct spi_anyka_slave *slave)
{
	unsigned int div;
	unsigned int hz = slave->max_speed_hz;
	unsigned long clk = ak_get_asic_clk();
	u16 spicon;
	
	sdbug("%s: set clk-------------.\n", __func__);
	spin_lock(&slave->regs_lock);
	spicon = ioread32(slave->regs + AK_SPICON);
	
	div = clk / (hz*2) - 1;
	if (div > 255){
		div = 255;
	}
	else if (div < 3){
		div = 3;
	}

	spicon &=~(0xff << 8);
	spicon |= div << 8;
	iowrite32(spicon, slave->regs + AK_SPICON);
	spin_unlock(&slave->regs_lock);
	return 0;
}


/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		ioctl slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*node
*  @param[in]   	*filp, cmd, arg
*  @return      	fail or not
*/
static long akspi_slave_ioctl(struct file *filp,	unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	int err = 0;
	u32	tmp;
	struct spi_anyka_slave *slave;

	sdbug("%s: cmd=%u\n", __func__, cmd);
	
	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	/* Check access direction once here; don't repeat below.
	 * IOC_DIR is from the user perspective, while access_ok is
	 * from the kernel perspective; so they look reversed.
	 */
	if (_IOC_DIR(cmd) & _IOC_READ){
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	
	if (err == 0 && _IOC_DIR(cmd) & _IOC_WRITE){
		err = !access_ok(VERIFY_READ,	(void __user *)arg, _IOC_SIZE(cmd));
	}
	
	if (err){
		return -EFAULT;
	}

	slave = filp->private_data;
	if (!slave){
		return -ESHUTDOWN;
	}

	mutex_lock(&slave->slave_lock);

	switch (cmd){
	 /* read requests */
	 case SPI_IOC_RD_MODE:
		 retval = __put_user(slave->mode & SPI_MODE_MASK, (__u8 __user *)arg);
		 break;
	 case SPI_IOC_RD_LSB_FIRST:
		 retval = __put_user((slave->mode & SPI_LSB_FIRST) ?	1 : 0, (__u8 __user *)arg);
		 break;
	 case SPI_IOC_RD_BITS_PER_WORD:
		 retval = __put_user(slave->bits_per_word, (__u8 __user *)arg);
		 break;
	 case SPI_IOC_RD_MAX_SPEED_HZ:
		 retval = __put_user(slave->max_speed_hz, (__u32 __user *)arg);
		 break;
 
	 /* write requests */
	 case SPI_IOC_WR_MODE:
		 retval = __get_user(tmp, (u8 __user *)arg);
		 if (retval == 0) {
			 u8  save = slave->mode;
 
			 if (tmp & ~SPI_MODE_MASK) {
				 retval = -EINVAL;
				 break;
			 }
 
			 tmp |= slave->mode & ~SPI_MODE_MASK;
			 slave->mode = (u8)tmp;
			 retval = akspi_slave_setmode(slave);
			 if (retval < 0)
				 slave->mode = save;
			 else
				 dev_dbg(&slave->pdev->dev, "spi mode %02x\n", tmp);
		 }
		 break;
	 case SPI_IOC_WR_LSB_FIRST:
		 retval = -EINVAL;
		 break;
	 case SPI_IOC_WR_BITS_PER_WORD:
		 retval = -EINVAL;
		 break;
	 case SPI_IOC_WR_MAX_SPEED_HZ:
		 retval = __get_user(tmp, (__u32 __user *)arg);
		 if (retval == 0) {
			 u32 save = slave->max_speed_hz;
 
			 slave->max_speed_hz = tmp;
			 retval = akspi_slave_setclk(slave);
			 if (retval < 0)
				 slave->max_speed_hz = save;
			 else
				 dev_dbg(&slave->pdev->dev, "%d Hz (max)\n", tmp);
		 }
		 break;
 
	 default:
		 retval = -EINVAL;
		 break;
	 }
	 
	 mutex_unlock(&slave->slave_lock);
	 return retval;
 }

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		read slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*buf
*  @param[in]   	*filp
*  @return      	read data count
*/
static ssize_t akspi_slave_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct spi_anyka_slave *slave = filp->private_data;

	if (mutex_lock_interruptible(&slave->slave_lock)){
		return -ERESTARTSYS;
	}

	sdbug("%s: %d\n", __func__, __LINE__);
	while (slave->recvp == slave->readp){
		mutex_unlock(&slave->slave_lock);
		if (filp->f_flags & O_NONBLOCK){
			return -EAGAIN;
		}
		if (wait_event_interruptible(slave->readq, (slave->recvp != slave->readp))){
			return -ERESTARTSYS;
		}
		if (mutex_lock_interruptible(&slave->slave_lock)){
			return -ERESTARTSYS;
		}
		sdbug("%s: %d\n", __func__, __LINE__);
	}
	sdbug("%s: %d\n", __func__, __LINE__);

	if (slave->recvp > slave->readp)	{
		/* return the data */
		count = min(count, (size_t)(slave->recvp - slave->readp));
	}
	else	{
		/* the write pointer has wrapped, return data up to end */	
		count = min (count, (size_t)(slave->rdend - slave->readp));
	}
	
	if (copy_to_user(buf, slave->readp, count)) {
		mutex_unlock(&slave->slave_lock);
		return -EFAULT;
	}

	slave->readp += count;
	if (slave->readp == slave->rdend){
		slave->readp = slave->rdbuf;
	}
	
	mutex_unlock(&slave->slave_lock);
	sdbug("%s: %d\n", __func__, __LINE__);
	return count;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		write slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*buf
*  @param[in]   	*filp, count, pos
*  @return      	write data count
*/
static ssize_t akspi_slave_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct spi_anyka_slave *slave = filp->private_data;

	sdbug("%s: %d\n", __func__, __LINE__);

	return count;

	if (mutex_lock_interruptible(&slave->slave_lock)){
		return -ERESTARTSYS;
	}

	while (slave->sentp != slave->writep){
		mutex_unlock(&slave->slave_lock);
		if (filp->f_flags & O_NONBLOCK){
			return -EAGAIN;
		}
		if (wait_event_interruptible(slave->writeq, slave->sentp != slave->writep)){
			return -ERESTARTSYS;
		}
		if (mutex_lock_interruptible(&slave->slave_lock)){
			return -ERESTARTSYS;
		}
	}

	/* spi slave has something to write */

	mutex_unlock(&slave->slave_lock);
	sdbug("%s: %d\n", __func__, __LINE__);
	return count;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		poll slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*table
*  @param[in]   	*filp
*  @return      	fail or not
*/
unsigned int akspi_slave_poll (struct file *filp, struct poll_table_struct *table)
{
	struct spi_anyka_slave *slave = filp->private_data;
	int mask = 0;

	sdbug("%s: %d\n", __func__, __LINE__);
	mutex_lock(&slave->slave_lock);
	poll_wait(filp, &slave->readq, table);
	poll_wait(filp, &slave->writeq, table);

	if (slave->recvp != slave->readp){
		mask |= POLLIN | POLLRDNORM;	/* readable */
	}

	if (slave->sentp != slave->writep){
		mask |= POLLOUT | POLLWRNORM;	/* writable */
	}
	mutex_unlock(&slave->slave_lock);
	sdbug("%s: %d\n", __func__, __LINE__);
	return mask;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		fasync slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*node
*  @param[in]   	*filp, fd, mode
*  @return      	fail or not
*/
static int akspi_slave_fasync(int fd, struct file *filp, int mode)
{
	struct spi_anyka_slave *slave = filp->private_data;

	return fasync_helper(fd, filp, mode, &slave->async_queue);
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		initialize slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*slave
*  @param[in]   	void
*  @return      	void
*/
static void akspi_slave_initial_setup(struct spi_anyka_slave *slave)
{
	u32 value = 0;

	sdbug("%s: %d\n", __func__, __LINE__);
	clk_enable(slave->clk);

	spin_lock(&slave->regs_lock);
	value =  DFT_CON ;
	iowrite32(value, slave->regs + AK_SPICON);
	spin_unlock(&slave->regs_lock);
	akspi_slave_setclk(slave);
	akspi_slave_setmode(slave);
	sdbug("value=%08x, reg=%08x\n",value, ioread32(slave->regs + AK_SPICON));
	akspi_slave_set_irq(slave, 0);
	
	if (slave->pdata) {
		if (slave->pdata->gpio_setup){
			slave->pdata->gpio_setup(slave->pdata, 1);
		}
	}
}

static const struct file_operations slave_ops = {
	.owner	= THIS_MODULE,
	.open	= akspi_slave_open,
	.release 	= akspi_slave_release,
	.unlocked_ioctl = akspi_slave_ioctl,
	.read	= akspi_slave_read,
	.write	= akspi_slave_write,
	.poll		= akspi_slave_poll,
	.fasync	= akspi_slave_fasync,
};

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		probe slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	slave
*  @param[in]   	*pdev
*  @param[in]   	*pdata
*  @return      	fail or not
*/
static int ak_spi_slave_probe(struct platform_device *pdev)
{
	struct ak_spi_info *pdata;
	struct spi_anyka_slave *slave = NULL;
	struct resource *res;
	int err = 0;

	pdata = pdev->dev.platform_data;
	if (pdata == NULL) 
	{
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_no_pdata;
	}

	/* Allocate Slave with space for drv_data and null dma buffer */
	slave = kzalloc(sizeof(struct spi_anyka_slave), GFP_KERNEL);
	if (!slave) {
		dev_err(&pdev->dev, "cannot alloc mem\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	slave->pdata = pdata;
	slave->pdev 	= pdev;
	cdev_init(&slave->cdev, &slave_ops);
	slave->cdev.owner = THIS_MODULE;
	
	spin_lock_init(&slave->regs_lock);
	mutex_init(&slave->slave_lock);
	init_waitqueue_head(&slave->readq);
	init_waitqueue_head(&slave->writeq);
	slave->mode = SPI_MODE_0;
	slave->bits_per_word = 8;
	slave->max_speed_hz = 2147483647;

	/* get basic io resource and map it */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	slave->ioarea = request_mem_region(res->start, resource_size(res), pdev->name);
	if (slave->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	slave->regs = ioremap(res->start, resource_size(res));
	if (!slave->regs) {
		err = -ENOMEM;
		goto err_no_iomap;
	}
	printk(KERN_INFO "SPI-Slave:map regs = %08x\n", (int)slave->regs);

	/* Attach to IRQ */
	slave->irq = platform_get_irq(pdev, 0);
	if (slave->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}
	printk(KERN_INFO "SPI-Slave:get irq = %04x\n", (int)slave->irq);
	
	err = request_irq(slave->irq, akspi_slave_int, IRQF_DISABLED, pdev->name, slave);
	if (err < 0) {
		dev_err(&pdev->dev, "can not get IRQ\n");
		goto err_no_irq;
	}
	printk(KERN_INFO "SPI-Slave: request IRQ: %04x\n", slave->irq);

	err = cdev_add(&slave->cdev, MKDEV(slave_major, slave_minor), 1);
	if (err){
		dev_err(&pdev->dev, "cannot add cdev\n");
		err = -ENOMEM;
		goto err_register;
	}
	printk(KERN_INFO "SPI-Slave: register with char device framework\n");

	if (IS_ERR(device_create(slave_class, &pdev->dev,
				MKDEV(slave_major, slave_minor),
				slave, "spi_slave.%u", slave_minor))){
		dev_err(&pdev->dev, "cannot device_create\n");
	}

	slave->clk = clk_get(&pdev->dev, pdata->clk_name);
	sdbug("%s: %lu \n", slave->clk->name, clk_get_rate(slave->clk));
	if (IS_ERR(slave->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(slave->clk);
		goto err_no_clk;
	}
	
	akspi_slave_initial_setup(slave);
	
	platform_set_drvdata(pdev, slave);
	printk("Ak spi slave initialized!\n");
	return 0;

 err_no_clk:
	device_destroy(slave_class, MKDEV(slave_major, slave_minor));
	cdev_del(&slave->cdev);
 err_register:
	free_irq(slave->irq, slave);
 err_no_irq:
	iounmap(slave->regs);
 err_no_iomap:
	release_resource(slave->ioarea);
	kfree(slave->ioarea);
 err_no_iores:
 err_no_pdata: 
 err_nomem:
	return err;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		remove slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*pdev
*  @param[in]   	*slave
*  @return      	void
*/
static void __devexit ak_spi_slave_remove(struct platform_device *pdev)
{
	struct spi_anyka_slave *slave = platform_get_drvdata(pdev);
	int minor = MINOR(slave->cdev.dev);

	if (!slave)
		return;

	platform_set_drvdata(pdev, NULL);
	device_destroy(slave_class, MKDEV(slave_major, minor));
	cdev_del(&slave->cdev);

	/* Release IRQ */
	free_irq(slave->irq, slave);
	iounmap(slave->regs);
	release_resource(slave->ioarea);
	kfree(slave->ioarea);
	kfree(slave);

	return;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		suspend and resume slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	*dev
*  @param[in]   	state
*  @return      	fail or not
*/
#ifdef CONFIG_PM
static int ak_spi_slave_suspend(struct device *dev, pm_message_t state)
{
	struct spi_anyka_slave *slave = dev_get_drvdata(dev);
	
	printk(KERN_ERR "%s: suspend\n", slave->pdev->name);
	return 0;
}

static int ak_spi_slave_resume(struct device *dev)
{
	struct spi_anyka_slave *slave = dev_get_drvdata(dev);

	printk(KERN_ERR "%s: resume\n",  slave->pdev->name);
	return 0;
}
#else
#define ak_spi_slave_suspend NULL
#define ak_spi_slave_resume NULL
#endif /* CONFIG_PM */

static struct platform_driver akspi_slave_driver = {
	.driver = {
		.name	= "akspi-spi",
		.owner	= THIS_MODULE,
		.suspend = ak_spi_slave_suspend,
		.resume 	= ak_spi_slave_resume,
	},
	.remove		= __exit_p(ak_spi_slave_remove),
	.probe		= ak_spi_slave_probe,
};

static ssize_t slave_show_version(struct class *cls, struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "spi-1.0.01\n");
}
static CLASS_ATTR(version, 0444, slave_show_version, NULL);

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		init slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	void
*  @param[in]   	void
*  @return      	fail or not
*/
static int __init ak_spi_slave_init(void)
{
	int ret;
	dev_t dev;

	slave_class = class_create(THIS_MODULE, "spi_slave");
	if (IS_ERR(slave_class)){
		ret = PTR_ERR(slave_class);
		printk("%s:class create fail!\n", __func__);
		goto err;
	}

	ret = class_create_file(slave_class, &class_attr_version);
	if (ret){
		printk("%s:class create file fail!\n", __func__);
		goto err_class;
	}

	ret = alloc_chrdev_region(&dev, 0, SLAVE_MAX_MINOR, "spi_slave");
	if (ret){
		printk("%s:alloc chrdev fail!\n", __func__);
		goto err_chrdev;
	}
	slave_major = MAJOR(dev);

	ret = platform_driver_register(&akspi_slave_driver);
	if (ret){
		printk("%s:platform_driver_register fail!\n", __func__);
		goto err_plat;
	}

	return 0;
	
err_plat:
	unregister_chrdev_region(dev, SLAVE_MAX_MINOR);
err_chrdev:
	class_remove_file(slave_class, &class_attr_version);
err_class:
	class_destroy(slave_class);
err:
	return ret;
}

/**
*  @Copyright (C) 	Anyka 2012
*  @brief       		exit slave device
*  @author   		Gao wangsheng
*  @email		gao_wangsheng@anyka.oa
*  @date        	2012-10-23
*  @param[out]  	void
*  @param[in]   	void
*  @param[in]   	void
*  @return      	void
*/
static void __exit ak_spi_slave_exit(void)
{
	platform_driver_unregister(&akspi_slave_driver);
	unregister_chrdev_region(MKDEV(slave_major, 0), SLAVE_MAX_MINOR);
	class_remove_file(slave_class, &class_attr_version);
	class_destroy(slave_class);
}

MODULE_AUTHOR("Wangsheng Gao");
MODULE_DESCRIPTION("Anyka SPI Slave Contoller");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AK-spi-slave");
module_init(ak_spi_slave_init);
module_exit(ak_spi_slave_exit);
