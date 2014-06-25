/*
 *  linux/drivers/mmc/host/plat-anyka/akmci.c - Anyka MMC/SD/SDIO driver
 *
 *  Copyright (C) 2010 Anyka, Ltd, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/log2.h>
#include <linux/mmc/host.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/dma-mapping.h>
#include <linux/notifier.h>

#include <asm/cacheflush.h>
#include <asm/div64.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <plat/l2.h>

#include <plat-anyka/adkey.h>
#include <plat-anyka/drv_module_lock.h>
#include <mach/gpio.h>
#include <mach/clock.h>
#include <mach/reset.h>
#include <plat-anyka/akmci.h>

#define DRIVER_NAME 	"akmci"
//#define DRIVER_NAME 	"ak-mci"

#undef PDEBUG

#define ___hdbg___()	//printk("akmci:----func:%s---line:%d----\n", __func__, __LINE__);
#define DDREGS(host)		//dbg_dumpregs(host, __func__, __LINE__)
#define DDDATA(h, d, s)		//dbg_dumpdata(h, d, s)
#define HDBG(fmt, args...) 	//printk(fmt, ##args)

//#define MCI_DBG

#ifdef MCI_DBG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) 	printk(KERN_INFO "akmci:" fmt, ##args)
#else
#define PDEBUG(fmt, args...) 	fprintf(stderr, "%s %d:" fmt,__FILE__, __LINE__, ## args)
#endif
#else
#define PDEBUG(fmt, args...) 
#endif


static inline void dbg_dumpregs(struct akmci_host *host,
		const char *prefix, int eflags)
{
	u32 clkcon, cmdarg, cmd, cmdrsp, rsp1, rsp2, rsp3, rsp4;
	u32 dtimer, datlen, datcon, datcnt, stat, imask, dmamode, cpumode;

	clkcon = readl(host->base + MCI_CLK_REG);
	cmdarg = readl(host->base + MCI_ARGUMENT_REG);
	cmd = readl(host->base + MCI_COMMAND_REG);
	cmdrsp = readl(host->base + MCI_RESPCMD_REG);
	
	rsp1 = readl(host->base + MCI_RESPONSE0_REG);
	rsp2 = readl(host->base + MCI_RESPONSE1_REG);
	rsp3 = readl(host->base + MCI_RESPONSE2_REG);
	rsp4 = readl(host->base + MCI_RESPONSE3_REG);

	dtimer = readl(host->base + MCI_DATATIMER_REG);
	datlen = readl(host->base + MCI_DATALENGTH_REG);
	datcon = readl(host->base + MCI_DATACTRL_REG);
	datcnt = readl(host->base + MCI_DATACNT_REG);
	
	stat = 0;	//readl(host->base + MCI_STATUS_REG);
	imask = readl(host->base + MCI_MASK_REG);
	dmamode = readl(host->base + MCI_DMACTRL_REG);
	cpumode = readl(host->base + MCI_FIFO_REG);

	PDEBUG("current prefix: %s (%d)\n", prefix, eflags);
	
	PDEBUG("clkcon:[%08x], cmdarg:[%08x], cmd:[%08x], cmdrsp:[%08x].\n",
		clkcon, cmdarg, cmd, cmdrsp);
	PDEBUG("rsp1:[%08x], rsp2:[%08x], rsp3:[%08x], rsp4:[%08x]\n",
		rsp1, rsp2, rsp3, rsp4);
	PDEBUG("dtimer:[%08x], datlen:[%08x], datcon:[%08x], datcnt:[%08x]\n",
		dtimer, datlen, datcon, datcnt);
	PDEBUG("stat:[%08x], imask:[%08x], dmamode:[%08x], cpumode:[%08x]\n",
		stat, imask, dmamode, cpumode);
}

static inline void dbg_dumpdata(struct akmci_host *host,
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

/**
 * the data transfer mode description.
*/
static char* xfer_mode_desc[] = {
		"unknown", 
		"l2dma",
		"l2pio",
		"inner pio",
	};

/**
 * the sd/mmc/sdio card detect mode description.
*/
static char* detect_mode_desc[] = {
		"plugin alway", 
		"GPIO detect",
		"AD detect",
	};


static void akmci_drv_lock(struct akmci_host *host)
{
	if(host->mci_mode == MCI_MODE_MMC_SD) {
		ak_drv_module_protect(DRV_MODULE_SDMMC);
	}
}
static void akmci_drv_unlock(struct akmci_host *host)
{
	if(host->mci_mode == MCI_MODE_MMC_SD) {
		ak_drv_module_unprotect(DRV_MODULE_SDMMC);
	}
}

static void akmci_init_sharepin(struct akmci_host *host)
{
	if(host->mci_mode == MCI_MODE_MMC_SD) {
		if(ak_drv_module_lock(DRV_MODULE_SDMMC) < 0)
			ak_group_config(ePIN_AS_MCI);
		ak_drv_module_unlock(DRV_MODULE_SDMMC);
	} else {
		if(ak_drv_module_lock(DRV_MODULE_SDIO) < 0)
			ak_group_config(ePIN_AS_SDIO);
		ak_drv_module_unlock(DRV_MODULE_SDIO);
	}
}


#define MCI_L2_ADDR(host)	\
	((host->mci_mode == MCI_MODE_MMC_SD) ? ADDR_MMC_SD : ADDR_SDIO)
		
