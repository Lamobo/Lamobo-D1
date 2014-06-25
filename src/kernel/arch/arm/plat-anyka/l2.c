/*
 * linux/arch/arm/plat-anyka/l2.c
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
//#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/stddef.h>
#include <linux/irq.h>
#include <linux/sched.h>
 
#include <asm/dma.h>
#include <asm/sizes.h>

#include <plat/l2.h>
#include <mach/clock.h>

static l2_buffer_info_t l2_buffer_info[L2_COMMON_BUFFER_NUM];
static l2_dma_info_t l2_dma_info[L2_COMMON_BUFFER_NUM + L2_UART_BUFFER_NUM];
static bool l2_frac_started = false;	/* L2 fraction DMA start flag */

static l2_device_info_t l2_device_info[] = {
	{ ADDR_USB_EP2,		BUF_NULL },
	{ ADDR_USB_EP3,		BUF_NULL },
	{ ADDR_USB_EP4,		BUF_NULL },
	{ ADDR_RESERVED,	BUF_NULL },
	{ ADDR_MMC_SD,		BUF_NULL },
	{ ADDR_SDIO,		BUF_NULL },
	{ ADDR_RESERVED,	BUF_NULL },
	{ ADDR_SPI1_RX,		BUF_NULL },
	{ ADDR_SPI1_TX,		BUF_NULL },
	{ ADDR_DAC,			BUF_NULL },
	{ ADDR_SPI2_RX,		BUF_NULL },
	{ ADDR_SPI2_TX,		BUF_NULL },
	{ ADDR_GPS,			BUF_NULL },
	{ ADDR_PCM_TX,		BUF_NULL },
	{ ADDR_ADC,			BUF_NULL },
	{ ADDR_USB_EP5, 	BUF_NULL },
	{ ADDR_RESERVED, 	BUF_NULL },
};

static int l2_wait = 0;
static wait_queue_head_t l2_wq;

extern void l2cache_invalidate(void);

static void l2_combuf_ctrl(u8 id, bool enable);
static void l2_select_combuf(l2_device_t device, u8 id);
static void l2_assert_combuf_id(u8 id);
static void l2_assert_buf_id(u8 id);
static void l2_clear_dma(u8 id);
static void l2_frac_dma(unsigned long ram_addr, u8 id, u8 frac_offset,
	unsigned int bytes, l2_dma_transfer_direction_t direction, bool intr_enable);
static u32 l2_get_addr(u8 id);
static bool l2_get_dma_param(unsigned int bytes, unsigned int *low, unsigned int *high);
static void l2_dma(unsigned long ram_addr, u8 id, unsigned int bytes,
		l2_dma_transfer_direction_t direction, bool intr_enable);
static bool l2_wait_dma_finish(u8 id);
static void l2_cpu(unsigned long ram_addr, u8 id,
	unsigned long buf_offset, unsigned int bytes, l2_dma_transfer_direction_t direction);
static irqreturn_t l2_interrupt_handler(int irq, void *dev_id);

/**
 * l2_assert_buf_id - Assert a L2 buffer ID is valid
 *  @id:		L2 buffer ID
 *
 *  NOTE: Assert only L2 common buffer and UART buffer, USB buffer is not checked.
 *            Since this function is called internally by other L2 API, invalid id will cause
 *            linux kernel to oops for bug tracking.
 */
static void l2_assert_buf_id(u8 id)
{
	if (id >= L2_COMMON_BUFFER_NUM + L2_UART_BUFFER_NUM)
		BUG();
}

/**
 * l2_assert_combuf_id - Assert a L2 common buffer ID is valid
 *  @id:		L2 buffer ID
 *
 *  NOTE: Assert only L2 common buffer, UART & USB buffer is not checked.
 *            Since this function is called internally by other L2 API, invalid id will cause
 *            linux kernel to oops for bug tracking.
 */
static void l2_assert_combuf_id(u8 id)
{
	if (id >= L2_COMMON_BUFFER_NUM)
		BUG();
}

/**
 * l2_combuf_ctrl - L2 buffer enable/disable
 *  @id:		L2 buffer ID
 *  @enable:	true to enable L2 buffer, false to disable L2 buffer 
 */
static void l2_combuf_ctrl(u8 id, bool enable)
{
	unsigned long regval;
	unsigned long flags;

	l2_assert_buf_id(id);

	local_irq_save(flags);
	
	regval = rL2_CONBUF0_7;
	if (enable) {
		/* Enable L2 buffer & L2 Buffer DMA */
		regval |= (1 << (id + L2_COMMON_BUF_CFG_BUF_DMA_VLD_START)) |
			(1 << (id + L2_COMMON_BUF_CFG_BUF_VLD_START));
	} else {
		/* Disable L2 buffer & L2 Buffer DMA */
		regval &= ~((1 << (id + L2_COMMON_BUF_CFG_BUF_DMA_VLD_START)) |
			(1 << (id + L2_COMMON_BUF_CFG_BUF_VLD_START)));
	}
	rL2_CONBUF0_7 = regval;

	local_irq_restore(flags);

}

/**
 * l2_select_combuf - Select a L2 buffer for given device
 *  @device:	Device which need to assign a L2 buffer
 *  @id:		L2 buffer ID
 */
