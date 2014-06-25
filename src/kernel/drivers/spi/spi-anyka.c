/**
*  @file      /driver/spi/spi_anyka.c
*  @brief     AK On-chip SPI driver
*   Copyright C 2011 Anyka CO.,LTD
*   modify based on  spi_s3c24xx.c
*
*   Copyright (c) 2006 Ben Dooks
*   Copyright (c) 2006 Simtec Electronics
*	Ben Dooks <ben@simtec.co.uk>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*  @author    zhou wenyong
*  @date      2011-08-19
*  @note      2011-5-16  created
*  @note      2011-08-19 add more comments
*/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <mach/gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>

#include <mach/spi.h>
#include <mach/clock.h>
#include <plat-anyka/anyka_types.h>
#include <plat/l2.h>
#include <linux/dma-mapping.h>
#include <plat-anyka/drv_module_lock.h>
#include <mach/reset.h>

//#define SPI_DEBUG

#define TMPDBG(fmt, args...) //printk( KERN_INFO fmt,## args)

#define TMPDEBUG(fmt, args...) //printk( KERN_INFO fmt,## args)

/* #define DEBUG */
#undef PDEBUG           /* undef it, just in case */
#ifdef SPI_DEBUG
# ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
# define PDEBUG(fmt, args...) printk( KERN_INFO fmt,## args)
# else
/* This one for user space */
# define PDEBUG(fmt, args...) fprintf(stderr, "%s %d: "fmt,__FILE__, __LINE__, ## args)
# endif
#else
# define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

	/*usually use for spi keyboard or spi mouse.*/
//#define SPI_CPU_MODE_USE_INTERRUPT


/**
 * ak_spi_devstate - per device data
 * @hz: Last frequency calculated for @sppre field.
 * @mode: Last mode setting for the @spcon field.
 * @spcon: Value to write to the SPCON register.
 * @sppre: Value to write to the SPIINT register.
 */
struct ak_spi_devstate {
	unsigned int	hz;
	u16		mode;
	u32		spcon;
	u8		spint;
	u8  	initialed;
};

struct ak_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;
	struct spi_master	*master;
	struct spi_device	*curdev;
	struct device		*dev;
	struct ak_spi_info 	*pdata;

	void __iomem		*regs;
	struct clk			*clk;
	struct resource		*ioarea;
	int			 		irq;
	int			 		len; 	/*need transfer len*/
	int			 		count;	/*have transferred len*/
	
	u8 					l2buf_tid;
	u8 					l2buf_rid;

	/* data buffers */
	unsigned char		*tx;
	unsigned char		*rx;
	int 				xfer_dir;
	int 				xfer_mode; /*use for dma or cpu*/
};

enum spi_xfer_dir {
	SPI_DIR_TX,
	SPI_DIR_RX,
	SPI_DIR_TXRX, 
	SPI_DIR_XFER_NUM,
};


#define TRANS_TIMEOUT 			(10000)
#define MAX_XFER_LEN 			(8*1024)
#define SPI_TRANS_TIMEOUT 		(5000)

#define DFT_CON 			(AK_SPICON_EN | AK_SPICON_MS)
#define DFT_DIV				(1) //5 /*127*/
#define DFT_BIT_PER_WORD 	(8)
#define FORCE_CS   			(1 << 5)
#define SPPIN_DEFAULT 		(0)

#if 1
/**
*  @brief       print the value of registers related spi bus.
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *hw
*  @return      void
*/
static void dbg_dumpregs(struct ak_spi *hw)
{
	PDEBUG("\n");
	PDEBUG("CON: \t0x%x\n", ioread32(hw->regs + AK_SPICON));
	PDEBUG("STA: \t0x%x\n", ioread32(hw->regs + AK_SPISTA));
	PDEBUG("INT: \t0x%x\n", ioread32(hw->regs + AK_SPIINT));
	PDEBUG("CNT: \t0x%x\n", ioread32(hw->regs + AK_SPICNT));
	PDEBUG("DOUT: \t0x%x\n", ioread32(hw->regs + AK_SPIOUT));
	PDEBUG("DIN: \t0x%x\n", ioread32(hw->regs + AK_SPIIN));
}

static inline void dbg_dumpdata(struct ak_spi *hw,
		void *data, int size)
{
	int ii;
	int dsize = (size +3)/4;
	u32 *dptr = data;
	
	printk("xfer data (size:%d):", size);
	
	for(ii = 0; ii < dsize; ii++) {
		if((ii%10) == 0)
			printk("\n");
		
		printk("%08x ", *(dptr + ii));
	}
	printk("\n");
}
#endif


#define AKSPI_DATA_WIRE(mode) 	\
	(((mode & XFER_4DATAWIRE) == XFER_4DATAWIRE) ?  \
	AKSPI_4DATAWIRE:((mode & XFER_2DATAWIRE) == XFER_2DATAWIRE) ?  \
	AKSPI_2DATAWIRE : AKSPI_1DATAWIRE)


#define SPI_L2_TXADDR(m)   \
	((m->bus_num == AKSPI_BUS_NUM1) ? ADDR_SPI1_TX : ADDR_SPI2_TX)

#define SPI_L2_RXADDR(m)   \
	((m->bus_num == AKSPI_BUS_NUM1) ? ADDR_SPI1_RX : ADDR_SPI2_RX)

 
#define SPI_SHAREPIN(m)   \
	((m->bus_num == AKSPI_BUS_NUM1) ? ePIN_AS_SPI1 : ePIN_AS_SPI2)

#define SPI_RESET_NUM(m)	\
	((m->bus_num == AKSPI_BUS_NUM1) ? AK39_SRESET_SPI1 : AK39_SRESET_SPI2)


static inline struct ak_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

/**
*  @brief       hw_txbyte
*  TODO: send one bytes
*  @author      lixinhai
*  @date        2013-03-19
*  @param[in]   *hw
*  @param[in]   count
*  @return      unsigned int
*/
static inline unsigned int hw_txbyte(struct ak_spi *hw, int len)
{
	u32 val = 0;
	int i = 0;
	
	while (i < len)
	{
		val |= (hw->tx[hw->count+i] << i*8);
		i++;
	}						
	return val;
}