/**
* akmci_xfer_mode - judgement the mci transfer mode.
* ret: 	AKMCI_XFER_L2DMA: use for l2 dma mode
* 		AKMCI_XFER_L2PIO: use for l2 fifo mode.
*		AKMCI_XFER_INNERPIO: use for inner fifo mode.
*/
static inline int akmci_xfer_mode(struct akmci_host *host)
{
	return host->xfer_mode;
}

static inline int enable_imask(struct akmci_host *host, u32 imask)
{
	u32 newmask;

	newmask = readl(host->base + MCI_MASK_REG);
	newmask |= imask;
	writel(newmask, host->base + MCI_MASK_REG);

	return newmask;
}

static inline int disable_imask(struct akmci_host *host, u32 imask)
{
	u32 newmask;

	newmask = readl(host->base + MCI_MASK_REG);
	newmask &= ~imask;
	writel(newmask, host->base + MCI_MASK_REG);

	return newmask;
}

static inline void clear_imask(struct akmci_host *host)
{
	u32 mask = readl(host->base + MCI_MASK_REG);

	/* preserve the SDIO IRQ mask state */
	mask &= MCI_SDIOINTMASK;
	writel(mask, host->base + MCI_MASK_REG);
}

static void akmci_reset(struct akmci_host *host)
{
	if(host->mci_mode == MCI_MODE_MMC_SD) {
		ak_soft_reset(AK_SRESET_MMCSD);	
	} else {
		ak_soft_reset(AK_SRESET_SDIO);
	}
}

/**
 * @brief transmitting data.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of data transmitted, including data buf pointer, data len .
 * @return void.
*/
static void akmci_l2xfer(struct akmci_host *host)
{
	int  sg_remain;
	u32 xferlen;
	u8 dir;
	u32 *tempbuf = NULL;
	dma_addr_t phyaddr = 0;

	if (host->data->flags & MMC_DATA_WRITE) {		
		dir = MEM2BUF;
	} else {
		dir = BUF2MEM;
	}	

    sg_remain = host->sg_ptr->length - host->sg_off;
    if (sg_remain <= 0)
    {        
        host->sg_ptr = sg_next(host->sg_ptr);        
        if (host->sg_ptr == NULL)
			return;
				
		host->sg_off = 0;
        sg_remain = host->sg_ptr->length - host->sg_off;
    } 
	
#ifdef CONFIG_MMC_BLOCK_BOUNCE
	xferlen = sg_remain;
#else
	xferlen = (sg_remain > host->data->blksz) ? host->data->blksz : sg_remain; 
#endif

	if ((akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) &&
		xferlen >= L2_DMA_ALIGN)
	{
		PDEBUG("akmci transfer data: DMA mode.\n");
		phyaddr = sg_dma_address(host->sg_ptr) + host->sg_off;
	    l2_combuf_dma(phyaddr, host->l2buf_id, xferlen, dir, AK_FALSE); 
	} else {
		PDEBUG("akmci transfer data: CPU mode.\n");
	    tempbuf = sg_virt(host->sg_ptr) + host->sg_off;	
	    l2_combuf_cpu((unsigned long)tempbuf, host->l2buf_id, xferlen, dir); 
	}
	
    host->sg_off += xferlen;
    host->data_xfered += xferlen;
	host->size -= xferlen;

	/* debug info if data transfer error */
	if(host->data_err_flag > 0) {
		printk("mci_xfer transfered: xferptr = 0x%p, xfer_offset=%d, xfer_bytes=%d\n",
			sg_virt(host->sg_ptr)+host->sg_off, host->sg_off, host->data_xfered);
	}
}


void akmci_init_sg(struct akmci_host *host, struct mmc_data *data)
{
	/*
	 * Ideally, we want the higher levels to pass us a scatter list.
	 */
	host->sg_len = data->sg_len;
	host->sg_ptr = data->sg;
	host->sg_off = 0;
}

int akmci_next_sg(struct akmci_host *host)
{
	host->sg_ptr++;
	host->sg_off = 0;
	return --host->sg_len;
}

/**
 * @brief stop data, close interrupt.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host get the base address of resgister.
 * @return void.
 */
static void akmci_stop_data(struct akmci_host *host)
{
	writel(0, host->base + MCI_DMACTRL_REG);
	writel(0, host->base + MCI_DATACTRL_REG);
	
	/* disable mci data irq */
	disable_imask(host, MCI_DATAIRQMASKS|MCI_FIFOFULLMASK|MCI_FIFOEMPTYMASK);
     
	if(akmci_xfer_mode(host) ==AKMCI_XFER_L2DMA) {
		if (host->data->flags & MMC_DATA_WRITE) {
			dma_sync_sg_for_cpu(mmc_dev(host->mmc), host->data->sg, host->data->sg_len, DMA_TO_DEVICE);
			dma_unmap_sg(mmc_dev(host->mmc), host->data->sg, host->data->sg_len, DMA_TO_DEVICE);
		} else {
			dma_sync_sg_for_cpu(mmc_dev(host->mmc), host->data->sg, host->data->sg_len, DMA_FROM_DEVICE);
			dma_unmap_sg(mmc_dev(host->mmc), host->data->sg, host->data->sg_len, DMA_FROM_DEVICE);
		}
	}

	host->sg_ptr = NULL;
	host->sg_len = 0;
	host->sg_off = 0;
	
	host->data = NULL; 
}

/**
 * @brief  finish a request,release resource.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of sd controller.
 * @param [in] *mrq information of request.
 * @return void.
 */