static void l2_select_combuf(l2_device_t device, u8 id)
{
	unsigned long regval;
	unsigned long bits_offset;
	
	l2_assert_combuf_id(id);

	if ((u8)device < 10) {
		/*
		 * USB Bulkout ~ DAC (Device 0 ~ 9) is controlled by Buffer Assignment Register 1
		 */
		regval = rL2_BUFASSIGN1;
		bits_offset = (u8)device * 3;
		regval &= ~(0x7 << bits_offset);
		regval |= ((id & 0x7) << bits_offset);
		rL2_BUFASSIGN1 = regval;
	} else {
		/*
		 * SPI2 Rx ~ ADC (Device 10 ~ 14) is controlled by Buffer Assignment Register 2
		 */
		regval = rL2_BUFASSIGN2;
		bits_offset = ((u8)device - 10) * 3;
		regval &= ~(0x7 << bits_offset);
		regval |= ((id & 0x7) << bits_offset);
		rL2_BUFASSIGN2 = regval;
	}

}


/**
 * l2_deselect_combuf - deselect a L2 buffer for given device
 *  @device:	Device which need to assign a L2 buffer
 *  @id:		L2 buffer ID
 *
 * note: buffer 0 is reserved for softuse, no hardware is assigned
 *          so when free a l2 buffer, select to this device use buffer 0 as deselect
 */
static void l2_deselect_combuf(l2_device_t device, u8 id)
{
	unsigned long regval;
	unsigned long bits_offset;
	
	l2_assert_combuf_id(id);

	if ((u8)device < 10) {
		/*
		 * USB Bulkout ~ DAC (Device 0 ~ 9) is controlled by Buffer Assignment Register 1
		 */
		regval = rL2_BUFASSIGN1;
		bits_offset = (u8)device * 3;
		regval &= ~(0x7 << bits_offset);
		rL2_BUFASSIGN1 = regval;
	} else {
		/*
		 * SPI2 Rx ~ ADC (Device 10 ~ 14) is controlled by Buffer Assignment Register 2
		 */
		regval = rL2_BUFASSIGN2;
		bits_offset = ((u8)device - 10) * 3;
		regval &= ~(0x7 << bits_offset);
		rL2_BUFASSIGN2 = regval;
	}

}

/**
 * l2_clear_dma - Clear L2 buffer DMA status
 *  @id:		L2 buffer ID which need to clear DMA status
 */
static void l2_clear_dma(u8 id)
{
	bool dmapending;
	u8 status;
	
	dmapending = rL2_DMAREQ & (1 << (id + L2_DMA_REQ_BUF_START));
	status = l2_get_status(id);

	if (status == 0) {
		return ;	/* NO DMA request, so do nothing */
	}

	if(l2_dma_info[id].direction == BUF2MEM) {
		printk("l2r:[%d]..", id); 
		while (dmapending) {
			l2_set_status(id, 8);
			dmapending = rL2_DMAREQ & (1 << (id + L2_DMA_REQ_BUF_START));
		}
		printk("done\n");
	} else {
		/*
		 * Wait until DMA request of this L2 buffer is finished.
		 */
		printk("l2t:[%d]..", id);
		while (dmapending) {
			l2_clr_status(id);
			dmapending = rL2_DMAREQ & (1 << (id + L2_DMA_REQ_BUF_START));
		}
		printk("done\n");
	}
}

/**
 * l2_frac_dma - Start data tranferring between memory and l2 common buffer in fraction DMA mode
 *  @ram_addr:		External RAM address(Physical)
 *  @id:		L2 buffer ID involved in DMA transfer
 *  @frac_offset:	The region offset between buffer start address and transfer start address
 *  @bytes:		Data transfer size
 *  @direction:		Data transfer direction between L2 memory and external RAM 
 *  @intr_enable:	Open interrupt for this L2 buffer or not
 *
 *  NOTE: Data transfer size should be 1~64Bytes, frac_offset should be 0~7 (*64Bytes)
 */
static void l2_frac_dma(unsigned long ram_addr, u8 id, u8 frac_offset,
	unsigned int bytes, l2_dma_transfer_direction_t direction,	bool intr_enable)
{
	u32 bufaddr;
	u32 highaddr;
	unsigned long regval;
	unsigned long flags;

#if 0
	printk("%s(): ram_addr=0x%08X, l2 buffer id=%d, frac_offset=%d, bytes=%d, direction=%s, intr_enable=%d.\n",
		__func__, (unsigned int)ram_addr, id, frac_offset, bytes, (direction == BUF2MEM)?"BUF2MEM":"MEM2BUF", intr_enable);
#endif

	if (bytes == 0) {
		printk("l2: no need to start fraction dma transfer: bytes=0.\n");
		return ;
	}

	local_irq_save(flags);

	/*
	 * Set fraction external RAM address.
	 */
	highaddr = (ram_addr << 2) & 0xC0000000;
	 
	regval = rL2_FRACDMAADDR;
	regval &= ~(L2_FRAC_DMA_LOW_ADDR_MASK | (3<<30)); //modified by anyka chenyingyu
	regval |= (ram_addr & L2_FRAC_DMA_LOW_ADDR_MASK) | highaddr;
	rL2_FRACDMAADDR = regval;

	/* Set fraction DMA address */
	bufaddr = (id < L2_COMMON_BUFFER_NUM) ? ((id & 0x7) << 3) | (frac_offset & 0x7) :
		(0x40 + ((id - L2_COMMON_BUFFER_NUM) << 1)) | (frac_offset & 0x1);

	/* Clear other fraction DMA request and info */
	regval = rL2_DMAREQ;
	regval &= ~(L2_DMA_REQ_FRAC_DMA_LEN_MASK | L2_DMA_REQ_FRAC_DMA_L2_ADDR_MASK |
		L2_DMA_REQ_FRAC_DMA_REQ | L2_DMA_REQ_BUF_REQ_MASK);

	switch (direction) {
	case MEM2BUF:
		if (bytes & 0x1)
			bytes = bytes + 1;	/* Round to even number when read data from external ram */
		regval |= L2_DMA_REQ_FRAC_DMA_REQ | L2_DMA_REQ_FRAC_DMA_DIR_WR |
			(bufaddr << L2_DMA_REQ_FRAC_DMA_L2_ADDR_START) |
			((bytes - 1) << L2_DMA_REQ_FRAC_DMA_LEN_START);
		rL2_DMAREQ = regval;
		break;
	case BUF2MEM:
		regval &= ~(L2_DMA_REQ_FRAC_DMA_DIR_WR);
		regval |= L2_DMA_REQ_FRAC_DMA_REQ |
			(bufaddr << L2_DMA_REQ_FRAC_DMA_L2_ADDR_START) |
			((bytes - 1) << L2_DMA_REQ_FRAC_DMA_LEN_START);
		rL2_DMAREQ = regval;
		break;
	default:
		BUG();
	}

	if (intr_enable) {
		regval = rL2_BUFINTEN;
		regval |= L2_DMA_INTR_ENABLE_FRAC_INTR_EN;
		rL2_BUFINTEN = regval;
	}

	local_irq_restore(flags);
}