/**
*  @brief       hw_txdword
*  TODO: send double words
*  @author      lixinhai
*  @date        2013-03-19
*  @param[in]   *hw
*  @param[in]   count
*  @return      unsigned int
*/
static unsigned int hw_txdword(struct ak_spi *hw)
{
	u32 val = 0;
	int l = 0;
	
	l = (hw->len - hw->count) > 4 ? 4 : (hw->len - hw->count);

	val = hw_txbyte(hw, l);
					
	hw->count += l;
	PDEBUG("[%08x] ", val);
	return val;
}

/**
*  @brief       hw_rxbyte
*  TODO: recv one bytes
*  @author      lixinhai
*  @date        2013-03-19
*  @param[in]   count
*  @return      unsigned int
*/
static inline void hw_rxbyte(struct ak_spi *hw, unsigned int val, int len)
{
	int i = 0;
	
	while (i < len)
	{
		hw->rx[hw->count + i] = (val >> i*8) & 0xff;
		i++;
	}
}

/**
*  @brief       hw_rxbyte
*  TODO: double words
*  @author      lixinhai
*  @date        2013-03-19
*  @param[in]   count
*  @return      unsigned int
*/
static void hw_rxdword(struct ak_spi *hw, unsigned int val)
{
	int l = 0;
	
	l = (hw->len - hw->count) > 4 ? 4 : (hw->len - hw->count);
	
	hw_rxbyte(hw, val, l);
	hw->count += l;	
	PDEBUG("[%08x] ", val);
}

static inline u32 hw_remain_datalen(struct ak_spi *hw)
{
	return (hw->len - hw->count);
}

static inline bool ak_spi_use_dma(struct ak_spi *hw)
{
	return (hw->xfer_mode == AKSPI_XFER_MODE_DMA);
}

static inline int wait_for_spi_cnt_to_zero(struct ak_spi *hw, u32 timeout)
{
	do {			 
		if (readl(hw->regs + AK_SPICNT) == 0)
			break;
		udelay(1);
	}while(timeout--);
	
	return (timeout > 0) ? 0 : -EBUSY;
}


static void ak_spi_gpio_setup(struct ak_spi *hw, int enable)
{
	struct ak_spi_info *plat = hw->pdata;

	if(!plat)
		return;

	if (enable)
	{
		int ii;
		ak_group_config(SPI_SHAREPIN(hw->master));

		for(ii=0; ii<plat->num_cs; ii++) {
			if(ii == AKSPI_ONCHIP_CS)
				continue;
			
			ak_setpin_as_gpio(plat->pin_cs[ii]);
			ak_gpio_dircfg(plat->pin_cs[ii], AK_GPIO_DIR_OUTPUT);
			ak_gpio_setpin(plat->pin_cs[ii], 1);
		}
	}
}

static void ak_spi_set_cs(struct ak_spi *hw, int cs, int pol)
{
	struct ak_spi_info *plat = hw->pdata;

	if(!plat)
		return;

	BUG_ON(cs >= plat->num_cs);

	ak_gpio_setpin(plat->pin_cs[cs], pol);
}



/**
*  @brief       ak_spi_chipsel
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *spi
*  @param[in]   value
*  @return      void
*/
static void ak_spi_chipsel(struct spi_device *spi, int value)
{
	struct ak_spi_devstate *cs = spi->controller_state;
	struct ak_spi *hw = to_hw(spi);
	unsigned int cspol = (spi->mode & SPI_CS_HIGH) ? 1 : 0;
	
	/* change the chipselect state and the state of the spi engine clock */	
	switch (value) {
		case BITBANG_CS_INACTIVE:
			PDEBUG("BITBANG_CS_INACTIVE\n");

			if(spi->chip_select == AKSPI_ONCHIP_CS) {
				cs->spcon = ioread32(hw->regs + AK_SPICON);
				cs->spcon &= ~FORCE_CS;
				iowrite32(cs->spcon, hw->regs + AK_SPICON);
			}
			else
				ak_spi_set_cs(hw, spi->chip_select, cspol^1);	
			
			break;
		case BITBANG_CS_ACTIVE:
			PDEBUG("BITBANG_CS_ACTIVE");
			
			if(spi->chip_select == AKSPI_ONCHIP_CS) {
				cs->spcon = ioread32(hw->regs + AK_SPICON);
				cs->spcon |= FORCE_CS; //by Shaohua			
				iowrite32(cs->spcon, hw->regs + AK_SPICON);
			}
			else
				ak_spi_set_cs(hw, spi->chip_select, cspol);
			
			break;
		default:
			break;
	}
}

/**
*  @brief       update the device's state
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi
*  @param[in]   *t
*  @return      int
*/
static int ak_spi_update_state(struct spi_device *spi,
				    struct spi_transfer *t)
{
	struct ak_spi_devstate *cs = spi->controller_state;
	unsigned int bpw;
	unsigned int hz, typical_hz;
	unsigned int div = 0;
	unsigned long clk;

	bpw = t ? t->bits_per_word : spi->bits_per_word;
	typical_hz  = t ? t->speed_hz : spi->max_speed_hz;

	if (!bpw)
		bpw = DFT_BIT_PER_WORD;

	if (bpw != DFT_BIT_PER_WORD) {
		dev_err(&spi->dev, "invalid bits-per-word (%d)\n", bpw);
		return -EINVAL;
	}

	/*spi mode change, config again*/
	if (spi->mode != cs->mode) {
		cs->spcon &= ~(AK_SPICON_CPHA|AK_SPICON_CPOL);
		
		if ((spi->mode & SPI_CPHA) == SPI_CPHA)
			cs->spcon |= AK_SPICON_CPHA;

		if ((spi->mode & SPI_CPOL) == SPI_CPOL)
			cs->spcon |= AK_SPICON_CPOL;

		cs->mode = spi->mode;
	}

	if (cs->hz != typical_hz) {
		if (!typical_hz)
			typical_hz = spi->max_speed_hz;
		
		clk = ak_get_asic_clk();
		div = clk / (typical_hz*2) - 1;

		if (div > 255)
			div = 255;
		
		if (div < 0)
			div = 0;
		
		hz = clk /((div+1)*2);

		/*when got clock greater than wanted clock, increase divider*/
		if((hz - typical_hz) > 0)
			div++;

		printk("pre-scaler=%d (wanted %ldMhz, got %ldMhz)\n",
			div, typical_hz/MHz, (clk / (2 * (div + 1)))/MHz);

		cs->hz = hz;
		cs->spcon &= ~AK_SPICON_CLKDIV;
		cs->spcon |= div << 8;
	}
	PDEBUG("spi new hz is %u, div is %u(%s).\n", cs->hz, div, div ?"change":"not change");
    return 0;
}