static void akmci_request_end(struct akmci_host *host, struct mmc_request *mrq)
{
    int not_retry = 0;

	writel(0, host->base + MCI_COMMAND_REG); 
	
	BUG_ON(host->data);
	host->mrq = NULL;
	host->cmd = NULL;

	if(host->data_err_flag > 0) {
		akmci_reset(host);

		writel(MCI_ENABLE|MCI_FAIL_TRIGGER, host->base + MCI_CLK_REG);
		writel(readl(host->base + MCI_CLK_REG)|host->clkreg, host->base + MCI_CLK_REG);
		mdelay(10);
	}	
	
	if(host->l2buf_id != BUF_NULL) {
		l2_free(MCI_L2_ADDR(host));
		host->l2buf_id = BUF_NULL;
	}
    
	if (mrq->data)
		mrq->data->bytes_xfered = host->data_xfered;
	
	/*
	 * Need to drop the host lock here; mmc_request_done may call
	 * back into the driver...
	 */
	spin_unlock(&host->lock);    

    not_retry = (!mrq->cmd->error) || ((mrq->cmd->error && (mrq->cmd->retries == 0)));
	
    mmc_request_done(host->mmc, mrq);
	PDEBUG("finalize the mci request.\n");

	/*if request fail,then mmc_request_done send request again, 
	* ak_mci_send_request not down nand_lock in interrupt,so not to up nand_lock.
	*/
	if (not_retry) {
		akmci_drv_unlock(host);
	}	

#ifdef CONFIG_CPU_FREQ
	 /*if request fail,then mmc_request_done send request again, ak_mci_send_request
	  *  not down freq_lock in interrupt,so not to unlock freq_lock.
	  */
	 if (not_retry) {					   
		 up(&host->freq_lock);
	 }	  
#endif

	spin_lock(&host->lock);
}

#define BOTH_DIR (MMC_DATA_WRITE | MMC_DATA_READ)

/**
 * @brief  config sd controller, start transmitting data.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of sd controller.
 * @param [in] *data information of data transmitted.
 * @return void.
 */
static void akmci_start_data(struct akmci_host *host, struct mmc_data *data)
{
	unsigned int datactrl, dmacon;
    
	PDEBUG("%s: blksz %04x blks %04x flags %08x\n",
	       __func__, data->blksz, data->blocks, data->flags);
	BUG_ON((data->flags & BOTH_DIR) == BOTH_DIR);

	host->data = data;
	host->size = data->blksz * data->blocks;
	host->data_xfered = 0;

	akmci_init_sg(host, data); 

	if(akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) {
		/* set dma addr */
		if (data->flags & MMC_DATA_WRITE)
			dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len, DMA_TO_DEVICE);
		else
			dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len, DMA_FROM_DEVICE);
	}
	
	writel(TRANS_DATA_TIMEOUT, host->base + MCI_DATATIMER_REG);
	writel(host->size, host->base + MCI_DATALENGTH_REG);

	if(akmci_xfer_mode(host) != AKMCI_XFER_INNERPIO) {
		/*dma mode register*/
		dmacon = MCI_DMA_BUFEN | MCI_DMA_SIZE(MCI_L2FIFO_SIZE/4);

		if(akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) {
			dmacon |= MCI_DMA_EN;
		}		
		writel(dmacon, host->base + MCI_DMACTRL_REG);	
	}

	/* enable mci data irq */
	enable_imask(host, MCI_DATAIRQMASKS);

	datactrl = MCI_DPSM_ENABLE;

	switch (host->bus_width) {
	case MMC_BUS_WIDTH_8:
		datactrl |= MCI_DPSM_BUSMODE(2);
		break;
	case MMC_BUS_WIDTH_4:
		datactrl |= MCI_DPSM_BUSMODE(1);
		break;
	case MMC_BUS_WIDTH_1:
	default:
		datactrl |= MCI_DPSM_BUSMODE(0);
		break;
	}

	if (data->flags & MMC_DATA_STREAM) {
		PDEBUG("STREAM Data\n");
		datactrl |= MCI_DPSM_STREAM;
	} else {
		PDEBUG("BLOCK Data: %u x %u\n", data->blksz, data->blocks);
		datactrl |= MCI_DPSM_BLOCKSIZE(data->blksz);
		datactrl &= ~MCI_DPSM_STREAM;
	}

	if (data->flags & MMC_DATA_READ) 
		datactrl |= MCI_DPSM_DIRECTION;
	else if (data->flags & MMC_DATA_WRITE) 
		datactrl &= ~MCI_DPSM_DIRECTION;

	/* configurate data controller register */
	writel(datactrl, host->base + MCI_DATACTRL_REG);
	
	PDEBUG("ENABLE DATA IRQ, datactrl: 0x%08x, timeout: 0x%08x, len: %u\n",
	       datactrl, readl(host->base + MCI_DATATIMER_REG), host->size);

	if((akmci_xfer_mode(host) != AKMCI_XFER_INNERPIO) &&
			data->flags & MMC_DATA_WRITE)
		akmci_l2xfer(host);
}

/**
 * @brief  config sd controller, start sending command.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of sd controller.
 * @param [in] *cmd information of cmd sended.
 * @return void.
 */