/**
 * l2_get_addr - Get L2 memory start address for given L2 buffer
 *  @id:		L2 buffer ID
 *  Return L2 memory start address(Logical/Virtual) (NOT physical address)
 */
static u32 l2_get_addr(u8 id)
{
	u32 bufaddr = 0;

	if (id < L2_UART_BUFFER_INDEX) {	/* L2 common buffer */
		bufaddr = (u32)AK_VA_L2MEM + L2_COMMON_BUFFER_OFFSET +
			id * L2_COMMON_BUFFER_LEN;
	} else if (id < L2_USB_HOST_BUFFER_INDEX) {	/* UART L2 buffer */
		bufaddr = (u32)AK_VA_L2MEM + L2_UART_BUFFER_OFFSET + 
			(id - L2_COMMON_BUFFER_NUM) * L2_UART_BUFFER_LEN;
	} else {
		printk("l2: invalid buffer id %d.\n", (int)id);
	}

	return bufaddr;
}

/**
 * l2_get_dma_param - Calculate l2 buffer big loop/small loop counter value
 *  @bytes:		L2 buffer ID
 *  @low:			CNT_cfg (bit[7:0] of DMA Operation Times Configuration Register)
 *  @high:		CNT_cfg_H (bit[23:16] of DMA Operation Times Configuration Register)
 *  Return true when correct counter value (high/low) is found, else return false.
 *
 *  NOTE: Use a simplified calculation method for L2 buffer 0~7 and 8~15 for bytes > 8KB
 */
static bool l2_get_dma_param(unsigned int bytes, unsigned int *low, unsigned int *high)
{
	unsigned int factor;
	unsigned int dma_times = bytes / DMA_ONE_SHOT_LEN;

	if (bytes <= 8 * 1024) {
		*low = dma_times;
		*high = 0;

		return true;
	} else if (dma_times & 0x7) {
		printk("l2: Invalid L2 DMA buffer size(%u).\n", bytes);
		return false;
	}

	factor = 16 * 8;

	while (factor > 0) {
		if ((dma_times % factor) == 0) {
			*low = factor;
			*high = dma_times / factor - 1;

			return (*high < 0xFF) ? true : false;
		}

		factor -= 8;
	}
	
	return false;
}


/**
 * l2_dma - Start data tranferring between memory and l2 buffer in DMA mode
 *  @ram_addr:		External RAM address(Physical)
 *  @id:		L2 buffer ID involved in DMA transfer
 *  @bytes:		Data transfer size
 *  @direction:		Data transfer direction between L2 memory and external RAM 
 *  @intr_enable:	Open interrupt for this L2 buffer or not
 */