/**
*  @brief       setup one transfer
*  setup_transfer() changes clock and/or wordsize to match settings
*  for this transfer; zeroes restore defaults from spi_device.
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi
*  @param[in]   *t
*  @return      int
*/
static int ak_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t)
{
	struct ak_spi_devstate *cs = spi->controller_state;
	struct ak_spi *hw = to_hw(spi);
	int ret=0;

	/*spi device not change and has been initilize, return.*/
	if(likely((cs->initialed == 1) && (spi == hw->curdev)))
		return ret;

	cs->spcon = readl(hw->regs + AK_SPICON);

	ret = ak_spi_update_state(spi, t);
	if (!ret)
	{
	    /*Add code to fix the issue that the SPICON register 
	          was deranged. Shaohua @2012-3-23	           
	     */
		writel(cs->spcon, hw->regs + AK_SPICON);
	    printk("ak_spi_setupxfer,con:%08x.\n", cs->spcon);
		cs->initialed = 1;
		hw->curdev = spi;
	}

	return ret;
}

#define MODEBITS (SPI_CPOL | SPI_CPHA | SPI_CS_HIGH)

/**
*  @brief       ak_spi_setup
*  updates the device mode and clocking records used by a
*  device's SPI controller;
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi -- Master side proxy
*  @return      int -- return 0 on success
*/
static int ak_spi_setup(struct spi_device *spi)
{
	struct ak_spi_devstate *cs = spi->controller_state;
	struct ak_spi *hw = to_hw(spi);
	int ret;

	printk("ak_spi setup the master.\n");
	
	/* allocate settings on the first call */
	if (!cs) {
		cs = kzalloc(sizeof(struct ak_spi_devstate), GFP_KERNEL);
		if (!cs) {
			dev_err(&spi->dev, "no memory for controller state\n");
			return -ENOMEM;
		}

		cs->spcon = DFT_CON;
		cs->hz = -1;
		spi->controller_state = cs;
		cs->initialed = 0;
	}

	if (spi->mode & ~(hw->pdata->mode_bits)) {
		dev_dbg(&spi->dev, "setup: unsupported mode bits %x\n",
			spi->mode & ~(hw->pdata->mode_bits));
		return -EINVAL;
	}
	/* initialise the state from the device */
	ret = ak_spi_update_state(spi, NULL);
	if (ret)
		return ret;

	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		hw->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick ? */
	}
	spin_unlock(&hw->bitbang.lock);

	return 0;
}

/**
*  @brief       ak_spi_cleanup
*  called on to free memory provided by spi_master 
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *spi
*  @return      void
*/
static void ak_spi_cleanup(struct spi_device *spi)
{
	kfree(spi->controller_state);
}


/**
*  @brief       enable the irq mask.
*  @author      lixinhai
*  @date        2013-03-10
*  @param[in]  *hw :akspi master dev.
*  @param[in]   imask: interrupt bit mask.
*  @return      new mask.
*/
static inline u32 enable_imask(struct ak_spi *hw, u32 imask)
{
	u32 newmask;

	newmask = readl(hw->regs + AK_SPIINT);
	newmask |= imask;

	writel(newmask, hw->regs + AK_SPIINT);

	return newmask;
}


/**
*  @brief       disable the irq mask.
*  @author      lixinhai
*  @date        2013-03-10
*  @param[in]  *hw :akspi master dev.
*  @param[in]   imask: interrupt bit mask.
*  @return      new mask.
*/
static inline u32 disable_imask(struct ak_spi *hw, u32 imask)
{
	u32 newmask;

	newmask = readl(hw->regs + AK_SPIINT);
	newmask &= ~imask;

	writel(newmask, hw->regs + AK_SPIINT);

	return newmask;
}


/**
*  @brief       configure the master register when start transfer data.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   t: a read/write buffer pair.
*  @return      void
*/
static void ak_spi_start_txrx(struct ak_spi *hw, struct spi_transfer *t)
{
    u32 reg_value;
	enum spi_xfer_dir dir = hw->xfer_dir;
		
	PDEBUG("the spi transfer mode is %s.\n", (dir == SPI_DIR_TXRX) ? 
				"txrx" : (dir == SPI_DIR_RX)? "rx":"tx");
	
    reg_value = readl(hw->regs + AK_SPICON);

	switch(dir) {
		case SPI_DIR_TX:
		    reg_value &= ~AK_SPICON_TGDM;
   			reg_value |= AK_SPICON_ARRM;
			break;
		case SPI_DIR_RX:
		    reg_value |= AK_SPICON_TGDM;
    		reg_value &= ~AK_SPICON_ARRM;
			break;
		case SPI_DIR_TXRX:
			reg_value &= ~AK_SPICON_TGDM;
			reg_value &= ~AK_SPICON_ARRM;
			break;
		default:
			break;
	}

	/*configure the data wire*/
	reg_value &= ~AK_SPICON_WIRE;
	reg_value |= AKSPI_DATA_WIRE(t->xfer_mode);

    writel(reg_value, hw->regs + AK_SPICON);
}


/**
*  @brief       configure the master register when stop transfer data.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   t: a read/write buffer pair.
*  @return      void
*/
static void ak_spi_stop_txrx(struct ak_spi *hw, struct spi_transfer *t)
{
	u32 reg_value;

    reg_value = readl(hw->regs + AK_SPICON);
	reg_value &= ~AK_SPICON_WIRE;	
    writel(reg_value, hw->regs + AK_SPICON);
}