static void akmci_start_command(struct akmci_host *host, struct mmc_command *cmd)
{
	unsigned int ccon;

	PDEBUG("mci send cmd: op %i arg 0x%08x flags 0x%08x.%s data.\n", 
		cmd->opcode, cmd->arg, cmd->flags, cmd->data ? "contain":"no");

	writel(cmd->arg, host->base + MCI_ARGUMENT_REG);
	/* enable mci cmd irq */
	enable_imask(host, MCI_CMDIRQMASKS);


	ccon = MCI_CPSM_CMD(cmd->opcode) | MCI_CPSM_ENABLE;
	if (cmd->flags & MMC_RSP_PRESENT) {
		ccon |= MCI_CPSM_RESPONSE;
		if (cmd->flags & MMC_RSP_136)
			ccon |= MCI_CPSM_LONGRSP;
	}

	if (cmd->data)
		ccon |= MCI_CPSM_WITHDATA;

	host->cmd = cmd;

	/* configurate cmd controller register */
	writel(ccon, host->base + MCI_COMMAND_REG);
}


static void print_mci_data_err(struct mmc_data *data,
					unsigned int status, const char *err)
{
	if (data->flags & MMC_DATA_READ) {
		printk("akmci: data(read) status=%d %s\n", status, err);
	} else if (data->flags & MMC_DATA_WRITE) {
		printk("akmci: data(write) status=%d %s\n", status, err);
	}
}

/**
 * @brief  data handle in sdio interrupt.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of sd controller.
 * @param [in] *data information of data transmitting.
 * @return void.
 */
static void akmci_data_irq(struct akmci_host *host, struct mmc_data *data,
		  unsigned int status)
{
	if(status & MCI_DATABLOCKEND) {		
		if((akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) ||
				(akmci_xfer_mode(host) == AKMCI_XFER_L2PIO)) {
			//wait L2 dma finish, if need frac dma,start frac dma
			if((akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) && 
					(AK_FALSE == l2_combuf_wait_dma_finish(host->l2buf_id)))
				return;

			if (data->flags & MMC_DATA_WRITE)
				l2_clr_status(host->l2buf_id);

			if (host->size > 0) {
				akmci_l2xfer(host);		
			}
		}
	}

	if (status & (MCI_DATACRCFAIL|MCI_DATATIMEOUT)) {
		if (status & MCI_DATACRCFAIL) {
			data->error = -EILSEQ;
			print_mci_data_err(data, status, "illeage byte sequence");
		} else if (status & MCI_DATATIMEOUT) {
			data->error = -ETIMEDOUT;
			print_mci_data_err(data, status, "transfer timeout");
		}
		
		status |= MCI_DATAEND;
		host->data_err_flag = 1;
		
		printk("akmci need transfer: data->sg = 0x%p, data->sg_len=%d(sg),"
			" data->sg->length=%d, remain data = %d\n", 
			data->sg, data->sg_len, data->sg->length,
			__raw_readl(host->base + 0x30));
		
		/*
		 * We hit an error condition.  Ensure that any data
		 * partially written to a page is properly coherent.
		 */
		if (host->sg_len && data->flags & MMC_DATA_READ)
			flush_dcache_page(sg_page(host->sg_ptr));
	}
	
	if (status & MCI_DATAEND) {
		if ((data->retries > 0) && data->error) {
			printk("data->error = %d, data->retries = %d\n", 
				data->error, data->retries);
			
			/* support retry if error */
			akmci_stop_data(host);
			akmci_request_end(host, data->mrq);
		} else {		
		
			//wait L2 dma finish, if need frac dma,start frac dma
			if((akmci_xfer_mode(host) == AKMCI_XFER_L2DMA) && 
				(AK_FALSE == l2_combuf_wait_dma_finish(host->l2buf_id)))
				return;
		
			host->data_err_flag = 0;
			akmci_stop_data(host);

			if (!data->stop)
				akmci_request_end(host, data->mrq);
			else
				akmci_start_command(host, data->stop);
		}
	}
}

/**
 * @brief  cmd handle in sd interrupt.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *host information of sd controller.
 * @param [in] *cmd information of cmd sended.
  *@param [in] *status the status of sd controller.
 * @return void.
 */
static void akmci_cmd_irq(struct akmci_host *host, struct mmc_command *cmd,
		 unsigned int status)
{
	host->cmd = NULL;

	cmd->resp[0] = readl(host->base + MCI_RESPONSE0_REG);
	cmd->resp[1] = readl(host->base + MCI_RESPONSE1_REG);
	cmd->resp[2] = readl(host->base + MCI_RESPONSE2_REG);
	cmd->resp[3] = readl(host->base + MCI_RESPONSE3_REG);
	
    PDEBUG("resp[0]=0x%x, [1]=0x%x, resp[2]=0x%x, [3]=0x%x\n",
		cmd->resp[0],cmd->resp[1],cmd->resp[2],cmd->resp[3]);
	
	if (status & MCI_RESPTIMEOUT) {
		cmd->error = -ETIMEDOUT;
		PDEBUG("CMD: send timeout\n");
	} else if (status & MCI_RESPCRCFAIL && cmd->flags & MMC_RSP_CRC) {
		cmd->error = -EILSEQ;
		PDEBUG("CMD: illegal byte sequence\n");
	}

	/* disable mci cmd irq */
	disable_imask(host, MCI_CMDIRQMASKS);

	if (!cmd->data || cmd->error) {
		if (host->data)
			akmci_stop_data(host);
		akmci_request_end(host, cmd->mrq);
	} else if (!(cmd->data->flags & MMC_DATA_READ)) {
	 		/* transfer data from host to sd card */
	 	akmci_start_data(host, cmd->data);
	}
}

/*
 * Handle completion of command and data transfers.
 */