static void l2_dma(unsigned long ram_addr, u8 id, unsigned int bytes,
	l2_dma_transfer_direction_t direction, bool intr_enable)
{
	unsigned long regid;
	unsigned long regval;
	unsigned long flags;
	unsigned int cnt_low;
	unsigned int cnt_high;

#if 0
	printk("%s(): ram_addr=0x%0X, id=%d, bytes=%d, direction=%d, intr_enable=%d.\n",
		__func__, (unsigned int)ram_addr, id, bytes, direction, intr_enable);
#endif
	if (bytes == 0) {
		printk("l2: no need to start dma transfer: bytes=0.\n");
		return ;
	}

	if (!l2_get_dma_param(bytes, &cnt_low, &cnt_high)) {
		printk("l2: L2 DMA buffer size error: bytes=%d.\n", bytes);
		return ;
	}
	
	if (l2_dma_info[id].dma_start || l2_dma_info[id].dma_frac_start) {
		printk("l2: unable to start dma, dma NOT finished, buf id=%d.\n", (int)id);
		return ;
	}

	l2_dma_info[id].dma_op_times = bytes / DMA_ONE_SHOT_LEN;
	l2_dma_info[id].dma_frac_data_len = bytes % DMA_ONE_SHOT_LEN;
	l2_dma_info[id].dma_addr = (void *)ram_addr;
	l2_dma_info[id].direction = direction;
	l2_dma_info[id].intr_enable = intr_enable;
	l2_dma_info[id].need_frac = false;

	if (l2_dma_info[id].dma_frac_data_len > 0) {
		l2_dma_info[id].need_frac = true;
		l2_dma_info[id].dma_frac_addr = (void *)(u8 *)l2_dma_info[id].dma_addr +
			l2_dma_info[id].dma_op_times* DMA_ONE_SHOT_LEN;
		l2_dma_info[id].dma_frac_offset = l2_dma_info[id].dma_op_times;
	}

	if (l2_dma_info[id].dma_op_times== 0) {
		/*
		 * If DMA transfer size < 64, we start fraction DMA immediately.
		 */
		 
		l2_dma_info[id].dma_start = false;
		l2_dma_info[id].dma_frac_start = true;

		l2_frac_dma((unsigned long)l2_dma_info[id].dma_frac_addr, id,
			l2_dma_info[id].dma_frac_offset, l2_dma_info[id].dma_frac_data_len,
			l2_dma_info[id].direction, intr_enable);
		return ;
	}
	l2_dma_info[id].dma_start = true;

	local_irq_save(flags);

	l2cache_invalidate();
	asm("MMU_Clean_Invalidate_Dcache:\n" "mrc  p15,0,r15,c7,c14,3\n" "bne MMU_Clean_Invalidate_Dcache"); 

	/*
	 * Set address of external RAM
	 */
	regval = (unsigned long)l2_dma_info[id].dma_addr;
	regid = (unsigned long)vL2DMA_ADDRBUF0 + id * 4;
	__raw_writel(regval, regid);

	/*
	 * Set DMA operation times
	 */
	regid = (unsigned long)vL2DMA_CONBUF0 + id * 4;
	regval = (cnt_high << 16) | (cnt_low & 0xFF);
	__raw_writel(regval, regid);

	/*
	 * Set DMA direction for L2 common buffer
	 */
	if (id < L2_COMMON_BUFFER_NUM) {
		regval = rL2_CONBUF0_7;
		if (l2_dma_info[id].direction == MEM2BUF) {
			regval |= (1 << (id + L2_COMMON_BUF_CFG_BUF_DIR_START));;
		} else {
			regval &= ~(1 << (id + L2_COMMON_BUF_CFG_BUF_DIR_START)); 
		}
		rL2_CONBUF0_7 = regval;
	}

	
	/*
	 * Start buffer DMA request
	 */
	regval = rL2_DMAREQ;
	regval &= ~(L2_DMA_REQ_FRAC_DMA_REQ | L2_DMA_REQ_BUF_REQ_MASK);
	if (id < L2_COMMON_BUFFER_NUM) {
		regval |= (1 << (id + L2_DMA_REQ_BUF_START));
	} else {
		regval |= (1 << ((id - L2_UART_BUF_START_ID + L2_UART_BUF_CFG_BUF_START)));
	}
	rL2_DMAREQ = regval;

	
	/*
	 * Enable DMA interrupt now
	 */
	if (intr_enable) {
		regval = rL2_BUFINTEN;
		if (id < L2_COMMON_BUFFER_NUM) {
			regval |= 1 << (id + L2_DMA_INTR_ENABLE_BUF_START);
		} else {
			regval |= 1 << (id - L2_COMMON_BUFFER_NUM + L2_DMA_INTR_ENABLE_UART_BUF_START);
		}
		rL2_BUFINTEN = regval;
	}

	local_irq_restore(flags);
}

/**
 * l2_wait_dma_finish - Wait for L2 DMA to finish
 *  @id:	L2 buffer ID involved in DMA transfer
 *  Return true: DMA transfer finished successfully.
 *            false: DMA transfer failed.
 *  NOTE: DMA transfer is started by l2_dma.
 */
static bool l2_wait_dma_finish(u8 id)
{
	unsigned int timeout;
	unsigned long dmareq;
	unsigned long dma_bit;
	const unsigned int max_wait_time = L2_MAX_DMA_WAIT_TIME;

	timeout = 0;
	if (l2_dma_info[id].dma_start) {
		dma_bit = (id < L2_COMMON_BUFFER_NUM) ? (1 << (id + L2_DMA_REQ_BUF_START)) :
			(1 << (id - L2_COMMON_BUFFER_NUM + L2_DMA_REQ_UART_BUF_REQ_START));
		do {
			dmareq = rL2_DMAREQ;
		} while((dmareq & dma_bit) && timeout++ < max_wait_time);

		l2_dma_info[id].dma_start = false;

		if (timeout >= max_wait_time) {
			printk("l2: wait dma timeout, buf id=%d, status=%d.\n", id, l2_get_status(id));
			l2_clear_dma(id);
			__raw_writel(0x0, vL2DMA_CONBUF0 + id * 4);
			return false;
		}

		/*
		 * If fraction DMA  is NOT need, then everything is done.
		 */
		if (!l2_dma_info[id].need_frac) {
			return true;
		}	


		/*
		 * Start fraction DMA here for remain bytes transfer (<64Bytes).
		 */
		l2_dma_info[id].dma_frac_start = true;
		l2_frac_dma((unsigned long)l2_dma_info[id].dma_frac_addr, id,
			l2_dma_info[id].dma_frac_offset, l2_dma_info[id].dma_frac_data_len,
			l2_dma_info[id].direction, false);

	}

	/*
	 * Fraction DMA handling starts here.
	 */
	if (l2_dma_info[id].dma_frac_start) {
		timeout = 0;
		do {
			dmareq = rL2_DMAREQ;
		} while((dmareq & L2_DMA_REQ_FRAC_DMA_REQ) && (timeout++ < max_wait_time));

		l2_dma_info[id].dma_frac_start = false;

		if (timeout >= max_wait_time) {
			printk("l2:wait frac dma timeout, buf id=%d, status=%d.\n", id, l2_get_status(id));
			return false;
		}

		if ((l2_dma_info[id].direction == MEM2BUF) && 
			(l2_dma_info[id].dma_frac_data_len < 60)) {

			unsigned int bufaddr;

			bufaddr = l2_get_addr(id);
			write_buf(0, bufaddr + (l2_dma_info[id].dma_frac_offset & 0x1FF) + 60);
		}
	}

	return true;
}