/**
*  @brief       spi write data function by l2dma/l2cpu mode.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   buf: transfer data pointer.
*  @param[in]   count: transfer data length.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int spi_dma_write(struct ak_spi *hw, unsigned char *buf, int count)
{
	int ret = 0;
	bool flags = false;
	u32 val;
	dma_addr_t phyaddr = 0;

	init_completion(&hw->done);	
	enable_imask(hw, AK_SPIINT_TRANSF);
	
	val = AK_SPIEXTX_BUFEN|AK_SPIEXTX_DMAEN;
	iowrite32(val, hw->regs + AK_SPIEXTX);
	iowrite32(count, hw->regs + AK_SPICNT);

	/*use for dma mode: greater than 256 bytes, align 4bytes of buf addr,
		align 64 bytes of data count*/
	if((count < 256) || ((unsigned long)buf & 0x3) || (count & (64 - 1))) {
		l2_combuf_cpu((unsigned long)buf, hw->l2buf_tid, count, MEM2BUF);
		
	} else {
		phyaddr = dma_map_single(hw->dev, buf, count, DMA_TO_DEVICE);
		if (phyaddr == 0) {
			printk("tx dma_map_single error!\n");
			ret = -EINVAL;
			goto wr_exit;
		}
	
		//start l2 dma transmit
		l2_combuf_dma(phyaddr, hw->l2buf_tid, count, MEM2BUF, AK_FALSE);
		flags = true;
	}
	
	ret = wait_for_completion_timeout(&hw->done, msecs_to_jiffies(SPI_TRANS_TIMEOUT));
	if(ret <= 0) {
		printk("wait for spi transfer interrupt timeout(%s).\n", __func__);
		dbg_dumpregs(hw);
		ret = -EINVAL;
		goto xfer_fail;
	}

	if (flags && (l2_combuf_wait_dma_finish(hw->l2buf_tid) == AK_FALSE))	{
		printk("%s: l2_combuf_wait_dma_finish failed!\n", __func__);
		ret = -EINVAL;
		goto xfer_fail;
	}
	
	ret = wait_for_spi_cnt_to_zero(hw, TRANS_TIMEOUT);
	if(ret)	{
		printk("%s: wait_for_spi_cnt_to_zero failed!\n", __func__);
		ret = -EINVAL;
		goto xfer_fail;
	}
		
	ret = count;
xfer_fail:
	if(phyaddr)
		dma_unmap_single(hw->dev, phyaddr, count, DMA_TO_DEVICE);
	
wr_exit:
	//disable l2 dma
	iowrite32(0, hw->regs + AK_SPIEXTX);
	l2_clr_status(hw->l2buf_tid);
	return ret;
}


/**
*  @brief       spi read data function by l2dma/l2cpu mode.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   buf: transfer data pointer.
*  @param[in]   count: transfer data length.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int spi_dma_read(struct ak_spi *hw, unsigned char *buf, int count)
{
	int ret = 0;
	bool flags = false;
	u32 val;
	dma_addr_t phyaddr = 0;
	
	//prepare spi read
	init_completion(&hw->done);
	enable_imask(hw, AK_SPIINT_TRANSF);
	
	val = AK_SPIEXTX_BUFEN|AK_SPIEXTX_DMAEN;
	iowrite32(val, hw->regs + AK_SPIEXRX);
	iowrite32(count, hw->regs + AK_SPICNT);	

	if(count < 256 || ((unsigned long)buf & 0x3) || (count & (64 - 1))) {
		l2_combuf_cpu((unsigned long)buf, hw->l2buf_rid, count, BUF2MEM);
	} 
	else {
		phyaddr = dma_map_single(hw->dev, buf, count, DMA_FROM_DEVICE);
		if (phyaddr == 0) {
			printk("tx dma_map_single error!\n");
			ret =  -EINVAL;
			goto rd_exit;
		}

		//start L2 dma
		l2_combuf_dma(phyaddr, hw->l2buf_rid, count, BUF2MEM, AK_FALSE);
		flags = true;
	}

	ret = wait_for_completion_timeout(&hw->done, msecs_to_jiffies(SPI_TRANS_TIMEOUT));
	if(ret <= 0) {
		printk("wait for spi transfer interrupt timeout(%s).\n", __func__);
		dbg_dumpregs(hw);
		ret = -EINVAL;
		goto xfer_fail;
	}

	//wait L2 dma finish, if need frac dma,start frac dma
	if (flags && l2_combuf_wait_dma_finish(hw->l2buf_rid) ==  AK_FALSE)	{
		ret = -EINVAL;
		goto xfer_fail;
	}

	/*wait for spi count register value to zero.*/
	ret = wait_for_spi_cnt_to_zero(hw, TRANS_TIMEOUT);
	if(ret)	{
		printk("%s: wait for spi count to zero failed!\n", __func__);
		ret = -EINVAL;
		goto xfer_fail;
	}

	ret = count;
xfer_fail:
	if(phyaddr)
		dma_unmap_single(hw->dev, phyaddr, count, DMA_FROM_DEVICE);
	
rd_exit:
	//disable l2 dma
	iowrite32(0, hw->regs + AK_SPIEXRX);
	l2_clr_status(hw->l2buf_rid);
	return ret;
}