static irqreturn_t akmci_irq(int irq, void *dev_id)
{
	struct akmci_host *host = dev_id;
	u32 stat_mask;
	u32 status;
	int ret = 0;

	spin_lock(&host->lock);

	status = readl(host->base + MCI_STATUS_REG);
	if (status & MCI_SDIOINT) {
	    /*must disable sdio irq ,than read status to clear the sdio status,
         * else sdio irq will come again.
	    */
		mmc_signal_sdio_irq(host->mmc);
		status |= readl(host->base + MCI_STATUS_REG);
	}

	stat_mask = MCI_RESPCRCFAIL|MCI_RESPTIMEOUT|MCI_CMDSENT|MCI_RESPEND;
	if ((status & stat_mask) && host->cmd)
		akmci_cmd_irq(host, host->cmd, status);

	stat_mask = MCI_DATACRCFAIL|MCI_DATATIMEOUT|MCI_DATAEND|
				MCI_DATABLOCKEND|MCI_STARTBIT_ERR;

	if ((status & stat_mask) && host->data)
		akmci_data_irq(host, host->data, status);

	ret = 1;
	spin_unlock(&host->lock);

	return IRQ_RETVAL(ret);

}


static void akmci_send_request(struct mmc_host *mmc)
{
	struct akmci_host *host = mmc_priv(mmc); 
	struct mmc_request *mrq = host->mrq;	
	unsigned long flags;

#ifdef CONFIG_CPU_FREQ
	 /* need not to acquire the freq_lock in interrupt.	*/
	 if (!in_interrupt())
	 	down(&host->freq_lock);
#endif

	/* need not to acquire the nand_lock in interrupt. */
	if (!in_interrupt()) {
		akmci_drv_lock(host);
	}

	if(mrq->data || mrq->cmd->data) {
		host->l2buf_id = l2_alloc(MCI_L2_ADDR(host));	
		if (BUF_NULL == host->l2buf_id)	{
			printk("L2 buffer malloc fail!\n");
			BUG();
		}
	}

	spin_lock_irqsave(&host->lock, flags);	
	
	if (mrq->data && (mrq->data->flags & MMC_DATA_READ))
		akmci_start_data(host, mrq->data);	

	akmci_start_command(host, mrq->cmd);

	spin_unlock_irqrestore(&host->lock, flags);
}


/**
 * @brief  detect sdio card's level type .
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] data  getting the information of sd host.
 * @return void.
 */
static void akmci_detect_change(unsigned long data)
{
	struct akmci_host *host = (struct akmci_host *)data;
	
	PDEBUG("card detect change.\n");

	mmc_detect_change(host->mmc, 0);

	if (host->irq_cd_type == IRQ_TYPE_LEVEL_LOW) {
		host->irq_cd_type = IRQ_TYPE_LEVEL_HIGH;
	} else {
		host->irq_cd_type = IRQ_TYPE_LEVEL_LOW;
	}
	irq_set_irq_type(host->irq_cd, host->irq_cd_type);
	enable_irq(host->irq_cd);
}

static irqreturn_t akmci_card_detect_irq(int irq, void *dev)
{
	struct akmci_host *host = dev;

	disable_irq_nosync(irq);
	mod_timer(&host->detect_timer, jiffies + msecs_to_jiffies(400));

	return IRQ_HANDLED;
}


/**
 * @brief   detect the sdio card whether or not is in.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *mmc information of host ,getting the sdio detect gpio.
 * @return int.
 * @retal 1 sdio card is in ;0 sdio card is not in
 */
static int set_mci_plugin(struct notifier_block *nb,
	unsigned long val, void *data)
{
	struct akmci_host *host = container_of(nb, struct akmci_host, detect_nb);
	
	if(host->mci_mode == MCI_MODE_MMC_SD) {
		if (val == ADDETECT_MMC_PLUGIN)
			host->plugin_flag = 1;
		else if (val == ADDETECT_MMC_PLUGOUT)
			host->plugin_flag = 0;
	} else {
		if (val == ADDETECT_SDIO_PLUGIN)
			host->plugin_flag = 1;
		else if (val == ADDETECT_SDIO_PLUGOUT)
			host->plugin_flag = 0;
	}
	
	mmc_detect_change(host->mmc, 50);
	return 0;
}

static int akmci_enable(struct mmc_host *mmc)
{
	PDEBUG("akmci_enable:host is claimed.\n");	
	//akmci_drv_lock(host);
	return 0;
}

static int akmci_disable(struct mmc_host *mmc)
{
	PDEBUG("akmci_disable:host is released.\n");
	//akmci_drv_unlock(host);
	return 0;
}

static int akmci_card_present(struct mmc_host *mmc)
{
	struct akmci_host *host = mmc_priv(mmc);

	if(host->detect_mode == AKMCI_DETECT_MODE_AD) 
	{
		return host->plugin_flag;
	} 
	else if(host->detect_mode == AKMCI_DETECT_MODE_GPIO) 
	{
		if (host->gpio_cd == -ENOSYS)
			return -ENOSYS;	

		if(host->gpio_cd >= 0)
			return (ak_gpio_getpin(host->gpio_cd) == 0);
		else
			return 1;
	} 
	else 
	{
		return 1; //plugin alway.
	}
}

static void akmci_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct akmci_host *host = mmc_priv(mmc);

	host->mrq = mrq;
	host->data_err_flag = 0;
	
	PDEBUG("start the mci request.\n");

	if(akmci_card_present(mmc) == 0) {
		printk("%s: no medium present.\n", __func__);
		host->mrq->cmd->error = -ENOMEDIUM;
		mmc_request_done(mmc, mrq);
	} else {
		akmci_send_request(mmc);
	}
}

