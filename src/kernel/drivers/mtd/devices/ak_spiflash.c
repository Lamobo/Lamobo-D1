 /**
 *  @file      /driver/mtd/devices/ak_SPIFlash.c
 *  @brief     SPI Flash driver for Anyka AK37 platform.
 *   Copyright C 2012 Anyka CO.,LTD
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *  @author    She Shaohua
 *  @date      2012-03-23
 *  @note      2011-03-20  created
 *  @note      2011-03-23  Debug OK.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/delay.h>
#include <mach-anyka/fha.h>
#include <mach-anyka/anyka_types.h>
#include <linux/mm.h>


//#define SPIFLASH_DEBUG
#undef PDEBUG           
#ifdef SPIFLASH_DEBUG
#define PDEBUG(fmt, args...) printk( KERN_INFO fmt,## args)
#define DEBUG(n, args...) printk(KERN_INFO args)
#else
#define PDEBUG(fmt, args...)
#define DEBUG(n, args...) 
#endif

#define SPI_FLASH_BIN_PAGE_START 	558

#define FLASH_BUF_SIZE			(32*1024)
#define FLASH_PAGESIZE		256

#define SPI_FLASH_READ		1
#define SPI_FLASH_WRITE		2

#define CONFIG_SPIFLASH_USE_FAST_READ 1

/*mtd layer allocate memory use for 'vmalloc' interface, need to convert.*/
//#define SPIFLASH_USE_MTD_BLOCK_LAYER  

#define OPCODE_WREN  			0x06    /* Write Enable */ 
#define OPCODE_WRDI     		0x04    /* Write Disable */ 
#define OPCODE_RDSR1    		0x05    /* Read Status Register1 */
#define OPCODE_RDSR2     		0x35    /* Read Status Register2 */ 
#define OPCODE_RDSR3     		0x15    /* Read Status Register3 */ 

#define OPCODE_WRSR1          	0x01    /* Write Status Register */ 
#define OPCODE_WRSR2          	0x31    /* Write Status2 Register eg:gd25q128c*/ 
#define OPCODE_WRSR3          	0x11    /* Write Status3 Register eg:gd25q128c*/ 

#define OPCODE_NORM_READ     	0x03    /* Read Data Bytes */ 
#define OPCODE_FAST_READ      	0x0b    /* Read Data Bytes at Higher Speed */ 
#define OPCODE_FAST_D_READ     	0x3b    /* Read Data Bytes at Dual output */ 
#define OPCODE_FAST_Q_READ     	0x6b    /* Read Data Bytes at Quad output */ 
#define OPCODE_FAST_D_IO     	0xbb    /* Read Data Bytes at Dual i/o */ 
#define OPCODE_FAST_Q_IO     	0xeb    /* Read Data Bytes at Quad i/o */ 

#define OPCODE_PP            	0x02    /* Page Program */
#define OPCODE_PP_DUAL			0x12	/* Dual Page Program*/
#define OPCODE_PP_QUAD			0x32	/* Quad Page Program*/
#define OPCODE_2IO_PP			0x18	/* 2I/O Page Program (tmp)*/
#define OPCODE_4IO_PP			0x38	/* 4I/O Page Program*/

#define OPCODE_BE_4K         	0x20    /* Sector (4K) Erase */ 
#define OPCODE_BE_32K       	0x52    /* Block (32K) Erase */
#define OPCODE_BE_64K          	0xd8    /* Block (64K) Erase */ 
#define	OPCODE_SE				0xd8	/* Sector erase (usually 64KiB) */
#define OPCODE_CHIP_ERASE     	0xc7    /* Chip Erase */ 
#define	OPCODE_RDID				0x9f	/* Read JEDEC ID */
#define OPCODE_DP           	0xb9    /* Deep Power-down */ 
#define OPCODE_RES          	0xab    /* Release from DP, and Read Signature */ 


#define SPI_STATUS_REG1	1
#define SPI_STATUS_REG2	2
#define SPI_STATUS_REG3	3


/* Used for SST flashes only. */
#define	OPCODE_BP		0x02	/* Byte program */
#define	OPCODE_WRDI		0x04	/* Write disable */
#define	OPCODE_AAI_WP		0xad	/* Auto address increment word program */


/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(40 * HZ)	/* 40s max chip erase */

#define	CMD_SIZE		(1)
#define ADDR_SIZE		(3)
#define CMD_ADDR_SIZE	(CMD_SIZE + ADDR_SIZE)
#define MAX_DUMMY_SIZE	(4)

#define MTD_PART_NAME_LEN (4)

#ifdef CONFIG_SPIFLASH_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#define ALIGN_DOWN(a, b)  (((a) / (b)) * (b))

/****************************************************************************/
struct partitions
{
	char name[MTD_PART_NAME_LEN]; 		   
	unsigned long long size;
	unsigned long long offset;         
	unsigned int mask_flags;
}__attribute__((packed));

typedef struct
{
    T_U32 BinPageStart; /*bin data start addr*/
    T_U32 PageSize;     /*spi page size*/
    T_U32 PagesPerBlock;/*page per block*/
    T_U32 BinInfoStart;
    T_U32 FSPartStart;
}
T_SPI_BURN_INIT_INFO;


/*
 * SPI device driver setup and teardown
 */
struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32			jedec_id;
	u16			ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16			n_sectors;

	/**
	 *  chip character bits:
	 *  bit 0: under_protect flag, the serial flash under protection or not when power on
	 *  bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
	 *  bit 2: AAI flag, the serial flash support auto address increment word programming
	 *  bit 3: support dual write or no
	 *  bit 4: support dual read or no
	 *  bit 5: support quad write or no
	 *  bit 6: support quad read or no
	 *  bit 7: the second status command (35h) flag,if use 4-wire(quad) mode,the bit must be is enable
	 */
	u16			flags;
#define	SFLAG_UNDER_PROTECT			(1<<0)
#define SFLAG_FAST_READ           	(1<<1)
#define SFLAG_AAAI                	(1<<2)
#define SFLAG_COM_STATUS2         	(1<<3)

#define SFLAG_DUAL_IO_READ         	(1<<4)
#define SFLAG_DUAL_READ           	(1<<5)
#define SFLAG_QUAD_IO_READ         	(1<<6)
#define SFLAG_QUAD_READ           	(1<<7)

#define SFLAG_DUAL_IO_WRITE        	(1<<8)
#define SFLAG_DUAL_WRITE          	(1<<9)
#define SFLAG_QUAD_IO_WRITE        	(1<<10)
#define SFLAG_QUAD_WRITE          	(1<<11)

#define SFLAG_SECT_4K       		(1<<12)
#define SFLAG_COM_STATUS3         	(1<<13)
};

struct ak_spiflash;