/**
*  @brief       spi transfer function by l2dma/l2cpu mode.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   tx: send data pointer.
*  @param[in]   rx: receive data pointer.
*  @param[in]   count: transfer data length.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int spi_dma_duplex(struct ak_spi *hw, 
			unsigned char *tx, unsigned char *rx, int count)
{
	int ret = 0;
	bool flags = false;
	dma_addr_t tx_phyaddr, rx_phyaddr;
	u32 val;

	init_completion(&hw->done);	
	enable_imask(hw, AK_SPIINT_TRANSF);
	
	val = AK_SPIEXTX_BUFEN|AK_SPIEXTX_DMAEN;
	iowrite32(val, hw->regs + AK_SPIEXTX);

	val = AK_SPIEXRX_BUFEN|AK_SPIEXRX_DMAEN;
	iowrite32(val, hw->regs + AK_SPIEXRX);

	iowrite32(count, hw->regs + AK_SPICNT);

	if((count < 512) || ((unsigned long)tx & 0x3)) {
		//printk("write: l2_combuf_cpu\n");
		l2_combuf_cpu((unsigned long)tx, hw->l2buf_tid, count, MEM2BUF);
		l2_combuf_cpu((unsigned long)rx, hw->l2buf_rid, count, BUF2MEM);
	}
	else {
		tx_phyaddr = dma_map_single(hw->dev, tx, count, DMA_TO_DEVICE);
		if (tx_phyaddr == 0) {
			printk("tx dma_map_single error!\n");
			ret = -EINVAL;
			goto wr_fail;
		}
	
		//start l2 dma transmit
		l2_combuf_dma(tx_phyaddr, hw->l2buf_tid, count, MEM2BUF, AK_FALSE);

		rx_phyaddr = dma_map_single(hw->dev, rx, count, DMA_FROM_DEVICE);
		if (rx_phyaddr == 0) {
			printk("tx dma_map_single error!\n");
			ret =  -EINVAL;
			goto rd_fail;
		}
		
		//start L2 dma
		l2_combuf_dma(rx_phyaddr, hw->l2buf_rid, count, BUF2MEM, AK_FALSE); 
	}
	ret = wait_for_completion_timeout(&hw->done, msecs_to_jiffies(SPI_TRANS_TIMEOUT));
	if(ret <= 0) {
		printk("wait for spi transfer interrupt timeout(%s).\n", __func__);
		dbg_dumpregs(hw);
		ret = -EINVAL;
		goto xfer_fail;
	}

	if (flags && ((AK_FALSE == l2_combuf_wait_dma_finish(hw->l2buf_tid)) || 
		(AK_FALSE == l2_combuf_wait_dma_finish(hw->l2buf_rid))))
	{
		printk("%s: l2_combuf_wait_dma_finish failed!\n", __func__);
		ret = -EINVAL;
		goto xfer_fail;
	}
	
	ret = wait_for_spi_cnt_to_zero(hw, TRANS_TIMEOUT);
	if(ret)	{
		printk("%s: wait for spi count to zero failed!\n", __func__);
		ret = -EINVAL;
		goto xfer_fail;
	}

	ret = 0;

xfer_fail:
	dma_unmap_single(hw->dev, rx_phyaddr, count, DMA_FROM_DEVICE);

rd_fail:
	dma_unmap_single(hw->dev, tx_phyaddr, count, DMA_TO_DEVICE);

wr_fail:
	//disable l2 dma
	iowrite32(0, hw->regs + AK_SPIEXTX);
	iowrite32(0, hw->regs + AK_SPIEXRX);
	l2_clr_status(hw->l2buf_tid);
	l2_clr_status(hw->l2buf_rid);
	return ret;
}


/**
*  @brief       spi transfer function by l2dma/l2cpu mode, actual worker 
*  				spi_dma_read()/spi_dma_write() to be call.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   dir: transfer direction.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int ak_spi_dma_txrx(struct ak_spi *hw, struct spi_transfer *t)
{
	int ret = 0;
	u32 retlen;
	u32 count = t->len;
	
	hw->tx = (unsigned char*)t->tx_buf;
	hw->rx = t->rx_buf;
	hw->l2buf_tid = hw->l2buf_rid = BUF_NULL;
	PDEBUG("start the spi dma transfer.\n");

	switch(hw->xfer_dir) {
		case SPI_DIR_TXRX:
		{	
			//alloc L2 buffer
			hw->l2buf_tid = l2_alloc(SPI_L2_TXADDR(hw->master));
			hw->l2buf_rid = l2_alloc(SPI_L2_RXADDR(hw->master));

			if ((BUF_NULL == hw->l2buf_tid) || (BUF_NULL == hw->l2buf_rid))
			{
				printk("%s: l2_alloc failed!\n", __func__);
				ret = -EBUSY;
				goto txrx_ret;
			}
			
			while(count > 0) {
				hw->count = 0;
				hw->len = (count > MAX_XFER_LEN) ?	MAX_XFER_LEN : count;
				
				retlen = spi_dma_duplex(hw, hw->tx, hw->rx, hw->len);
				if(unlikely(retlen < 0)) {
					printk("spi master transfer data error!\n");
					ret = -EBUSY;
					goto txrx_ret;
				}
				hw->tx += retlen;
				hw->rx += retlen;
				count -= retlen;
			}
			break;
		}
		case SPI_DIR_TX:
		{
			//alloc L2 buffer
			hw->l2buf_tid = l2_alloc(SPI_L2_TXADDR(hw->master));
			if (unlikely(BUF_NULL == hw->l2buf_tid)) {
				printk("%s: l2_alloc failed!\n", __func__);
				return -EBUSY;
			}

			while(count > 0) {
				hw->count = 0;
				hw->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;
				
				retlen = spi_dma_write(hw, hw->tx + hw->count, hw->len);
				if(unlikely(retlen < 0)) {
					printk("spi master read data error!\n");	
					ret = -EBUSY;
					goto txrx_ret;
				}
				hw->tx += retlen;
				count -= retlen;
			}
			break;
		}
		case SPI_DIR_RX:
		{
			//alloc L2 buffer
			hw->l2buf_rid = l2_alloc(SPI_L2_RXADDR(hw->master));
			if (unlikely(BUF_NULL == hw->l2buf_rid))
			{
				return -EBUSY;
			}
			
			while(count > 0) {
				hw->count = 0;
				hw->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;
				
				retlen = spi_dma_read(hw, hw->rx, hw->len);
				if(unlikely(retlen < 0)) {
					printk("spi master read data error!\n");
					ret = -EBUSY;
					goto txrx_ret;
				}
				hw->rx += retlen;
				count -= retlen;
			}		
			break;
		}
	}
	PDEBUG("finish the spi dma transfer.\n");
txrx_ret:
	if(hw->l2buf_tid != BUF_NULL)
		l2_free(SPI_L2_TXADDR(hw->master));
	if(hw->l2buf_rid != BUF_NULL)
		l2_free(SPI_L2_RXADDR(hw->master));

	return ret ? ret : t->len;
}


/**
*  @brief       spi read/write data function by cpu mode, 
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   t: a read/write buffer pair.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int ak_cpu_duplex(struct spi_device *spi, struct spi_transfer *t)
{
	struct ak_spi *hw = to_hw(spi);
	u32 tran_4_nbr = hw->len/4;
	u32 frac_nbr = hw->len%4;
	u32 status, val;
	u32 off_set_read = 0, off_set_write = 0;
	const u8 *buff_tx;
	u8 *buff_rx;
	int i = 0, j;
	u32 to_read = 0, to_write = 0;	//, to = 0;

	hw->tx = (unsigned char*)t->tx_buf;
	hw->rx = t->rx_buf;

	buff_tx = hw->tx;
	buff_rx = hw->rx;

	PDEBUG("duplex transfer,tran_4_nbr:%u  frac_nbr:%u\n", tran_4_nbr, frac_nbr);
	
	if (hw->len >= MAX_XFER_LEN)
	{
		printk("Too much to be read and send...\n");
		return -EINVAL;
	}

	//set data count, and the the master will rise clk
	writel(hw->len, hw->regs + AK_SPICNT);
	
	while(1) {
		//write 4 bytes first, and then read 4 bytes
		if (i<tran_4_nbr) {
			while(1) {
				status = ioread32(hw->regs + AK_SPISTA);
				if ((status & AK_SPISTA_TXHEMP) == AK_SPISTA_TXHEMP) {
					PDEBUG("TX HEMP...\n");
					break;
				} else {
					if(to_write++ < TRANS_TIMEOUT) 
						udelay(10);
					else {
						PDEBUG("master transfer timeout...\n");
						goto SPI_TRANS_FAIL;
					}
				}
			}			
			iowrite32(*(volatile u32 *)(buff_tx + off_set_write), hw->regs + AK_SPIOUT);
			off_set_write += 4;
			i++;
		}
		//write not finished
		else if (off_set_write < hw->len) {			
			PDEBUG("write frac...\n");
			to_write = 0;
			val = 0;
			if (frac_nbr != 0) {
				while(1) {
					status = ioread32(hw->regs + AK_SPISTA);
					if ((status & AK_SPISTA_TXHEMP) == AK_SPISTA_TXHEMP)
						break;
					if (to_write++ < TRANS_TIMEOUT)	
						udelay(10);
					else {
						printk("SPI master write timeout...\n");						
						goto SPI_TRANS_FAIL;
					}
				}
					
				for (j=0; j<frac_nbr; j++)
				{
					PDEBUG("[%d]:%x ", off_set_write+j, *(buff_tx+off_set_write+j));
					val |= (*(buff_tx + off_set_write + j) << (j*8));
				}
				PDEBUG("\nval: %x", val);
				PDEBUG("\n\n");

				iowrite32(val, hw->regs + AK_SPIOUT); 
				off_set_write += frac_nbr;
			}
		}
		
		//read
		status = ioread32(hw->regs + AK_SPISTA);			
		
		if ((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF) {
			if (status & AK_SPISTA_RXFULL) {
				val = ioread32(hw->regs + AK_SPIIN);
				*(volatile u32 *)(buff_rx + off_set_read) = val;
				off_set_read += 4;

				val = ioread32(hw->regs + AK_SPIIN);
				*(volatile u32 *)(buff_rx + off_set_read) = val;
				off_set_read += 4;

			} else if (status & AK_SPISTA_RXHFULL) {

				val = ioread32(hw->regs + AK_SPIIN);
				*(volatile u32 *)(buff_rx + off_set_read) = val;
				off_set_read += 4;
			}
			if (frac_nbr != 0) {
				PDEBUG("read frac...\n");
				val = ioread32(hw->regs + AK_SPIIN);

				for (j=0; j<frac_nbr; j++)
				{
					*(buff_rx+off_set_read+j) = (val >> j*8) & 0xff;
					PDEBUG("%x ", *(buff_rx+off_set_read+j));
				}
			}
			break;
		} else {
			if ( (status & AK_SPISTA_RXHFULL) == AK_SPISTA_RXHFULL)	{
				val = ioread32(hw->regs + AK_SPIIN);
				*(volatile u32 *)(buff_rx + off_set_read) = val;
				PDEBUG("rx hfull .. %x\n", val);
				PDEBUG("[0]%x [1]%x [2]%x [3]%x\n", buff_rx[0], buff_rx[1], buff_rx[2], buff_rx[3]);
				off_set_read += 4;
			} else {
				if (to_read++ < TRANS_TIMEOUT) 
					udelay(10);
				else {
					PDEBUG("master read timeout...\n");
					goto SPI_READ_TIMEOUT;
				}
			}
		}	
	}
	
	return hw->len;
	
SPI_TRANS_FAIL:
SPI_READ_TIMEOUT:
	return off_set_read > off_set_write ? off_set_read: off_set_write;		
	
}


#if defined(SPI_CPU_MODE_USE_INTERRUPT)
static int pio_imasks[SPI_DIR_XFER_NUM] = {
	AK_SPIINT_TRANSF | AK_SPIINT_TXHEMP,
	AK_SPIINT_TRANSF | AK_SPIINT_RXHFULL,
	AK_SPIINT_TXHEMP | AK_SPIINT_RXHFULL,
};


/**
*  @brief       spi transfer function by cpu mode.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   dir: transfer direction.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int ak_spi_pio_txrx(struct ak_spi *hw, struct spi_transfer *t)
{
	int ret;

	init_completion(&hw->done);

	writel(hw->len, hw->regs + AK_SPICNT);
	enable_imask(hw, pio_imasks[hw->xfer_dir]);

	ret = wait_for_completion_timeout(&hw->done, msecs_to_jiffies(SPI_TRANS_TIMEOUT));
	if(ret <= 0) {
		printk("wait for spi transfer interrupt timeout(%s).\n", __func__);
		dbg_dumpregs(hw);
		return -EINVAL;
	}
	
	return hw->count;
}
#else


/**
*  @brief       ak37_spi_writeCPU
*  send message function with CPU mode and polling mode
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi
*  @param[in]   *t
*  @return      int
*/
static int spi_pio_write(struct ak_spi *hw, unsigned char *buf, int count)
{
	u32 status;
	u32 to = 0;

	PDEBUG("ak spi write by cpu mode\n");
	if (count > 64*1024)
	{
		printk("too much to be send...\n");
		return -EINVAL;
	}
	//set data count, and the the master will rise clk
	writel(count, hw->regs + AK_SPICNT);
	
	while(hw_remain_datalen(hw) > 0) {
		status = readl(hw->regs + AK_SPISTA);
		if ((status & AK_SPISTA_TXHEMP) == AK_SPISTA_TXHEMP) {
			writel(hw_txdword(hw), hw->regs + AK_SPIOUT);
		}else {
			if (to++ > 10 * 1000000) {
				printk("master write data timeout.\n");	
				return hw->count;
			}
		}
	}

	//wait transfer finish
	while(1) {
		status = readl(hw->regs + AK_SPISTA);		
		if ((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF)
			break;
		
		if (to++ > 10 * 1000000) {
			printk("wait for write data finish timeout..\n");	
			return hw->count;
		}
	}

	if (hw_remain_datalen(hw) > 0)
		printk("write wasn't finished.\n");

	return hw->count;
}

/**
*  @brief       spi_pio_read
*  receiving message function with CPU mode and polling mode
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi
*  @param[in]   *t
*  @return      int
*/
static int spi_pio_read(struct ak_spi *hw, unsigned char *buf, int count)
{
	u32 status;
	u32 to=0;
	
	PDEBUG("ak spi read by cpu mode\n");
	if (count >= 64*1024)
	{
		printk("too much to be read...\n");
		return -EINVAL;
	}

	//set data count, and the the master will rise clk
	writel(count, hw->regs + AK_SPICNT);
	
	while(1) {
		status = readl(hw->regs + AK_SPISTA);			
		
		if((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF)
		{
			if(status & AK_SPISTA_RXFULL) {
				hw_rxdword(hw, readl(hw->regs + AK_SPIIN));
				hw_rxdword(hw, readl(hw->regs + AK_SPIIN));
			}else if (status & AK_SPISTA_RXHFULL) {
				hw_rxdword(hw, readl(hw->regs + AK_SPIIN));
			} 

			if (hw_remain_datalen(hw) > 0) {
				hw_rxdword(hw, readl(hw->regs + AK_SPIIN));
			}
			break;
		} else {
			if((status & AK_SPISTA_RXHFULL) == AK_SPISTA_RXHFULL) {
				hw_rxdword(hw, readl(hw->regs + AK_SPIIN));
			}
			else {
				if (to++ > 10 * 1000000) {
					PDEBUG("master read timeout.\n");
					return hw->count;
				}
			}
		}	
	}
	if (hw_remain_datalen(hw) > 0)
		printk("read wasn't finished.\n");

	return hw->count;	
}


/**
*  @brief       spi transfer function by cpu mode.
*  @author      lixinhai
*  @date        2013-03-15
*  @param[in]  *hw :akspi master dev.
*  @param[in]   dir: transfer direction.
*  @return      transfer success result 0, otherwise result a negative value
*/
static int ak_spi_pio_txrx(struct ak_spi *hw, struct spi_transfer *t)
{
	int retlen;
	u32 count = t->len;
			
	hw->tx = (unsigned char*)t->tx_buf;
	hw->rx = t->rx_buf;
	PDEBUG("start the spi dma transfer.\n");

	switch(hw->xfer_dir) {
		case SPI_DIR_TX:
		{
			while(count > 0) {
				hw->count = 0;
				hw->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;
				
				retlen = spi_pio_write(hw, hw->tx, hw->len);
				if(unlikely(retlen < 0)) {
					printk("spi master transfer data error!\n");
					goto txrx_ret;
				}
				hw->tx += retlen;
				count -= retlen;

			}
			break;
		}
		case SPI_DIR_RX:
		{
			while(count > 0) {
				hw->count = 0;
				hw->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;

				retlen = spi_pio_read(hw, hw->rx, hw->len);
				if(unlikely(retlen < 0)) {
					printk("spi master transfer data error!\n");
					goto txrx_ret;
				}
				hw->rx += retlen;
				count -= retlen;
			}
			break;
		}
	}
	PDEBUG("finish the spi dma transfer.\n");

txrx_ret:
	return (retlen<0) ? retlen : t->len;
}

#endif

/**
*  @brief       transfer a message
*  call proper function to complete the transefer
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   *spi
*  @param[in]   *t
*  @return      int
*/
static int ak_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
    int ret;
	struct ak_spi *hw = to_hw(spi);

	TMPDEBUG("txrx: tx %p, rx %p, len %d\n",
		t->tx_buf, t->rx_buf, t->len);
	
	dev_dbg(&spi->dev, "txrx: tx %p, rx %p, len %d\n",
		t->tx_buf, t->rx_buf, t->len);

	hw->xfer_mode = t->len < 256 ? AKSPI_XFER_MODE_CPU : AKSPI_XFER_MODE_DMA;

	hw->xfer_dir = (t->tx_buf && t->rx_buf) ? SPI_DIR_TXRX : 
				t->tx_buf ? SPI_DIR_TX : SPI_DIR_RX;

	ak_spi_start_txrx(hw, t);

	ak_drv_module_protect(DRV_MODULE_SPI);
	
	if(hw->xfer_dir == SPI_DIR_TXRX) {		
		//printk(KERN_WARNING "this spi master driver no support duplex now!\n");
		ret = ak_cpu_duplex(spi, t);
		ak_drv_module_unprotect(DRV_MODULE_SPI);
		return ret;
	}	

	if(ak_spi_use_dma(hw)) {
		ret = ak_spi_dma_txrx(hw, t);
	} else {
		ret = ak_spi_pio_txrx(hw, t);
	}

	ak_drv_module_unprotect(DRV_MODULE_SPI);
	ak_spi_stop_txrx(hw, t);

	return ret;
}

/**
*  @brief       ak_spi_irq
*  TODO: used by interrupt mode
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]   irq
*  @param[out]  *dev
*  @return      irqreturn_t
*/
static irqreturn_t ak_spi_irq(int irq, void *dev)
{
	struct ak_spi *hw = dev;
	unsigned int status;

	status = readl(hw->regs + AK_SPISTA);

	PDEBUG("spi interrupt: status register value is %08x\n", status);

	if(ak_spi_use_dma(hw)) {
		if((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF ) {
			PDEBUG("spi transfer data have been finish.\n"); 

			//printk("--->status:%d, cnt:%d\n", status, readl(hw->regs + AK_SPICNT));
			disable_imask(hw, AK_SPIINT_TRANSF);
			complete(&hw->done);
		}
	} else {
		switch(hw->xfer_dir) {	
			case SPI_DIR_RX:
				if(status & (AK_SPISTA_RXHFULL | AK_SPISTA_TRANSF)) {
					PDEBUG("spi recv buffer half full or data transfer finish.\n");

					if(status & (AK_SPISTA_RXFULL|AK_SPISTA_TRANSF))
						hw_rxdword(hw, readl(hw->regs + AK_SPIIN));						
					hw_rxdword(hw, readl(hw->regs + AK_SPIIN));

					if (hw->count >= hw->len) {
						disable_imask(hw, AK_SPIINT_RXHFULL|AK_SPIINT_TRANSF);
						complete(&hw->done);
					}				
				}
				break;
			case SPI_DIR_TX:
				if(status & (AK_SPISTA_TXHEMP | AK_SPISTA_TRANSF)) {
					PDEBUG("spi send buffer half empty or data transfer finish.\n");
					
					if (hw->count < hw->len) {
						if((status & AK_SPISTA_TXEMP) && ((hw->count - hw->len)>4))
							writel(hw_txdword(hw), hw->regs + AK_SPIOUT);
						
						writel(hw_txdword(hw), hw->regs + AK_SPIOUT);
					} else {
						disable_imask(hw, AK_SPIINT_TXHEMP|AK_SPIINT_TRANSF);
						complete(&hw->done);
					}
				}
				break;
			case SPI_DIR_TXRX :
			default:
				BUG();
				break;
		}
	}

	return IRQ_HANDLED;
}


/**
*  @brief       ak_spi_initialsetup
*  set up gpio and spi master
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *hw
*  @return      void
*/
static void ak_spi_initialsetup(struct ak_spi *hw)
{
	/* for the moment, permanently enable the clock */
	TMPDEBUG("Entering %s\n", __FUNCTION__);

	BUG_ON(hw->master && hw->master->bus_num >= AKSPI_MAX_BUS_NUM);
	
	ak_soft_reset(SPI_RESET_NUM(hw->master));
	clk_enable(hw->clk);

	/* program defaults into the registers */
	writel(DFT_DIV<<8 | DFT_CON | (1<<1),hw->regs + AK_SPICON);

	writel(0, hw->regs + AK_SPIINT);

	ak_spi_gpio_setup(hw, 1);

    printk("akpi regs: SPICON:%08x, SPISTA:%08x, SPIINT:%08x.\n", 
		ioread32(hw->regs + AK_SPICON), 
		ioread32(hw->regs + AK_SPISTA), 
		ioread32(hw->regs + AK_SPIINT));

}

/**
*  @brief       ak_spi_probe
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *pdev
*  @return      int __init
*/
static int __init ak_spi_probe(struct platform_device *pdev)
{
	struct ak_spi_info *pdata;
	struct ak_spi *hw;
	struct spi_master *master;
	struct resource *res;
	int err = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(struct ak_spi));
	if (master == NULL) 
	{
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct ak_spi));

	hw->master = spi_master_get(master);
	hw->pdata = pdata = pdev->dev.platform_data;
	hw->dev = &pdev->dev;
	hw->xfer_mode = pdata->xfer_mode;

	if (pdata == NULL) 
	{
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_no_pdata;
	}

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the master state. */
	/* the spi->mode bits understood by this driver: */
	master->mode_bits = hw->pdata->mode_bits;

	master->num_chipselect = hw->pdata->num_cs;
	master->bus_num = pdata->bus_num;

	/* setup the state for the bitbang driver */

	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = ak_spi_setupxfer;
	hw->bitbang.chipselect     = ak_spi_chipsel;
	hw->bitbang.txrx_bufs      = ak_spi_txrx;

	hw->master->setup  = ak_spi_setup;
	hw->master->cleanup = ak_spi_cleanup;

	dev_dbg(hw->dev, "bitbang at %p\n", &hw->bitbang);

	/* find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	hw->ioarea = request_mem_region(res->start, resource_size(res),
					pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	hw->regs = ioremap(res->start, resource_size(res));
	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	err = request_irq(hw->irq, ak_spi_irq, 0, pdev->name, hw);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

	hw->clk = clk_get(&pdev->dev, pdata->clk_name);
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_no_clk;
	}
	PDEBUG("%s: %luMhz  \n", hw->clk->name, clk_get_rate(hw->clk)/MHz);

	/* setup any gpio we can */

	ak_spi_initialsetup(hw);

	/* register our spi controller */

	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	printk("akspi master initialize success, use for %s mode.\n", 
		ak_spi_use_dma(hw)?"DMA":"PIO");

	return 0;

 err_register:
	//if (hw->set_cs == ak_spi_gpiocs)
	//	gpio_free(pdata->pin_cs);

	clk_disable(hw->clk);
	clk_put(hw->clk);

 err_no_clk:
	free_irq(hw->irq, hw);

 err_no_irq:
	iounmap(hw->regs);

 err_no_iomap:
	release_resource(hw->ioarea);

 err_no_iores:
 err_no_pdata:
	spi_master_put(hw->master);

 err_nomem:
	return err;
}