/**
 * l2_interrupt_handler - L2 memory interrupt handler
 *  @irq:	IRQ number for L2 memory (Must be IRQ_L2MEM)
 *  @dev_id:	Device specific information used by interrupt handler
 *
 *  NOTE: Only shared IRQ need to check @irq & @dev_id.
 *            No need to check them here since L2 memory IRQ is NOT shared IRQ.
 */
static irqreturn_t l2_interrupt_handler(int irq, void *dev_id)
{
	unsigned long regval;
	int i = 0;

	regval = rL2_DMAREQ;

	for (i = 0; i < L2_COMMON_BUFFER_NUM; i++) {
		unsigned long dmapending = regval & (1 << ( i + L2_DMA_REQ_BUF_START));

		if (l2_dma_info[i].dma_start && !dmapending) {
			if (!l2_frac_started && l2_dma_info[i].need_frac) {
				l2_dma_info[i].dma_frac_start = true;
				l2_dma_info[i].dma_start = false;

				l2_frac_dma((unsigned long)l2_dma_info[i].dma_frac_addr, i,
					l2_dma_info[i].dma_frac_offset, l2_dma_info[i].dma_frac_data_len,
					l2_dma_info[i].direction, true);

				l2_frac_started = true;
			} else {
				/* DMA has finished */
				unsigned long regval;

				regval = rL2_BUFINTEN;
				regval &= ~(1 << (i + L2_DMA_INTR_ENABLE_BUF_START));
				rL2_BUFINTEN = regval;

				l2_dma_info[i].dma_start = false;

				if (l2_dma_info[i].callback_func != NULL)
					l2_dma_info[i].callback_func(l2_dma_info[i].data);
				
			}
		}

		if (l2_dma_info[i].dma_frac_start) {
			unsigned long frac_dmapending = regval & L2_DMA_REQ_FRAC_DMA_REQ;
			if (l2_frac_started && !frac_dmapending) {
				l2_frac_started = false;

				switch (l2_dma_info[i].direction) {
				case MEM2BUF:
					if (l2_dma_info[i].dma_frac_data_len <= 60)
						__raw_writel(0x0, AK_VA_L2MEM + i * 512 + 0x1FC);
					break;
				case BUF2MEM:
					if (l2_dma_info[i].dma_frac_data_len <= 512 - 4)
						l2_clear_dma(i);
					break;
				default:
					BUG();
				}
				l2_dma_info[i].dma_frac_start = false;

				if (l2_dma_info[i].callback_func != NULL)
					l2_dma_info[i].callback_func(l2_dma_info[i].data);
				
			}
		}
		
	}

	return IRQ_HANDLED;
}

/**
 * l2_cpu - Transfer data between memory and l2 buffer in CPU mode
 *  @ram_addr:		External RAM address(Physical)
 *  @id:		L2 buffer ID
 *  @buf_offset:	The buffer offset
 *  @bytes:		Data transfer size
 *  @direction:		Data transfer direction between L2 memory and external RAM 
 */
static void l2_cpu(unsigned long ram_addr, u8 id,
	unsigned long buf_offset, unsigned int bytes, l2_dma_transfer_direction_t direction)
{
	int i;
	int j;
	unsigned long trans_no;
	unsigned long frac_no;
	unsigned long buf_count;
	unsigned long buf_remain;
	unsigned long temp_ram;
	unsigned long temp_buf;
	unsigned long bufaddr;
	
	/*
	 * L2 buffer caller MUST guarantee L2 buffer offset is 4-byte aligned
	 */
	if (unlikely(buf_offset % 4))
		BUG();

	bufaddr = l2_get_addr(id);

	if (bufaddr == 0) {
		return ;
	}

	bufaddr += buf_offset;
	trans_no = bytes / 4;
	frac_no = bytes % 4;

	buf_count = (buf_offset + bytes) / L2_BUF_STATUS_MULTIPLY_RATIO;
	buf_remain = (buf_offset + bytes) % L2_BUF_STATUS_MULTIPLY_RATIO;

	switch (direction) {
	case MEM2BUF:
		if (ram_addr % 4) {
			for (i = 0; i < trans_no; i++) {
				temp_ram = 0;
				for (j = 0; j < 4; j++)
					temp_ram |= ((read_ramb(ram_addr + i*4 + j))<<(j*8));
				write_buf(temp_ram, (bufaddr + i * 4));
			}
			if (frac_no) {
				temp_ram = 0;
				for (j = 0; j < frac_no; j++)
					temp_ram |= ((read_ramb(ram_addr + trans_no*4 + j))<<(j*8));
				write_buf(temp_ram, (bufaddr + trans_no * 4));
			}
		} else {
			for (i = 0; i < trans_no; i++)
				write_buf(read_raml(ram_addr + i*4), (bufaddr + i*4));
			if (frac_no)
				write_buf(read_raml(ram_addr + trans_no*4), (bufaddr + trans_no*4));
		}
		
		/*
		 * If we do NOT write data to L2 in multiple of 64Bytes, we must write something to the 4Bytes in 64Bytes-
		 * boundary so that CPU knows writing ends..
		 */
		if ((buf_remain > 0) && (buf_remain <= L2_BUF_STATUS_MULTIPLY_RATIO - 4))
			write_buf(0, (bufaddr - buf_offset + buf_count*L2_BUF_STATUS_MULTIPLY_RATIO + L2_BUF_STATUS_MULTIPLY_RATIO - 4));
		break;
	case BUF2MEM:
		if (ram_addr % 4) {
			for (i = 0; i < trans_no; i++) {
				temp_buf = read_buf(bufaddr + i * 4);
				for (j = 0; j < 4; j++)
					write_ramb((u8)((temp_buf>>j*8) & 0xFF), (ram_addr + i*4 + j));
			}
			if (frac_no) {
				temp_buf = read_buf(bufaddr+trans_no*4);
				for (j = 0; j < frac_no; j++)
					write_ramb((u8)((temp_buf>>j*8) & 0xFF), (ram_addr + trans_no*4 + j));
			}
		} else {
			for (i = 0; i < trans_no; i++)
				write_raml(read_buf(bufaddr+i*4), (ram_addr+i*4));
			if (frac_no) {
				temp_buf = read_buf(bufaddr+trans_no*4);
				temp_ram = read_raml(ram_addr+trans_no*4);
				temp_buf &= ((1<<(frac_no*8+1))-1);
				temp_ram &= ~((1<<(frac_no*8+1))-1);
				temp_ram |= temp_buf;
				write_raml(temp_ram, (ram_addr+trans_no*4));
			}
		}
		
		/*
		 * If we do NOT read data from L2 in multiple of 64Bytes, we must read the 4Bytes in 64Bytes-
		 * boundary so that CPU knows reading ends..
		 */
		if ((buf_remain > 0) && (buf_remain <= L2_BUF_STATUS_MULTIPLY_RATIO - 4))
			temp_buf = read_buf(bufaddr-buf_offset+buf_count*L2_BUF_STATUS_MULTIPLY_RATIO+L2_BUF_STATUS_MULTIPLY_RATIO - 4);
		break;
	default:
		BUG();
	}

}