/**
  *because of some spi flash is difference of status register difinition.
  *this structure use mapping the status reg function and corresponding.
*/
struct flash_status_reg
{
	u32		jedec_id;	
	u16		ext_id;
	unsigned b_wip:4;		/*write in progress*/
	unsigned b_wel:4;		/*wrute ebabke latch*/
	unsigned b_bp0:4;		/*block protected 0*/
	unsigned b_bp1:4;		/*block protected 1*/
	unsigned b_bp2:4;		/*block protected 2*/
	unsigned b_bp3:4;		/*block protected 3*/
	unsigned b_bp4:4;		/*block protected 4*/
	unsigned b_srp0:4;		/*status register protect 0*/
	
	unsigned b_srp1:4;		/*status register protect 1*/
	unsigned b_qe:4;		/*quad enable*/
	unsigned b_lb:4;		/*write protect control and status to the security reg.*/
/*
	unsigned b_reserved0:4;
	unsigned b_reserved1:4;
	unsigned b_reserved2:4;
*/
	unsigned b_cmp:4;		/*conjunction bp0-bp4 bit*/
	unsigned b_sus:4;		/*exec an erase/program suspend command*/

	u32 (*read_sr)(struct ak_spiflash *);
	int (*write_sr)(struct ak_spiflash *, u32);
};

struct ak_spiflash {
	struct spi_device	*spi;
	struct mutex		lock;
	struct flash_info	info;
	struct mtd_info		mtd;
	unsigned			partitioned:1;
	
	u8		bus_width;
	unsigned char 		*buf;
	u8		command[CMD_ADDR_SIZE + MAX_DUMMY_SIZE];
	u8		dummy_len;

	u8		erase_opcode;
	u8		tx_opcode;
	u8		rx_opcode;
	u8		txd_bus_width;
	u8		rxd_bus_width;
	
	u8		txa_bus_width;
	u8		rxa_bus_width;	
	struct flash_status_reg stat_reg;
};

static struct mtd_info *ak_mtd_info;

static inline int write_enable(struct ak_spiflash *flash);
static int wait_till_ready(struct ak_spiflash *flash);

static inline struct ak_spiflash *mtd_to_spiflash(struct mtd_info *mtd)
{
	return container_of(mtd, struct ak_spiflash, mtd);
}

#ifdef SPIFLASH_USE_MTD_BLOCK_LAYER
/**
* @brief: because of the _read() function call by mtd block layer, the buffer be
* allocate by vmalloc() in mtd layer, spi driver layer may use this buffer that 
* intents of use for DMA transfer, so, add this function to transition buffer.
* call this function at before real read/write data.
* 
* @author lixinhai
* @date 2013-04-10
* @param[in] flash  spiflash handle.
* @param[in] buffer.
* @param[in] buffer len
* @param[in] read/write
* @retval return the transition buffer
*/
static void *flash_buf_bounce_pre(struct ak_spiflash *flash,
				void *buf, u32 len, int dir)
{
	if(!is_vmalloc_addr(buf)) {
		return buf;
	}

	if(dir == SPI_FLASH_WRITE) {
		memcpy(flash->buf, buf, len);
	}
	return flash->buf;
}

/**
* @brief: because of the _read() function call by mtd block layer, the buffer be
* allocate by vmalloc() in mtd layer, spi driver layer may use this buffer that 
* intents of use for DMA transfer, so, add this function to transition buffer.
* call this function at after real read/write data
* 
* @author lixinhai
* @date 2013-04-10
* @param[in] flash  spiflash handle.
* @param[in] buffer.
* @param[in] buffer len
* @param[in] read/write
* @retval return the transition buffer
*/
static void flash_buf_bounce_post(struct ak_spiflash *flash,
				void *buf, u32 len, int dir)
{
	if(!is_vmalloc_addr(buf)) {
		return;
	}

	if(dir == SPI_FLASH_READ) {
		memcpy(buf, flash->buf, len);
	}
}
#else
static inline void *flash_buf_bounce_pre(struct ak_spiflash *flash,
				void *buf, u32 len, int dir)
{
	return buf;
}

static inline void flash_buf_bounce_post(struct ak_spiflash *flash,
				void *buf, u32 len, int dir)
{
}

#endif

static int gd25q128c_write_sr(struct ak_spiflash *flash, u32 val)
{
	int ret;

	wait_till_ready(flash);
	write_enable(flash);
	flash->command[0] = OPCODE_WRSR1;
	flash->command[1] = val & 0xff;
	ret = spi_write(flash->spi, flash->command, 2);

	wait_till_ready(flash);
	write_enable(flash);
	flash->command[0] = OPCODE_WRSR2;
	flash->command[1] = (val>>8) &0xff;
	ret |= spi_write(flash->spi, flash->command, 2);
	wait_till_ready(flash);

	return ret;
}


static u32 normal_read_sr(struct ak_spiflash *flash)
{
	ssize_t retval;
	u8 code;
	u32 status;
	u8 st_tmp= 0;

	code = OPCODE_RDSR1;

	if((retval = spi_write_then_read(flash->spi, &code, 1, &st_tmp, 1))<0)
		return retval;

	status = st_tmp;
	if(flash->info.flags & SFLAG_COM_STATUS2){
		code = OPCODE_RDSR2;
		if((retval = spi_write_then_read(flash->spi, &code, 1, &st_tmp, 1))<0)
			return retval;
		
		 status = (status | (st_tmp << 8));		
	}

   	if(flash->info.flags & SFLAG_COM_STATUS3){
		code = OPCODE_RDSR3;
		if((retval = spi_write_then_read(flash->spi, &code, 1, &st_tmp, 1))<0)
			return retval;
		
		 status = (status | (st_tmp << 16));		
	}

	return status;
}

static int normal_write_sr(struct ak_spiflash *flash, u32 val)
{
	int wr_cnt;
	
	flash->command[0] = OPCODE_WRSR1;
	flash->command[1] = val & 0xff;
	flash->command[2] = (val>>8) &0xff;
	
    if (flash->info.flags & SFLAG_COM_STATUS2) {
        wr_cnt = 3;
    } else {
        wr_cnt = 2;
    }

	return spi_write(flash->spi, flash->command, wr_cnt);
}


/*
 * Internal helper functions
 */

/**
* @brief Read the status register.
* 
*  returning its value in the location
* @author SheShaohua
* @date 2012-03-20
* @param[in] spiflash handle.
* @return int Return the status register value.
*/
static u32 read_sr(struct ak_spiflash *flash)
{
	struct flash_status_reg *fsr = &flash->stat_reg;

	if(fsr && fsr->read_sr)
		return fsr->read_sr(flash);

	return -EINVAL;
}


/**
* @brief Write status register
* 
*  Write status register 1 byte.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash  spiflash handle.
* @param[in] val  register value to be write.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a negative error code if failed
*/
static int write_sr(struct ak_spiflash *flash, u32 val)
{
	struct flash_status_reg *fsr = &flash->stat_reg;

	if(fsr && fsr->write_sr)
		return fsr->write_sr(flash, val);

	return -EINVAL;
}