/**
*  @brief       ak_spi_remove
*  free allocated resources while remove
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *dev
*  @return      int __exit
*/
static int __exit ak_spi_remove(struct platform_device *dev)
{
	struct ak_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	clk_disable(hw->clk);
	clk_put(hw->clk);

	free_irq(hw->irq, hw);
	iounmap(hw->regs);

	release_resource(hw->ioarea);

	spi_master_put(hw->master);
	return 0;
}


#ifdef CONFIG_PM

/**
*  @brief       ak_spi_suspend
*  suspend, disable the clock to the SPI
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[in]  *dev
*  @return      int
*/
static int ak_spi_suspend(struct device *dev)
{
	struct ak_spi *hw = platform_get_drvdata(to_platform_device(dev));

	ak_spi_gpio_setup(hw, 0);

	clk_disable(hw->clk);
	return 0;
}

/**
*  @brief       ak_spi_resume
*  resume, initialize the spi master
*  @author      zhou wenyong
*  @date        2011-08-19
*  @param[out]  *dev
*  @return      int
*/
static int ak_spi_resume(struct device *dev)
{
	struct ak_spi *hw = platform_get_drvdata(to_platform_device(dev));

	ak_spi_initialsetup(hw);
	return 0;
}

static struct dev_pm_ops ak_spi_pmops = {
	.suspend	= ak_spi_suspend,
	.resume		= ak_spi_resume,
};