/**
 * l2_init - Initialize linux kernel L2 memory support
 */
void __init l2_init(void)
{
	int i;
	int retval;

	/*
	 * Enable L2 controller working clock
	 */
	l2_enable_clock(true);

	/*
	 * Initialize all L2 common buffer status to IDLE(could be allocated)
	 */
	for (i = 0; i < L2_COMMON_BUFFER_NUM; i++) {
		l2_buffer_info[i].id = (u8)i;
		l2_buffer_info[i].usable = L2_STAT_IDLE;
		l2_buffer_info[i].used_time = 0;
	}

	/* L2 Memory Register initializations */
	rL2_DMAREQ = L2_DMA_REQ_EN;
	rL2_FRACDMAADDR = L2_FRAC_DMA_AHB_FLAG_EN | L2_FRAC_DMA_LDMA_FLAG_EN;
	rL2_CONBUF0_7 = 0x0;
	rL2_CONBUF8_15 = L2_UART_BUF_CFG_UART_EN_MASK | L2_UART_BUF_CFG_UART_CLR_MASK;
	rL2_BUFINTEN = 0x0;
	rL2_BUFASSIGN1 = 0x0;
	rL2_BUFASSIGN2 = 0x0;

	/* Initialize L2 DMA information status */
	memset(l2_dma_info, 0, ARRAY_SIZE(l2_dma_info));

	/* Initialize global L2 fraction DMA start flag */
	l2_frac_started = false;

	init_waitqueue_head(&l2_wq);

	/* L2 Memory Interrupt handler registered */
	if ((retval = request_irq(IRQ_L2MEM, &l2_interrupt_handler, IRQF_DISABLED, "l2", NULL)) < 0)
		printk(KERN_ERR "l2: failed to request_irq, irq number: %d, retval=%d.\n", IRQ_L2MEM, retval);

	printk("On-chip L2 memory initialized\n");
}

/**
 * __l2_alloc - Allocate a common L2 buffer for given device
 *  @device:	Device ID which need common L2 buffer
 *  Return L2 buffer ID (0 ~ 7)
 *
 *  Only common L2 buffers(ID 0 ~ 7) could be allocated by __l2_alloc.
 *  Other L2 buffers (UART/USB used) is handled by corresponding devices directly.
 */
static u8 __l2_alloc(l2_device_t device, bool need_wait)
{
	int i;
	u16 used_times = MAX_L2_BUFFER_USED_TIMES;
	u8 id = BUF_NULL;
	u8 first_id = BUF_NULL;
	unsigned long flags;
	bool l2_allocated = false;

	if (unlikely(device == ADDR_RESERVED)) {
		printk("l2: unable to allocate l2 buffer for reserved device.\n");
		
		return BUF_NULL;
	}

	if (unlikely(l2_device_info[(u8)device].id != BUF_NULL)) {
		printk("l2: device %d already have a l2 buffer %d\n",
			(int)(u8)device, (int)(u8)l2_device_info[(u8)device].id);
		
		return l2_device_info[(u8)device].id;
	}

	do {
		local_irq_save(flags);

		l2_allocated = false;

		for (i = 1; i < L2_COMMON_BUFFER_NUM; i++) {
			if (l2_buffer_info[i].usable == L2_STAT_IDLE) {
				if (first_id == BUF_NULL) {
					first_id = l2_buffer_info[i].id;
					used_times = l2_buffer_info[i].used_time;
					id = first_id;
				}
				if (l2_buffer_info[i].used_time < used_times) {
					used_times = l2_buffer_info[i].used_time;
					id = l2_buffer_info[i].id;
				}
			}
		}

		if (unlikely(first_id == BUF_NULL)) {
			if(!need_wait) {
				local_irq_restore(flags);
				return BUF_NULL;
			}
			local_irq_restore(flags);
			l2_wait = 0;
			wait_event(l2_wq, l2_wait);
		} else {
			l2_allocated = true;
		}
	} while (!l2_allocated);

	/*
	 * Got a L2 buffer successfully...
	 */
	l2_buffer_info[id].usable = L2_STAT_USED;
	l2_buffer_info[id].used_time++;
	if (l2_buffer_info[id].used_time == 0) {
		/*
		 * In case when the new allocated L2 buffer has been used MAX_L2_BUFFER_USED_TIMES,
		 * we just clear all L2 buffer used times as a simpfied method of balancing 8 L2 buffer usage.
		 */
		for (i = 0; i < L2_COMMON_BUFFER_NUM; i++)
			l2_buffer_info[i].used_time = 0;
	}

	/* Enable L2 buffer */
	l2_combuf_ctrl(id, true);

	/* Change device info */
	l2_device_info[device].id = id;

	/* Select L2 common buffer for device */
	l2_select_combuf(device, id);

	local_irq_restore(flags);

	/* Clear L2 buffer status */
	l2_clr_status(id);

	return id;
}