/**
* @brief Set write enable latch.
* 
*  Set write enable latch with Write Enable command.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash  spiflash handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a negative error code if failed
*/
static inline int write_enable(struct ak_spiflash *flash)
{
	u8	code = OPCODE_WREN;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}


/**
* @brief Set write disble
* 
*  Set write disble instruction to the chip.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash	spiflash handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a negative error code if failed
*/
static inline int write_disable(struct ak_spiflash *flash)
{
	u8	code = OPCODE_WRDI;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}

/**
* @brief  Wait for SPI flash ready.
* 
*  Service routine to read status register until ready, or timeout occurs.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash	spiflash handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int wait_till_ready(struct ak_spiflash *flash)
{
	unsigned long deadline;
	u32 sr;
	struct flash_status_reg *fsr = &flash->stat_reg;

	deadline = jiffies + MAX_READY_WAIT_JIFFIES;

	do {
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & (1<<fsr->b_wip)))
			return 0;

		cond_resched();

	} while (!time_after_eq(jiffies, deadline));

	return 1;
}

static int quad_mode_enable(struct ak_spiflash *flash)
{
	int ret;
	u32 regval;	
	struct flash_status_reg *fsr = &flash->stat_reg;
	
	ret = wait_till_ready(flash);
	if (ret)
		return -EBUSY;

	write_enable(flash);
	
	regval = read_sr(flash);
	regval |= 1<<fsr->b_qe;
	write_sr(flash, regval);

	write_disable(flash);
	return 0;
}

static int quad_mode_disable(struct ak_spiflash *flash)
{
	int ret;
	u32 regval;
	struct flash_status_reg *fsr = &flash->stat_reg;
		
	ret = wait_till_ready(flash);
	if (ret)
		return -EBUSY;
	
	write_enable(flash);
	
	regval = read_sr(flash);
	regval &= ~(1<<fsr->b_qe);
	write_sr(flash, regval);

	write_disable(flash);
	return 0;
}

/**
* @brief   Erase chip
* 
*  Erase the whole flash memory.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash	spiflash handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int erase_chip(struct ak_spiflash *flash)
{
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %lldKiB\n",
	      dev_name(&flash->spi->dev), __func__,
	      (long long)(flash->mtd.size >> 10));

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return -EBUSY;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;

	spi_write(flash->spi, flash->command, 1);

	return 0;
}



/**
* @brief  Erase sector
* 
*  Erase a sector specialed by user.
* @author SheShaohua
* @date 2012-03-20
* @param[in] flash	    spiflash handle.
* @param[in] offset    which is any address within the sector which should be erased.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int erase_sector(struct ak_spiflash *flash, u32 offset)
{
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB at 0x%08x\n",
			dev_name(&flash->spi->dev), __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return -EBUSY;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spi_write(flash->spi, flash->command, CMD_ADDR_SIZE);

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */


/**
* @brief  MTD Erase
* 
* Erase an address range on the flash chip.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd    mtd info handle.
* @param[in] instr   erase info.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int ak_spiflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct ak_spiflash *flash = mtd_to_spiflash(mtd);
	u32 addr,len;
	uint32_t rem;

	PDEBUG("ak_spiflash_erase\n");
	//printk(": instr->len=%lld, mtd->erasesize=%ld, addr=%lld\n", instr->len,mtd->erasesize,(long long)instr->addr);

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%llx, len %lld\n",
	      dev_name(&flash->spi->dev), __func__, "at",
	      (long long)instr->addr, (long long)instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > mtd->size)
	{
		printk(KERN_ERR "ak_spiflash_erase:instr->addr[0x%llx] + instr->len[%lld] > mtd->size[%lld]\n",
			instr->addr, instr->len, mtd->size );
		return -EINVAL;
	}
	div_u64_rem(instr->len, mtd->erasesize, &rem);
	if (rem != 0)
	{
		printk(KERN_ERR "ak_spiflash_erase:rem!=0 [%u]\n", rem );
		return -EINVAL;
	}

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* whole-chip erase? */
	if (len == mtd->size) {
		if (erase_chip(flash)) {
			instr->state = MTD_ERASE_FAILED;
			mutex_unlock(&flash->lock);
			return -EIO;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			if (erase_sector(flash, addr)) {
				instr->state = MTD_ERASE_FAILED;
				mutex_unlock(&flash->lock);
				return -EIO;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}


static int init_spiflash_rw_info(struct ak_spiflash *flash)
{
	/**default param.*/
	flash->rx_opcode = OPCODE_READ;
	flash->rxd_bus_width = XFER_1DATAWIRE;
	flash->rxa_bus_width = XFER_1DATAWIRE;
	flash->tx_opcode = OPCODE_PP;
	flash->txd_bus_width = XFER_1DATAWIRE;
	flash->txa_bus_width = XFER_1DATAWIRE;
	flash->dummy_len = 1;
	
	if(flash->bus_width & FLASH_BUS_WIDTH_2WIRE){
		if(flash->info.flags & SFLAG_DUAL_READ) {
			flash->rx_opcode = OPCODE_FAST_D_READ;
			flash->rxd_bus_width = XFER_2DATAWIRE;
			flash->rxa_bus_width = XFER_1DATAWIRE;
			flash->dummy_len = 1;
		} else if (flash->info.flags & SFLAG_DUAL_IO_READ) {
			flash->rx_opcode = OPCODE_FAST_D_IO;
			flash->rxd_bus_width = XFER_2DATAWIRE;
			flash->rxa_bus_width = XFER_2DATAWIRE;
			flash->dummy_len = 1;
		}

		if(flash->info.flags & SFLAG_DUAL_WRITE) {
			flash->tx_opcode = OPCODE_PP_DUAL;
			flash->txd_bus_width = XFER_2DATAWIRE;
			flash->txa_bus_width = XFER_1DATAWIRE;
		} else if(flash->info.flags & SFLAG_DUAL_IO_WRITE) {
			flash->tx_opcode = OPCODE_2IO_PP;
			flash->txd_bus_width = XFER_2DATAWIRE;
			flash->txa_bus_width = XFER_2DATAWIRE;
		}	
	}

	if(flash->bus_width & FLASH_BUS_WIDTH_4WIRE){
		if(flash->info.flags & SFLAG_QUAD_READ) {
			flash->rx_opcode = OPCODE_FAST_Q_READ;
			flash->rxd_bus_width = XFER_4DATAWIRE;
			flash->rxa_bus_width = XFER_1DATAWIRE;
			flash->dummy_len = 1;
		}else if(flash->info.flags & SFLAG_QUAD_IO_READ){
			flash->rx_opcode = OPCODE_FAST_Q_IO;
			flash->rxd_bus_width = XFER_4DATAWIRE;
			flash->rxa_bus_width = XFER_4DATAWIRE;
			flash->dummy_len = 3;
		}

		if(flash->info.flags & SFLAG_QUAD_WRITE) {
			flash->tx_opcode = OPCODE_PP_QUAD;
			flash->txd_bus_width = XFER_4DATAWIRE;			
			flash->txa_bus_width = XFER_1DATAWIRE;
		}else if(flash->info.flags & SFLAG_QUAD_IO_WRITE) {
			flash->tx_opcode = OPCODE_4IO_PP;
			flash->txd_bus_width = XFER_4DATAWIRE;
			flash->txa_bus_width = XFER_4DATAWIRE;
		}
	
	}
	return 0;
}

static int ak_spiflash_cfg_quad_mode(struct ak_spiflash *flash)
{
	int ret = 0;
	
	if((flash->bus_width & FLASH_BUS_WIDTH_4WIRE) && 
		(flash->info.flags & (SFLAG_QUAD_WRITE|SFLAG_QUAD_IO_WRITE|
			SFLAG_DUAL_READ|SFLAG_DUAL_IO_READ))) {		
		ret = quad_mode_enable(flash);
		if(ret) {
			flash->bus_width &= ~FLASH_BUS_WIDTH_4WIRE;
			printk("config the spiflash quad enable fail. transfer use 1 wire.\n");
		}
	}
	else
		quad_mode_disable(flash);

	return ret;
}


#define FILL_CMD(c, val) do{c[0] = (val);}while(0)
#define FILL_ADDR(c, val) do{	\
		c[CMD_SIZE] = (val) >> 16;	\
		c[CMD_SIZE+1] = (val) >> 8;	\
		c[CMD_SIZE+2] = (val);		\
		}while(0)
		
#define FILL_DUMMY_DATA(c, val) do{	\
			c[CMD_ADDR_SIZE] = val >> 16;	\
			c[CMD_ADDR_SIZE+1] = 0;	\
			c[CMD_ADDR_SIZE+2] = 0;	\
			c[CMD_ADDR_SIZE+3] = 0;	\
			}while(0)