static void akmci_set_clk(struct akmci_host *host, struct mmc_ios *ios)
{
	u32 clk, div;
	u32 clk_div_h, clk_div_l;

	if (ios->clock == 0) {
		clk = readl(host->base + MCI_CLK_REG);
		clk &= ~MCI_CLK_ENABLE;
		writel(clk, host->base + MCI_CLK_REG);	
		
		host->bus_clock = 0;
	} else {
		clk = readl(host->base + MCI_CLK_REG);
		clk |= MCI_CLK_ENABLE;//|MCI_CLK_PWRSAVE;
		clk &= ~0xffff; /* clear clk div */
		
		div = host->asic_clk/ios->clock;

        if (host->asic_clk % ios->clock)
            div += 1;
		
        div -= 2;
        clk_div_h = div/2;
        clk_div_l = div - clk_div_h;
		
		clk |= MMC_CLK_DIVL(clk_div_l) | MMC_CLK_DIVH(clk_div_h);
		writel(clk, host->base + MCI_CLK_REG);	

		host->bus_clock = host->asic_clk / ((clk_div_h+1)*(clk_div_l + 1));	
		
		PDEBUG("mmc clock is %lu Mhz. asic_clock is %ld MHz(div:l=%d, h=%d).\n",
			ios->clock/MHz, host->asic_clk/MHz, clk_div_l, clk_div_h); 
	}
	
	host->clkreg = clk;
}


static void akmci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct akmci_host *host = mmc_priv(mmc);

	switch(ios->power_mode) {
		case MMC_POWER_ON:
			PDEBUG("mci power on.\n");
			break;
		case MMC_POWER_UP:
			PDEBUG("mci power up.\n");
			break;
		case MMC_POWER_OFF:
			PDEBUG("mci power off.\n");
			break;
		default:
			break;
	}

	host->bus_mode = ios->bus_mode;
	host->bus_width = ios->bus_width;

	if(ios->clock != host->bus_clock) {
		akmci_set_clk(host, ios);
	}
}

/**
 * @brief   detect the sdio card writing protection.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *mmc information of host ,getting the sdio detect gpio.
 * @return int.
 * @retal 1 sdio card writing protected ;0 sdio card writing is not protected
 */
static int akmci_get_ro(struct mmc_host *mmc)
{
	struct akmci_host *host = mmc_priv(mmc);

	if (host->gpio_wp == -ENOSYS)
		return -ENOSYS;
	
	return (ak_gpio_getpin(host->gpio_wp) == 0);
}

/**
 * @brief  enable or disable sdio interrupt, mmc host not use..
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *mmc information of sd controller.
 * @param [in] enable  1: enable; 0: disable.
 * @return void.
 */
static void akmci_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	unsigned reg1,reg2;
	unsigned long flags;	
	struct akmci_host *host = mmc_priv(mmc);

	BUG_ON(host->mci_mode == MCI_MODE_MMC_SD);
	
	PDEBUG("%s the sdio interrupt.\n", enable ? "enable" : "disable");
	spin_lock_irqsave(&host->lock, flags); 
	
	reg1 = readl(host->base + MCI_MASK_REG);
	reg2 = readl(host->base + SDIO_INTRCTR_REG);
	
	if (enable) {
		reg1 |= SDIO_INTR_ENABLE; 
		reg2 |= SDIO_INTR_CTR_ENABLE;
	} else {
		reg1 &= ~SDIO_INTR_ENABLE;
		reg2 &= ~SDIO_INTR_CTR_ENABLE;
	}
	
	writel(reg2, host->base + SDIO_INTRCTR_REG);	
	writel(reg1, host->base + MCI_MASK_REG);
	spin_unlock_irqrestore(&host->lock, flags); 
}

/**
 * register the function of sd/sdio driver.
 * 
 */
static struct mmc_host_ops akmci_ops = {
	.enable  = akmci_enable,
	.disable = akmci_disable,
	.request = akmci_request,
	.set_ios = akmci_set_ios,
	.get_ro  = akmci_get_ro,
	.get_cd  = akmci_card_present,
	.enable_sdio_irq = akmci_enable_sdio_irq,
};


static int akmci_init_mmc_host(struct akmci_host *host)
{
	struct mmc_host *mmc = host->mmc;
	struct ak_mci_platform_data *plat = host->plat;
	
	mmc->ops = &akmci_ops;
	
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->caps = MMC_CAP_4_BIT_DATA; 

	if(host->mci_mode == MCI_MODE_SDIO)
		mmc->caps |= MMC_CAP_SDIO_IRQ;

	if(plat->cap_highspeed)
		mmc->caps |= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED;

//	mmc->caps |= MMC_CAP_NEEDS_POLL;
	mmc->f_min = host->asic_clk / (255+1 + 255+1);
	mmc->f_max = host->asic_clk / (0+1 + 0+1);
	mmc->f_max = (mmc->f_max < plat->max_speed_hz) ? 
			mmc->f_max : plat->max_speed_hz;

#ifdef CONFIG_MMC_BLOCK_BOUNCE
	/* use block bounce buffer. */
	mmc->max_segs = 1;
#else
	/* We can do SGIO */
	mmc->max_segs = MAX_MCI_REQ_SIZE/MAX_MCI_BLOCK_SIZE;
#endif

	/*
	 * Since we only have a 16-bit data length register, we must
	 * ensure that we don't exceed 2^16-1 bytes in a single request.
	 */
	mmc->max_req_size = MAX_MCI_REQ_SIZE;

	/*
	 * Set the maximum segment size.  Since we aren't doing DMA
	 * (yet) we are only limited by the data length register.
	 */
	mmc->max_seg_size = mmc->max_req_size;

	mmc->max_blk_size = MAX_MCI_BLOCK_SIZE;

	/*No limit on the number of blocks transferred.*/
	mmc->max_blk_count = mmc->max_req_size / MAX_MCI_BLOCK_SIZE;
	return 0;
}