#define AK_SPI_PMOPS &ak_spi_pmops
#else
#define AK_SPI_PMOPS NULL
#endif /* CONFIG_PM */

MODULE_ALIAS("platform:ak-spi");
static struct platform_driver ak_spi_driver = {
	.remove		= __exit_p(ak_spi_remove),
	.driver		= {
		.name	= "ak-spi",
		.owner	= THIS_MODULE,
		.pm	= AK_SPI_PMOPS,
	},
};

/**
*  @brief       ak_spi_init
*  init, register this driver
*  @author      zhou wenyong
*  @date        2011-08-19
*  @return      int __init
*/
static int __init ak_spi_init(void)
{
		printk("AK SPI Driver, (c) 2012 ANYKA\n");
        return platform_driver_probe(&ak_spi_driver, ak_spi_probe);
}

/**
*  @brief       ak_spi_exit
*  exit, unregister this driver
*  @author      zhou wenyong
*  @date        2011-08-19
*  @return      void __exit
*/
static void __exit ak_spi_exit(void)
{
        platform_driver_unregister(&ak_spi_driver);
}

module_init(ak_spi_init);
module_exit(ak_spi_exit);

MODULE_DESCRIPTION("AK On-Chip SPI Driver");
MODULE_AUTHOR("ANYKA");
MODULE_LICENSE("GPL");