static int spiflash_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct ak_spiflash *flash = mtd_to_spiflash(mtd);
	struct spi_transfer t[3];
	struct spi_message m;
	void *bounce_buf;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));
	
	mutex_lock(&flash->lock);

	bounce_buf = flash_buf_bounce_pre(flash, buf, len, SPI_FLASH_READ);

	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = &flash->command[CMD_SIZE];
	t[1].len = ADDR_SIZE + flash->dummy_len;
	t[1].xfer_mode = flash->rxa_bus_width;
	spi_message_add_tail(&t[1], &m);

	t[2].rx_buf = bounce_buf;
	t[2].len = len;	
	t[2].cs_change = 1;	
	t[2].xfer_mode = flash->rxd_bus_width;

	spi_message_add_tail(&t[2], &m);

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */		
		mutex_unlock(&flash->lock);
		return -EBUSY;
	}

	/* Set up the write data buffer. */
	FILL_CMD(flash->command, flash->rx_opcode);
	FILL_ADDR(flash->command, from);
	FILL_DUMMY_DATA(flash->command, 0x00);

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - CMD_ADDR_SIZE - flash->dummy_len;
	
	flash_buf_bounce_post(flash, buf, len, SPI_FLASH_READ);
	
	mutex_unlock(&flash->lock);

	return 0;
}


static int ak_spiflash_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	int ret = 0;
	size_t rlen = 0;
	u32 xfer_len;
	u32 offset = 0;
	u32 count = len;

	while(count > 0) {
		xfer_len = (count > FLASH_BUF_SIZE) ? FLASH_BUF_SIZE : count;

		if(xfer_len > FLASH_PAGESIZE)
			xfer_len = ALIGN_DOWN(xfer_len, FLASH_PAGESIZE);

		ret = spiflash_read(mtd, from + offset, xfer_len, &rlen, buf + offset);
		if(unlikely(ret)) {
			ret = -EBUSY;
			goto out;
		}
		
		*retlen += rlen;
		count -= rlen;		
		offset += rlen;
	}	
out:
	return ret;
}


/**
* @brief   MTD write
* 
* Write an address range to the flash chip.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd	mtd info handle.
* @param[in] to 	write start address.
* @param[in] len	write length.
* @param[out] retlen  write length at actually.
* @param[out] buf	   the pointer to write data.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int ak_spiflash_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct ak_spiflash *flash = mtd_to_spiflash(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[3];
	struct spi_message m;
	void *bounce_buf;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			dev_name(&flash->spi->dev), __func__, "to",
			(u32)to, len);

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > mtd->size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));
	
	mutex_lock(&flash->lock);
	bounce_buf = flash_buf_bounce_pre(flash, (void*)buf, len, SPI_FLASH_WRITE);

	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = &flash->command[CMD_SIZE];
	t[1].len = ADDR_SIZE;
	t[1].xfer_mode = flash->txa_bus_width;
	spi_message_add_tail(&t[1], &m);

	t[2].tx_buf = bounce_buf;
	t[2].cs_change = 1;
	t[2].xfer_mode = flash->txd_bus_width;

	spi_message_add_tail(&t[2], &m);

	//memcpy(flash->buf, buf, len);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		mutex_unlock(&flash->lock);
		return 1;
	}

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	FILL_CMD(flash->command, flash->tx_opcode);
	FILL_ADDR(flash->command, to);

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) {
		t[2].len = len;

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_ADDR_SIZE;
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		t[2].len = page_size;
		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_ADDR_SIZE;

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			/* write the next page to flash */
			FILL_ADDR(flash->command, to+i);

			t[2].tx_buf = buf + i;
			t[2].len = page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			if (retlen)
				*retlen += m.actual_length - CMD_ADDR_SIZE;
		}
	}

	PDEBUG("ak_spiflash_write: retlen=%ld\n", *retlen);
	flash_buf_bounce_post(flash, (void*)buf, len, SPI_FLASH_WRITE);

	mutex_unlock(&flash->lock);

	return 0;
}

/**
* @brief	MTD get device ID
* 
* get the device ID of  the spi flash chip.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd	 mtd info handle.
* @return int return device ID of  the spi flash chip.
*/
static int ak_spiflash_get_devid(struct mtd_info *mtd)
{
	struct ak_spiflash *flash = mtd_to_spiflash(mtd);
	int			ret;
	u8			code = OPCODE_RDID;
	u8			id[5];
	u32			jedec;

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return -EBUSY;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	ret = spi_write_then_read(flash->spi, &code, 1, id, 3);
	if (ret < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d ak_spiflash_get_devid\n",
			dev_name(&flash->spi->dev), ret);
		return AK_FALSE;
	}

	jedec = id[0] | (id[1]<<8) | (id[2]<<16);
	printk("spi flash ID: 0x%08x\n", jedec);

	return jedec;
}


 /**
 * @brief	MTD write only for SST spi flash.
 * 
 * Write an address range to the flash chip.
 * @author SheShaohua
 * @date 2012-03-20
 * @param[in] mtd	 mtd info handle.
 * @param[in] to	 write start address.
 * @param[in] len	 write length.
 * @param[out] retlen  write length at actually.
 * @param[out] buf		the pointer to write data.
 * @return int return write success or failed
 * @retval returns zero on success
 * @retval return a non-zero error code if failed
 */