static void akmci_init_host_cfg(struct akmci_host *host)
{
	akmci_init_sharepin(host);
	/*enable the mci clock*/
	writel(MCI_ENABLE|MCI_FAIL_TRIGGER, host->base + MCI_CLK_REG);
	
	clear_imask(host);
}


#if defined(CONFIG_CPU_FREQ)

static int akmci_cpufreq_transition(struct notifier_block *nb,
				     unsigned long val, void *data)
{
	struct akmci_host *host;
	struct mmc_host *mmc;
	unsigned long newclk;
	unsigned long flags;
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;
	host = container_of(nb, struct ak_mci_host, freq_transition);
	
	PDEBUG("%s(): in_interrupt()=%ld\n", __func__, in_interrupt());
	PDEBUG("ak_get_asic_clk = %ld\n",ak_get_asic_clk());	
	PDEBUG("freqs->new_cpufreq.asic_clk = %d\n",
		  freqs->new_cpufreq.asic_clk);	
		  
	mmc = host->mmc;
	newclk = freqs->new_cpufreq.asic_clk;
	if ((val == CPUFREQ_PRECHANGE && newclk > host->asic_clock) 
		|| (val == CPUFREQ_POSTCHANGE && newclk < host->asic_clock)) 
	{

		if (mmc->ios.power_mode != MMC_POWER_OFF &&
			mmc->ios.clock != 0)
		{		
			PDEBUG("%s(): preempt_count()=%d\n", __func__, preempt_count());
				
			down(&host->freq_lock);
			
			spin_lock_irqsave(&mmc->lock, flags);
		
			host->asic_clock = newclk;
			PDEBUG("MCI_CLK_REG1 = %d\n",readl(host->base + MCI_CLK_REG));	
			aksdio_set_clk(host, &mmc->ios);
			PDEBUG("MCI_CLK_REG2 = %d\n",readl(host->base + MCI_CLK_REG));	
			
			spin_unlock_irqrestore(&mmc->lock, flags);

			up(&host->freq_lock);
		}
	}

	return NOTIFY_DONE;
}