u8 l2_alloc(l2_device_t device)
{
		return __l2_alloc(device, true);
}
EXPORT_SYMBOL(l2_alloc);

u8 l2_alloc_nowait(l2_device_t device)
{
		return __l2_alloc(device, false);
}
EXPORT_SYMBOL(l2_alloc_nowait);

/**
 * l2_free - Free L2 common buffer for given device
 *  @device:	Device ID which need common L2 buffer
 *  Return L2 buffer ID (0 ~ 7)
 *
 *  Only common L2 buffers(ID 0 ~ 7) could be allocated by l2_alloc.
 *  Other L2 buffers (UART/USB used) is handled by corresponding devices directly.
 *  NOTE: Return the previous L2 buffer ID if a L2 buffer has been allocated to the device.
 *            This means one device could get only one L2 buffer maximum.
 */
void l2_free(l2_device_t device)
{
	u8 id;
	unsigned long regval;
	unsigned long flags;

	id = l2_device_info[(u8)device].id;
	if (unlikely(id == BUF_NULL)) {
		printk("l2: trying to free invalid buffer id %d\n", (int)id);
		return ;
	}

	l2_clear_dma(id);

	local_irq_save(flags);

	/*
	 * Disable DMA interrupt of this L2 buffer.
	 */
	regval = rL2_BUFINTEN;
	regval &= ~(1 << (id + L2_DMA_INTR_ENABLE_BUF_START));
	rL2_BUFINTEN = regval;

	/* Set DMA count to 0 */
	__raw_writel(0x0, vL2DMA_CONBUF0 + id * 4);

	/* Disable this L2 buffer */
	l2_combuf_ctrl(id, false);
	l2_deselect_combuf(device, id);

	/* Clear DMA & DMA fraction flags */
	if (l2_dma_info[id].dma_start || l2_dma_info[id].dma_frac_start) {
		l2_dma_info[id].dma_start = false;
		l2_dma_info[id].dma_frac_start = false;
	}

	l2_dma_info[id].callback_func = NULL;
	l2_dma_info[id].data = 0;

	l2_device_info[(u8)device].id = BUF_NULL;
	l2_buffer_info[id].usable = L2_STAT_IDLE;

	l2_wait = 1;
	wake_up(&l2_wq);

	local_irq_restore(flags);

}
EXPORT_SYMBOL(l2_free);

/**
 * l2_set_dma_callback - Set callback function when L2 DMA/fraction DMA interrupt handler is done
 *  @id:	L2 buffer ID
 *  @func:	Callback function
 *  Return true(Always)
 *  
 *  NOTE: Caller MUST guarantee that L2 buffer ID is valid. And since the callback function is called
 *  in interrupt handler, it MUST NOT call any functions which may sleep.
 */
bool l2_set_dma_callback(u8 id, l2_callback_func_t func, unsigned long data)
{
	if (unlikely(id >= L2_COMMON_BUFFER_NUM)) {
		printk(KERN_ERR "l2: Set dma callback, invalid buf id[%d].\n", id);
		return false;
	}

	if (unlikely(l2_dma_info[id].dma_start || l2_dma_info[id].dma_frac_start)) {
		printk(KERN_ERR "l2: Set dma callback, dma not finished.\n");
		return false;
	}

	l2_dma_info[id].callback_func = func;
	l2_dma_info[id].data = data;

	return true;
}
EXPORT_SYMBOL(l2_set_dma_callback);

/**
 * l2_combuf_dma - Start data tranferring between memory and l2 common buffer in DMA mode
 *  @ram_addr:		External RAM address(Physical)
 *  @id:		L2 buffer ID involved in DMA transfer
 *  @bytes:		Data transfer size
 *  @direction:		Data transfer direction between L2 memory and external RAM 
 *  @intr_enable:	Open interrupt for this L2 buffer or not
 */
void l2_combuf_dma(unsigned long ram_addr, u8 id, unsigned int bytes, l2_dma_transfer_direction_t direction, bool intr_enable)
{
	if (unlikely(id >= L2_COMMON_BUFFER_NUM)) {
		printk("l2: begin common buffer dma, error buf id=[%d].\n", id);
		return ;
	}

	l2_dma(ram_addr, id, bytes, direction, intr_enable);
}
EXPORT_SYMBOL(l2_combuf_dma);

/**
 * l2_combuf_wait_dma_finish - Wait for L2 DMA to finish
 *  @id:	L2 buffer ID involved in DMA transfer
 *  Return true: DMA transfer finished successfully.
 *            false: DMA transfer failed.
 *  NOTE: DMA transfer is started by l2_combuf_dma.
 */
bool l2_combuf_wait_dma_finish(u8 id)
{
	if (unlikely(id >= L2_COMMON_BUFFER_NUM)) {
		printk("l2: begin common buffer dma, error buf id=[%d].\n", id);
		return false;
	}
	return l2_wait_dma_finish(id);
}
EXPORT_SYMBOL(l2_combuf_wait_dma_finish);