static int sst_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct ak_spiflash *flash = mtd_to_spiflash(mtd);
	struct spi_transfer t[2];
	struct spi_message m;
	size_t actual;
	int cmd_sz, ret;

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return 0;

	if (to + len > flash->mtd.size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = CMD_ADDR_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	ret = wait_till_ready(flash);
	if (ret)
		goto time_out;

	write_enable(flash);

	actual = to % 2;
	/* Start write from odd address. */
	if (actual) {
		flash->command[0] = OPCODE_BP;
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;

		/* write one byte. */
		t[1].len = 1;
		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - CMD_ADDR_SIZE;
	}
	to += actual;

	flash->command[0] = OPCODE_AAI_WP;
	flash->command[1] = to >> 16;
	flash->command[2] = to >> 8;
	flash->command[3] = to;

	/* Write out most of the data here. */
	cmd_sz = CMD_ADDR_SIZE;
	for (; actual < len - 1; actual += 2) {
		t[0].len = cmd_sz;
		/* write two bytes. */
		t[1].len = 2;
		t[1].tx_buf = buf + actual;

		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - cmd_sz;
		cmd_sz = 1;
		to += 2;
	}
	write_disable(flash);
	ret = wait_till_ready(flash);
	if (ret)
		goto time_out;

	/* Write out trailing byte if it exists. */
	if (actual != len) {
		write_enable(flash);
		flash->command[0] = OPCODE_BP;
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;
		t[0].len = CMD_ADDR_SIZE;
		t[1].len = 1;
		t[1].tx_buf = buf + actual;

		spi_sync(flash->spi, &m);
		ret = wait_till_ready(flash);
		if (ret)
			goto time_out;
		*retlen += m.actual_length - CMD_ADDR_SIZE;
		write_disable(flash);
	}

time_out:
	mutex_unlock(&flash->lock);
	return ret;
}

/****************************************************************************/

static 	struct flash_status_reg __devinitdata  status_reg_list[] = {
		/*spiflash mx25l12805d*/
		{
			.jedec_id = 0xc22018,	.ext_id = 0,
			.b_wip = 0,	.b_wel = 1,	.b_bp0 = 2,	.b_bp1 = 3,
			.b_bp2 = 4,	.b_bp3 = 5,	.b_qe = 6,	.b_srp0 = 7,
			.read_sr = normal_read_sr,
			.write_sr = normal_write_sr,
		},
		/*spiflash gd25q128c*/
		{
			.jedec_id = 0xc84018,	.ext_id = 0,
			.b_wip = 0,	.b_wel = 1,	.b_bp0 = 2,	.b_bp1 = 3,
			.b_bp2 = 4,	.b_bp3 = 5,	.b_bp4 = 6,	.b_srp0 = 7,
			
			.b_srp1 = 8,.b_qe = 9,	.b_lb = 10,	.b_cmp = 14,
			.b_sus = 15,
			.read_sr = normal_read_sr,
			.write_sr = gd25q128c_write_sr,
		},
		/*normal status reg define*/
		{
			.jedec_id = 0,	.ext_id = 0,
			.b_wip = 0,	.b_wel = 1,	.b_bp0 = 2,	.b_bp1 = 3,
			.b_bp2 = 4,	.b_bp3 = 5,	.b_bp4 = 6,	.b_srp0 = 7,
			
			.b_srp1 = 8,.b_qe = 9,	.b_lb = 10,	.b_cmp = 14,
			.b_sus = 15,
			.read_sr = normal_read_sr,
			.write_sr = normal_write_sr,
		},
};