static inline int akmci_cpufreq_register(struct akmci_host *host)
{
	// use for requst and cpufreq
	sema_init(&host->freq_lock, 1);
	
	host->freq_transition.notifier_call = akmci_cpufreq_transition;

	return cpufreq_register_notifier(&host->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void akmci_cpufreq_deregister(struct akmci_host *host)
{
	cpufreq_unregister_notifier(&host->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int akmci_cpufreq_register(struct akmci_host *host)
{
	return 0;
}

static inline void akmci_cpufreq_deregister(struct akmci_host *host)
{
}
#endif


/**
 * @brief   sdio driver probe and init.
 * 
 * @author Hanyang
 * @date 2011-05-10
 * @param [in] *pdev information of platform device ,getting the sd driver resource .
 * @return int.
 * @retval -EINVAL no platform data , fail;
 * @retval -EBUSY  requset mem  fail;
 * @retval -ENOMEM  alloc mem fail;
 */
static int __devinit akmci_probe(struct platform_device *pdev)
{
	struct akmci_host *host;
	struct mmc_host *mmc;
	int ret;
	struct ak_mci_platform_data *plat;

	plat = pdev->dev.platform_data;
	if(!plat) {
		printk("not found mci platform data.");
		ret = -EINVAL;
		goto probe_out;
	}

	mmc = mmc_alloc_host(sizeof(struct akmci_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		goto probe_out;
	}

	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->pdev = pdev;

	spin_lock_init(&host->lock);

	host->plat = plat;
	host->mci_mode = plat->mci_mode;
	host->data_err_flag = 0;
	host->l2buf_id = BUF_NULL;

	host->gpio_wp = -ENOSYS;
	host->gpio_cd = -ENOSYS;
	host->detect_mode = plat->detect_mode;
	host->xfer_mode   = plat->xfer_mode;

	akmci_reset(host);

	host->mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!host->mem) {
		ret = -ENOENT;
		goto probe_free_host;
	}
	
	host->mem = request_mem_region(host->mem->start,
				       resource_size(host->mem), pdev->name);

	if (!host->mem) {
		dev_err(&pdev->dev, "failed to request io memory region.\n");
		ret = -ENOENT;
		goto probe_free_host;
	}

	host->base = ioremap(host->mem->start, resource_size(host->mem));
	if (!host->base) {
		dev_err(&pdev->dev, "failed to ioremap() io memory region.\n");
		ret = -EINVAL;
		goto probe_free_mem_region;
	}

	host->irq_mci = platform_get_irq(pdev, 0);
	if(host->irq_mci == 0) {
		dev_err(&pdev->dev, "failed to get interrupt resouce.\n");
		ret = -EINVAL;
		goto probe_iounmap;
	}

	if (request_irq(host->irq_mci, akmci_irq, IRQF_DISABLED, pdev->name, host)) {
		dev_err(&pdev->dev, "failed to request mci interrupt.\n");
		ret = -ENOENT;
		goto probe_iounmap;
	}

	/* We get spurious interrupts even when we have set the IMSK
	 * register to ignore everything, so use disable_irq() to make
	 * ensure we don't lock the system with un-serviceable requests. */
	//disable_irq(host->irq_mci);

	host->clk = clk_get(&pdev->dev, (host->mci_mode == MCI_MODE_MMC_SD)? "mci" : "sdio");
	if (IS_ERR(host->clk)) {
		dev_err(&pdev->dev, "failed to find clock source.\n");
		ret = PTR_ERR(host->clk);
		host->clk = NULL;
		goto probe_free_irq;
	}

	ret = clk_enable(host->clk);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable clock source.\n");
		goto clk_free;
	}

	host->asic_clk = clk_get_rate(host->clk);

	ret = akmci_init_mmc_host(host);
	if(ret) {
		dev_err(&pdev->dev, "failed to init mmc host.\n");
		goto clk_disable;
	}

	akmci_init_host_cfg(host);

	if(host->detect_mode == AKMCI_DETECT_MODE_GPIO) {
		if(plat->gpio_cd.pin >= 0) {
	    	host->gpio_cd = plat->gpio_cd.pin;
			plat->gpio_init(&plat->gpio_cd);
			
	        setup_timer(&host->detect_timer, akmci_detect_change, 
						(unsigned long)host);
	                    
	        host->irq_cd = ak_gpio_to_irq(host->gpio_cd);    
	        ret = request_irq(host->irq_cd, akmci_card_detect_irq, 
					IRQF_DISABLED|IRQF_TRIGGER_LOW, pdev->name, host);
			
	        dev_info(&pdev->dev, "pdev->name:%s request gpio irq ret = %d, irq=%d\n",
							pdev->name, ret, host->irq_cd);
	        if (ret)
	            goto clk_disable;

	        host->irq_cd_type = plat->irq_cd_type; 
		}
	}
	else if(host->detect_mode == AKMCI_DETECT_MODE_AD) {
		memset(&host->detect_nb, 0, sizeof(host->detect_nb));
		host->detect_nb.notifier_call = set_mci_plugin;
		addetect_register_client(&host->detect_nb);
	}

	if (plat->gpio_wp.pin >= 0) {
		host->gpio_wp = plat->gpio_wp.pin;
		plat->gpio_init(&plat->gpio_wp);
	}
	
	ret = akmci_cpufreq_register(host);
	if (ret) {
		goto detect_irq_free;
	}
	
	platform_set_drvdata(pdev, mmc);

	ret = mmc_add_host(mmc);
	if (ret) {
		goto probe_cpufreq_free;
	}

	dev_info(&pdev->dev, "Mci Interface driver.%s."
		" using %s, %s IRQ. detect mode:%s.\n", 
		mmc_hostname(mmc), xfer_mode_desc[akmci_xfer_mode(host)],
		 mmc->caps & MMC_CAP_SDIO_IRQ ? "hw" : "sw", 
		 detect_mode_desc[host->detect_mode]);

	return 0;
	
probe_cpufreq_free:
	
detect_irq_free:
	free_irq(host->irq_cd, host);

clk_disable:
	clk_disable(host->clk);

clk_free:
	clk_put(host->clk);

probe_free_irq:	
	free_irq(host->irq_mci, host);

probe_iounmap:
	iounmap(host->base);

probe_free_mem_region:
	release_mem_region(host->mem->start, resource_size(host->mem));

probe_free_host:
	mmc_free_host(host->mmc);

probe_out:
	return ret;
}

static int __devexit akmci_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct akmci_host *host;
	
	mmc = platform_get_drvdata(pdev);
	host = mmc_priv(mmc);
	
	mmc_remove_host(mmc);
	
	akmci_cpufreq_deregister(host);
	
	if(host->detect_mode == AKMCI_DETECT_MODE_AD)
		addetect_unregister_client(&host->detect_nb);

	clk_disable(host->clk);
	clk_put(host->clk);
	
	free_irq(host->irq_cd, host);
	free_irq(host->irq_mci, host);
	
	iounmap(host->base);
	release_mem_region(host->mem->start, resource_size(host->mem));

	mmc_free_host(host->mmc);

	return 0;
}

#ifdef CONFIG_PM
static int akmci_suspend(struct device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(to_platform_device(dev));

	return mmc_suspend_host(mmc);
}

static int akmci_resume(struct device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(to_platform_device(dev));

	return mmc_resume_host(mmc);
}

static struct dev_pm_ops akmci_pm = {
	.suspend = akmci_suspend,
	.resume = akmci_resume
};

#define akmci_pm_ops  &akmci_pm
#else
#define akmci_pm_ops  NULL
#endif

struct platform_device_id ak_mci_ids[] ={
	{.name = "ak_mci", .driver_data = MCI_MODE_MMC_SD,},
	{.name = "ak_sdio",	.driver_data = MCI_MODE_SDIO,},
};

static struct platform_driver akmci_driver = {
	.probe = akmci_probe,
	.remove = __devexit_p(akmci_remove),
	.id_table	= ak_mci_ids,
	.driver 	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.pm 	= akmci_pm_ops,
	},

};

static int __init akmci_init(void)
{
	printk("AK MCI Driver (c) 2010 ANYKA\n");
	return platform_driver_register(&akmci_driver);
}

static void __exit akmci_exit(void)
{
	return platform_driver_unregister(&akmci_driver);
}


module_init(akmci_init);
module_exit(akmci_exit);

MODULE_DESCRIPTION("Anyka MCI Interface driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anyka");