/**
 * l2_combuf_cpu - Transfer data between memory and l2 common buffer in CPU mode
 *  @ram_addr:	External RAM address(Physical)
 *  @id:	L2 buffer ID
 *  @bytes:	Data transfer size
 *  @direction:	Data transfer direction between L2 memory and external RAM
 *
 *  NOTE: According to XuChang, if one transfer data from Peripheral --> L2 Buffer --> RAM, 
 *            special care need to be taken when data size is NOT multiple of 64Bytes.
 *            Pheripheral driver must check hardware signals to confirm data has been transfer from
 *            peripheral to L2 buffer since L2 do NOT provide some mechanism to confirm data has
 *            been in L2 Buffer. Driver can and only can call l2_combuf_cpu() to copy data from L2
 *            Buffer --> RAM after checking hardware signals.
 *            As to 64Bytes * n size data, L2 could check Buffer Status Status Counter to confirm that
 *            Data has been transfer from peripheral to L2 buffer, so no hardware signals checking needed.
 */
void l2_combuf_cpu(unsigned long ram_addr, u8 id,
	unsigned int bytes, l2_dma_transfer_direction_t direction)
{
	int i;
	int loop;
	int remain;

	loop = bytes / L2_BUF_STATUS_MULTIPLY_RATIO;
	remain = bytes % L2_BUF_STATUS_MULTIPLY_RATIO;

	switch (direction) {
	case MEM2BUF:
		for (i = 0; i < loop; i++) {
			
			while (l2_get_status(id) == (L2_BUFFER_SIZE / L2_BUF_STATUS_MULTIPLY_RATIO))
				;	/* Waiting for L2 buffer to NOT full(means writable) */
			
			l2_cpu(ram_addr + i * L2_BUF_STATUS_MULTIPLY_RATIO, id,
				(i % 8) * L2_BUF_STATUS_MULTIPLY_RATIO, L2_BUF_STATUS_MULTIPLY_RATIO, direction);
		}
		if (remain > 0) {
			while (l2_get_status(id) > 0)
				;	/* Waiting for L2 buffer to empty */

			l2_cpu(ram_addr + loop * L2_BUF_STATUS_MULTIPLY_RATIO, id,
				(loop % 8) * L2_BUF_STATUS_MULTIPLY_RATIO, remain, direction);
		}
		break;
	case BUF2MEM:
		for (i = 0; i < loop; i++) {
			while (l2_get_status(id) == 0)
				;	/* Waiting for L2 buffer to be not empty (means readable) */
			
			l2_cpu(ram_addr + i * L2_BUF_STATUS_MULTIPLY_RATIO, id,
				(i % 8) * L2_BUF_STATUS_MULTIPLY_RATIO, L2_BUF_STATUS_MULTIPLY_RATIO, direction);
			
		}
		if (remain > 0) {
			l2_cpu(ram_addr + loop * L2_BUF_STATUS_MULTIPLY_RATIO, id,
				(loop % 8) * L2_BUF_STATUS_MULTIPLY_RATIO, remain, direction);
		}
		break;
	default:
		BUG();
	}
}
EXPORT_SYMBOL(l2_combuf_cpu);

/**
 * l2_get_status - Get L2 buffer status
 *  @id:	L2 buffer ID
 */
u8 l2_get_status(u8 id)
{
	l2_assert_buf_id(id);

	return (id < L2_COMMON_BUFFER_NUM) ? (rL2_BUFSTAT1 >> (id * 4)) & 0xF :
		(rL2_BUFSTAT2 >> ((id - L2_UART_BUF_START_ID) << 1)) & 0x3;
}
EXPORT_SYMBOL(l2_get_status);

/**
 * l2_clr_status - Clear L2 buffer status
 *  @id:	L2 buffer ID
 */
void l2_clr_status(u8 id)
{
	unsigned long flags;

	l2_assert_buf_id(id);
	
	local_irq_save(flags);

	if (id < L2_COMMON_BUFFER_NUM) {
		rL2_CONBUF0_7 |= 1 << (id + L2_COMMON_BUF_CFG_BUF_CLR_START);
	} else {
		rL2_CONBUF8_15 |= (1 << (id - L2_UART_BUF_START_ID + L2_UART_BUF_CFG_BUF_START));
	}

	local_irq_restore(flags);

}
EXPORT_SYMBOL(l2_clr_status);

/**
 * l2_set_status - Clear L2 buffer status
 *  @id:	L2 buffer ID
 *  @status:	Status to be set (0 ~ 8)
 */
void l2_set_status(u8 id, u8 status)
{
	unsigned long regval;
	unsigned long flags;
	
	l2_assert_buf_id(id);

	if ((id >= L2_COMMON_BUFFER_NUM) || status > MAX_L2_DMA_STATUS_VALUE)
		BUG();

	local_irq_save(flags);

	/*
	 * Enable CPU-controlled buffer function and set L2 buffer `id' status
	 * status = current number of data in the CPU controlled buffer.
	 */
	regval = rL2_CONBUF8_15;
	regval &= ~(L2_UART_BUF_CFG_CPU_BUF_NUM_MASK | L2_UART_BUF_CFG_CPU_BUF_SEL_EN |
		L2_UART_BUF_CFG_CPU_BUF_SEL_MASK);
	regval |= (id << L2_UART_BUF_CFG_CPU_BUF_SEL_START) | L2_UART_BUF_CFG_CPU_BUF_SEL_EN |
		(status << L2_UART_BUF_CFG_CPU_BUF_NUM_START);
	rL2_CONBUF8_15 = regval;

	/*
	* Disable CPU-controlled buffer function
	*/
	regval = rL2_CONBUF8_15;
	regval &= ~(L2_UART_BUF_CFG_CPU_BUF_NUM_MASK | L2_UART_BUF_CFG_CPU_BUF_SEL_EN |
		L2_UART_BUF_CFG_CPU_BUF_SEL_MASK);
	rL2_CONBUF8_15 = regval;

	local_irq_restore(flags);

}
EXPORT_SYMBOL(l2_set_status);