/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata ak_spiflash_supportlist [] = {

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 0, 32 * 1024, 4, SFLAG_SECT_4K, },
	{ "at25fs040",  0x1f6604, 0, 64 * 1024, 8, SFLAG_SECT_4K, },

	{ "at25df041a", 0x1f4401, 0, 64 * 1024, 8, SFLAG_SECT_4K, },
	{ "at25df641",  0x1f4800, 0, 64 * 1024, 128, SFLAG_SECT_4K, },

	{ "at26f004",   0x1f0400, 0, 64 * 1024, 8, SFLAG_SECT_4K, },
	{ "at26df081a", 0x1f4501, 0, 64 * 1024, 16, SFLAG_SECT_4K, },
	{ "at26df161a", 0x1f4601, 0, 64 * 1024, 32, SFLAG_SECT_4K, },
	{ "at26df321",  0x1f4701, 0, 64 * 1024, 64, SFLAG_SECT_4K, },

	/* Macronix */
	{ "mx25l3205d", 0xc22016, 0, 64 * 1024, 64, SFLAG_SECT_4K | SFLAG_DUAL_READ},
	{ "mx25l6405d", 0xc22017, 0, 64 * 1024, 128, SFLAG_SECT_4K | SFLAG_DUAL_READ},
	{ "mx25l12805d", 0xc22018, 0, 64 * 1024, 256, SFLAG_SECT_4K | SFLAG_DUAL_IO_READ | SFLAG_QUAD_IO_READ | SFLAG_QUAD_IO_WRITE},
	{ "mx25l12855e", 0xc22618, 0, 64 * 1024, 256, SFLAG_SECT_4K | SFLAG_DUAL_READ},

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 0, 64 * 1024, 8, },
	{ "s25sl008a", 0x010213, 0, 64 * 1024, 16, },
	{ "s25sl016a", 0x010214, 0, 64 * 1024, 32, },
	{ "s25sl032a", 0x010215, 0, 64 * 1024, 64, },
	{ "s25sl064a", 0x010216, 0, 64 * 1024, 128, },
	{ "s25sl12800", 0x012018, 0x0300, 256 * 1024, 64, },
	{ "s25sl12801", 0x012018, 0x0301, 64 * 1024, 256, },
	{ "s25fl129p0", 0x012018, 0x4d00, 256 * 1024, 64, },
	{ "s25fl129p1", 0x012018, 0x4d01, 64 * 1024, 256, },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 0, 64 * 1024, 8, SFLAG_SECT_4K, },
	{ "sst25vf080b", 0xbf258e, 0, 64 * 1024, 16, SFLAG_SECT_4K, },
	{ "sst25vf016b", 0xbf2541, 0, 64 * 1024, 32, SFLAG_SECT_4K, },
	{ "sst25vf032b", 0xbf254a, 0, 64 * 1024, 64, SFLAG_SECT_4K, },
	{ "sst25wf512",  0xbf2501, 0, 64 * 1024, 1, SFLAG_SECT_4K, },
	{ "sst25wf010",  0xbf2502, 0, 64 * 1024, 2, SFLAG_SECT_4K, },
	{ "sst25wf020",  0xbf2503, 0, 64 * 1024, 4, SFLAG_SECT_4K, },
	{ "sst25wf040",  0xbf2504, 0, 64 * 1024, 8, SFLAG_SECT_4K, },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  0, 32 * 1024, 2, },
	{ "m25p10",  0x202011,  0, 32 * 1024, 4, },
	{ "m25p20",  0x202012,  0, 64 * 1024, 4, },
	{ "m25p40",  0x202013,  0, 64 * 1024, 8, },
	{ "m25p80",         0,  0, 64 * 1024, 16, },
	{ "m25p16",  0x202015,  0, 64 * 1024, 32, },
	{ "m25p32",  0x202016,  0, 64 * 1024, 64, },
	{ "m25p64",  0x202017,  0, 64 * 1024, 128, },
	{ "m25p128", 0x202018, 0, 256 * 1024, 64, },

	{ "m45pe10", 0x204011,  0, 64 * 1024, 2, },
	{ "m45pe80", 0x204014,  0, 64 * 1024, 16, },
	{ "m45pe16", 0x204015,  0, 64 * 1024, 32, },

	{ "m25pe80", 0x208014,  0, 64 * 1024, 16, },
	{ "m25pe16", 0x208015,  0, 64 * 1024, 32, SFLAG_SECT_4K, },

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", 0xef3011, 0, 64 * 1024, 2, SFLAG_SECT_4K, },
	{ "w25x20", 0xef3012, 0, 64 * 1024, 4, SFLAG_SECT_4K, },
	{ "w25x40", 0xef3013, 0, 64 * 1024, 8, SFLAG_SECT_4K, },
	{ "w25x80", 0xef3014, 0, 64 * 1024, 16, SFLAG_SECT_4K, },
	{ "w25x16", 0xef3015, 0, 64 * 1024, 32, SFLAG_SECT_4K, },
	{ "w25x32", 0xef3016, 0, 64 * 1024, 64, SFLAG_SECT_4K, },
	{ "w25x64", 0xef3017, 0, 64 * 1024, 128, SFLAG_SECT_4K, },
	
	{ "w25q32", 0xef4016, 0, 32 * 1024, 128, SFLAG_SECT_4K|SFLAG_COM_STATUS2|SFLAG_DUAL_READ|SFLAG_QUAD_READ|SFLAG_QUAD_WRITE, },
	{ "w25q64", 0xef4017, 0, 64 * 1024, 128, SFLAG_SECT_4K|SFLAG_COM_STATUS2|SFLAG_DUAL_READ|SFLAG_QUAD_READ|SFLAG_QUAD_WRITE, },
	{ "w25q128", 0xef4018, 0, 64 * 1024, 256, SFLAG_SECT_4K|SFLAG_COM_STATUS2|SFLAG_DUAL_READ|SFLAG_QUAD_READ|SFLAG_QUAD_WRITE, },

	/* GigaDevice -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "gd25q64", 0xc84017, 0, 64 * 1024, 128, SFLAG_SECT_4K|SFLAG_COM_STATUS2|SFLAG_DUAL_READ|SFLAG_QUAD_READ|SFLAG_DUAL_IO_READ|SFLAG_QUAD_IO_READ|SFLAG_QUAD_WRITE , },
	{ "gd25q128", 0xc84018, 0, 64 * 1024, 256, SFLAG_SECT_4K|SFLAG_COM_STATUS2|SFLAG_DUAL_READ|SFLAG_QUAD_READ|SFLAG_DUAL_IO_READ|SFLAG_QUAD_IO_READ|SFLAG_QUAD_WRITE ,},
};

T_U32 ak_fha_erase_callback(T_U32 chip_num,  T_U32 startpage)
{
	struct erase_info einfo;
	memset(&einfo, 0, sizeof(struct erase_info));
	einfo.addr = startpage *  FLASH_PAGESIZE;
	einfo.len = ak_mtd_info->erasesize;
	einfo.mtd = ak_mtd_info;
	
	if(ak_spiflash_erase(ak_mtd_info, &einfo) == 0)
	{
		return FHA_SUCCESS;
	}
	else
	{
		printk("***erase failed\n");
		return FHA_FAIL;
	}
}

T_U32 ak_fha_write_callback(T_U32 chip_num, T_U32 page_num, const T_U8 *data,
		T_U32 data_len, T_U8 *oob, T_U32 oob_len, T_U32 eDataType)
{
	int ret;
	ssize_t retlen;
	loff_t to = page_num * FLASH_PAGESIZE;
	ret = ak_spiflash_write(ak_mtd_info, to, data_len * FLASH_PAGESIZE, &retlen, data);
	if(ret)
	{
		printk("%s:%d\n", __func__, __LINE__);
		return FHA_FAIL;
	}
	return FHA_SUCCESS;
}

T_U32 ak_fha_read_callback(T_U32 chip_num, T_U32 page_num, T_U8 *data,
		T_U32 data_len, T_U8 *oob, T_U32 oob_len, T_U32 eDataType)
{
	int ret;
	ssize_t retlen;
	loff_t from = page_num * FLASH_PAGESIZE;

	ret = ak_spiflash_read(ak_mtd_info, from, data_len * FLASH_PAGESIZE, &retlen, data);
	if (ret) {
		printk("%s:%d\n", __func__, __LINE__);
		return FHA_FAIL;
	}

	return FHA_SUCCESS;
}

static T_VOID *fha_ram_alloc(T_U32 size)
{
	return kmalloc(size, GFP_KERNEL);
}

static T_VOID *fha_ram_free(void *point)
{
	kfree(point);
	return NULL;
}

static T_S32 fha_print(T_pCSTR fmt, ...)
{
	va_list args;
	int r;
    
	va_start(args, fmt);
    	vprintk("FHA:",args);
	r = vprintk(fmt, args);
	va_end(args);

	return r;
}

int ak_fha_init(void)
{
	int ret = 0;
	T_PFHA_INIT_INFO pInit_info = NULL;
	T_PFHA_LIB_CALLBACK pCallback = NULL;
	T_SPI_BURN_INIT_INFO spi_info;

	pInit_info = kmalloc(sizeof(T_FHA_INIT_INFO), GFP_KERNEL);
	if (!pInit_info) {
		printk("allocate memory for pInit_info failed\n");
		return -ENOMEM;
	}

	pInit_info->nChipCnt = 0;
	pInit_info->nBlockStep = 1;
	pInit_info->eAKChip = FHA_CHIP_SET_TYPE;
	pInit_info->ePlatform = PLAT_LINUX;
	pInit_info->eMedium = MEDIUM_SPIFLASH;
	pInit_info->eMode = MODE_NEWBURN;

	pCallback = kmalloc(sizeof(T_FHA_LIB_CALLBACK), GFP_KERNEL);
	if (!pCallback) {
		printk("allocate memory for pCallback failed\n");
		ret = -ENOMEM;
		goto err_out;
	}
	
	pCallback->Erase = ak_fha_erase_callback;
	pCallback->Write = (FHA_Write)ak_fha_write_callback;
	pCallback->Read = (FHA_Read)ak_fha_read_callback;
	pCallback->RamAlloc = fha_ram_alloc;
	pCallback->RamFree = fha_ram_free;
	pCallback->MemCmp = (FHA_MemCmp)memcmp;
	pCallback->MemSet = (FHA_MemSet)memset;
	pCallback->MemCpy = (FHA_MemCpy)memcpy;
	pCallback->Printf = (FHA_Printf)fha_print;

	/* Yea, PagePerBlock=16 in producer_all, 
	 * when SPI flash didn`t suport 4k sector erase,
	 * all will dead, Why can be forbear of this big BUG ? */
	spi_info.BinPageStart = SPI_FLASH_BIN_PAGE_START;
	spi_info.PageSize = 256;
	spi_info.PagesPerBlock = ak_mtd_info->erasesize / 256;

	ret = FHA_mount(pInit_info, pCallback, &spi_info);
	if (ret == FHA_FAIL) {
		printk("FHA_mount failed\n");
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	kfree(pCallback);
err_out:
	kfree(pInit_info);
	return ret;
}

int ak_fha_init_for_update(int n)
{
	int ret = 0;
	T_PFHA_INIT_INFO pInit_info = NULL;
	T_PFHA_LIB_CALLBACK pCallback = NULL;
	T_SPI_BURN_INIT_INFO spi_info;

	pInit_info = kmalloc(sizeof(T_FHA_INIT_INFO), GFP_KERNEL);
	if (!pInit_info) {
		printk("allocate memory for pInit_info failed\n");
		return -ENOMEM;
	}

	pInit_info->nChipCnt = 0;
	pInit_info->nBlockStep = 1;
	pInit_info->eAKChip = FHA_CHIP_SET_TYPE;
	pInit_info->ePlatform = PLAT_LINUX;
	pInit_info->eMedium = MEDIUM_SPIFLASH;
	pInit_info->eMode = MODE_NEWBURN;

	pCallback = kmalloc(sizeof(T_FHA_LIB_CALLBACK), GFP_KERNEL);
	if (!pCallback) {
		printk("allocate memory for pCallback failed\n");
		ret = -ENOMEM;
		goto err_out;
	}
	
	pCallback->Erase = ak_fha_erase_callback;
	pCallback->Write = (FHA_Write)ak_fha_write_callback;
	pCallback->Read = (FHA_Read)ak_fha_read_callback;
	pCallback->RamAlloc = fha_ram_alloc;
	pCallback->RamFree = fha_ram_free;
	pCallback->MemCmp = (FHA_MemCmp)memcmp;
	pCallback->MemSet = (FHA_MemSet)memset;
	pCallback->MemCpy = (FHA_MemCpy)memcpy;
	pCallback->Printf = (FHA_Printf)fha_print;

	/* Yea, PagePerBlock=16 in producer_all, 
	 * when SPI flash didn`t suport 4k sector erase,
	 * all will dead, Why can be forbear of this big BUG ? */
	spi_info.BinPageStart = SPI_FLASH_BIN_PAGE_START;
	spi_info.PageSize = 256;
	spi_info.PagesPerBlock = ak_mtd_info->erasesize / 256;

	ret = FHA_burn_init(pInit_info, pCallback, &spi_info);
	if (ret == FHA_FAIL) {
		printk("FHA_mount failed\n");
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	kfree(pCallback);
err_out:
	kfree(pInit_info);
	return ret;
}

static int ak_mount_partitions(struct spi_device *spi)
{
	int i, ret;
	unsigned long nr_parts;
	unsigned char *buf;
	struct partitions *parts = NULL;
	struct mtd_partition *mtd_part;
	struct mtd_part_parser_data	ppdata;

	ret = ak_fha_init();
	if (ret) {
		printk("Init FHA lib failed\n");
		goto err_out;
	}

	buf = kzalloc(FLASH_PAGESIZE, GFP_KERNEL);
	if (!buf) {
		printk("allocate memory for page buffer failed\n");
		ret = -ENOMEM;
		goto err_out;
	}

	ret = FHA_get_fs_part(buf, FLASH_PAGESIZE);
	if (ret == FHA_FAIL) {
		printk("get partition info failed\n");
		ret = !ret;
		goto no_parts;
	}

	nr_parts = *(unsigned long *)buf;
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. */
	printk("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 5) {
		printk("partition count invalid\n");
		ret = -EINVAL;
		goto no_parts;
	}

	mtd_part = kzalloc(sizeof(struct mtd_partition) * nr_parts, GFP_KERNEL);
	if (!mtd_part) {
		printk("allocate memory for mtd_partition failed\n");
		ret = -ENOMEM;
		goto no_parts;
	}

	parts = (struct partitions *)(&buf[sizeof(unsigned long)]);
	for (i = 0; i < nr_parts; i++) {
		mtd_part[i].name = kzalloc(MTD_PART_NAME_LEN, GFP_KERNEL);
		memcpy(mtd_part[i].name, parts[i].name, MTD_PART_NAME_LEN);
		mtd_part[i].size = parts[i].size;
		mtd_part[i].offset = parts[i].offset;
		mtd_part[i].mask_flags = parts[i].mask_flags;
		printk("mtd_part[%d]:\nname = %s\nsize = 0x%llx\noffset = 0x%llx\nmask_flags = 0x%x\n\n",
				i, mtd_part[i].name, mtd_part[i].size, mtd_part[i].offset, mtd_part[i].mask_flags);
	}
	ppdata.of_node = spi->dev.of_node;

	ret = mtd_device_parse_register(ak_mtd_info, NULL, &ppdata, 
			(const struct mtd_partition *)mtd_part, nr_parts);
	if (ret) {
		printk("add mtd partition failed\n");
		goto no_parts;
	}

no_parts:
	kfree(buf);
err_out:
	return ret;
}

/**
* @brief	 jedec probe
* 
* Read the device ID and identify that it was supported or not.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd	  spi device handle.
* @return int return the device info.
*/
static struct flash_info *__devinit jedec_probe(struct spi_device *spi)
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[5];
	u32			jedec;
	u16                     ext_jedec = 0;
	struct flash_info	*info;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	tmp = spi_write_then_read(spi, &code, 1, id, 3);
	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			dev_name(&spi->dev), tmp);
		return NULL;
	}
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];
	
	printk("akspi flash ID: 0x%08x\n", jedec);

	//ext_jedec = id[3] << 8 | id[4];
	for (tmp = 0, info = ak_spiflash_supportlist;
			tmp < ARRAY_SIZE(ak_spiflash_supportlist);
			tmp++, info++) {
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return info;
		}
	}
	dev_err(&spi->dev, "jedec_probe() unrecognized JEDEC id %06x\n", jedec);
	return NULL;
}

static int ak_spiflash_init_stat_reg(struct ak_spiflash *flash)
{
	int i;
	struct flash_status_reg *sr;
	struct flash_info *info = &flash->info;

	for(i=0, sr=status_reg_list; i<ARRAY_SIZE(status_reg_list); i++, sr++) {
		if (sr->jedec_id == info->jedec_id) {
			if (info->ext_id != 0 && info->ext_id != sr->ext_id)
				continue;
			flash->stat_reg = *sr;
			return 0;
		}
	}
	
	flash->stat_reg = status_reg_list[i-1];
	return 0;
}



/**
* @brief	 spi flash probe
* 
* Initial the spi flash device driver to kernel.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd	  spi device handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int __devinit ak_spiflash_probe(struct spi_device *spi)
{
	struct flash_platform_data	*data;
	struct ak_spiflash		*flash;
	struct flash_info		*info;
	unsigned			i, ret = 0;

	printk("ak spiflash probe enter.\n");
	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
	data = spi->dev.platform_data;
	if (data && data->type) {
		for (i = 0, info = ak_spiflash_supportlist;
				i < ARRAY_SIZE(ak_spiflash_supportlist);
				i++, info++) {
			if (strcmp(data->type, info->name) == 0)
				break;
		}

		/* unrecognized chip? */
		if (i == ARRAY_SIZE(ak_spiflash_supportlist)) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: unrecognized id %s\n",
					dev_name(&spi->dev), data->type);
			info = NULL;

		/* recognized; is that chip really what's there? */
		} else if (info->jedec_id) {
			struct flash_info	*chip = jedec_probe(spi);

			if (!chip || chip != info) {
				dev_warn(&spi->dev, "found %s, expected %s\n",
						chip ? chip->name : "UNKNOWN",
						info->name);
				info = NULL;
			}
		}
	} else
		info = jedec_probe(spi);

	if (!info)
		return -ENODEV;

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return -ENOMEM;

#ifdef SPIFLASH_USE_MTD_BLOCK_LAYER
	/*pre-allocation buffer use for spi flash data transfer.*/
	flash->buf = kzalloc(FLASH_BUF_SIZE, GFP_KERNEL);
	if (!flash->buf) {
		printk("Allocate buf for spi page failed\n");
		kfree(flash);
		return -ENOMEM;
	}
#endif

	ak_mtd_info = &flash->mtd;

	flash->spi = spi;
	flash->info = *info;
	mutex_init(&flash->lock);
	dev_set_drvdata(&spi->dev, flash);
	
	flash->bus_width = data->bus_width;

	/*
	 * Atmel serial flash tend to power up
	 * with the software protection bits set
	 */

	if (info->jedec_id >> 16 == 0x1f) {
		write_enable(flash);
		write_sr(flash, 0);
	}
	
	if (data && data->name)
		flash->mtd.name = data->name;
	else
		flash->mtd.name = dev_name(&spi->dev);

	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = FLASH_PAGESIZE;
	flash->mtd.flags = MTD_WRITEABLE;
	flash->mtd.size = info->sector_size * info->n_sectors;
	flash->mtd._erase = ak_spiflash_erase;
	flash->mtd._read = ak_spiflash_read;
	flash->mtd.get_device_id = ak_spiflash_get_devid;
	printk("%s, info->sector_size = %d, info->n_sectors = %d\n", info->name, info->sector_size, info->n_sectors);
	//printk("flash->mtd.size = %x, %ld\n", flash->mtd.size, flash->mtd.size);
	
	/* sst flash chips use AAI word program */
	if (info->jedec_id >> 16 == 0xbf)
		flash->mtd._write = sst_write;
	else
		flash->mtd._write = ak_spiflash_write;

	/* prefer "small sector" erase if possible */
	if (info->flags & SFLAG_SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	ak_spiflash_init_stat_reg(flash);
	ak_spiflash_cfg_quad_mode(flash);
	init_spiflash_rw_info(flash);

	flash->mtd.dev.parent = &spi->dev;

	dev_info(&spi->dev, "%s (%lld Kbytes)\n", info->name,
			(long long)flash->mtd.size >> 10);

	DEBUG(MTD_DEBUG_LEVEL0,
		"mtd .name = %s, .size = 0x%llx (%lldMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		(long long)flash->mtd.size, (long long)(flash->mtd.size >> 20),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL0,
				"mtd.eraseregions[%d] = { .offset = 0x%llx, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, (long long)flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);


	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	ret = mtd_device_parse_register(ak_mtd_info, NULL, NULL, NULL, 0);
	if (ret) {
		printk("Add root MTD device failed\n");
		kfree(flash->buf);
		kfree(flash);
		return -EINVAL;
	}
	ret = ak_mount_partitions(spi);
	if (ret)
		printk("Add MTD partitions failed\n");

    printk("Init AK SPI Flash finish.\n"); 

	return 0;
}

/**
* @brief	  spi flash remove
* 
* Remove the spi flash device driver from kernel.
* @author SheShaohua
* @date 2012-03-20
* @param[in] mtd	   spi device handle.
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int __devexit ak_spiflash_remove(struct spi_device *spi)
{
	struct ak_spiflash	*flash = dev_get_drvdata(&spi->dev);
	int		status;
	
	status = mtd_device_unregister(&flash->mtd);
	
	if (status == 0) {
		kfree(flash->buf);
		kfree(flash);
	}
	return 0;
}


static struct spi_driver ak_spiflash_driver = {
	.driver = {
		.name	= "ak-spiflash",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= ak_spiflash_probe,
	.remove	= __devexit_p(ak_spiflash_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

/**
* @brief spi flash device init
* 
*  Moudle initial.
* @author SheShaohua
* @date 2012-03-20
* @return int return write success or failed
* @retval returns zero on success
* @retval return a non-zero error code if failed
*/
static int __init ak_spiflash_init(void)
{
    printk("Start to init Anyka SPI Flash...\n");
	return spi_register_driver(&ak_spiflash_driver);
}


/**
* @brief spi flash device exit
* 
*  Moudle exit.
* @author SheShaohua
* @date 2012-03-20
* @return None
*/
static void __exit ak_spiflash_exit(void)
{
	spi_unregister_driver(&ak_spiflash_driver);
}


module_init(ak_spiflash_init);
module_exit(ak_spiflash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("She Shaohua");
MODULE_DESCRIPTION("MTD SPI driver for Anyka spiflash chips");
