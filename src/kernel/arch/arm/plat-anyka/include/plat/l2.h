/*
 * linux/arch/arm/plat-anyka/include/plat/l2.h
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_L2_H
#define __ASM_ARCH_L2_H

#include <mach/regs-l2.h>
#include <mach/l2cache.h> 

#define L2_DEBUG		1
//#undef L2_DEBUG

#undef REG32
#define REG32(_reg)		(*(volatile unsigned long *)(_reg))

#undef REG16 
#define REG16(_reg)		(*(volatile unsigned short *)(_reg))

/*
 * ANYKA L2 Control Register List and Bit map definition
 * TODO: Add all register bit maps and move all to map.h in the future.
*/
#define L2_DMA_REQ_BUF_START			24
#define L2_DMA_REQ_BUF_REQ_MASK			(0xFFFF << 16)
#define L2_DMA_REQ_UART_BUF_REQ_START	16
#define L2_DMA_REQ_FRAC_DMA_LEN_START	10
#define L2_DMA_REQ_FRAC_DMA_LEN_MASK	(0x3F << L2_DMA_REQ_FRAC_DMA_LEN_START)
#define L2_DMA_REQ_FRAC_DMA_REQ			(1 << 9)
#define L2_DMA_REQ_FRAC_DMA_DIR_WR		(1 << 8)
#define L2_DMA_REQ_FRAC_DMA_L2_ADDR_START	1
#define L2_DMA_REQ_FRAC_DMA_L2_ADDR_MASK	(0x7F << L2_DMA_REQ_FRAC_DMA_L2_ADDR_START)
#define L2_DMA_REQ_EN					(1 << 0)

#define L2_FRAC_DMA_AHB_FLAG_EN			(1 << 29)
#define L2_FRAC_DMA_LDMA_FLAG_EN		(1 << 28)

#define L2_FRAC_DMA_LOW_ADDR_MASK		(0xFFFFFFF << 0)

#define L2_COMMON_BUF_CFG_BUF7_CLR		(1 << 31)
#define L2_COMMON_BUF_CFG_BUF6_CLR		(1 << 30)
#define L2_COMMON_BUF_CFG_BUF5_CLR		(1 << 29)
#define L2_COMMON_BUF_CFG_BUF4_CLR		(1 << 28)
#define L2_COMMON_BUF_CFG_BUF3_CLR		(1 << 27)
#define L2_COMMON_BUF_CFG_BUF2_CLR		(1 << 26)
#define L2_COMMON_BUF_CFG_BUF1_CLR		(1 << 25)
#define L2_COMMON_BUF_CFG_BUF0_CLR		(1 << 24)
#define L2_COMMON_BUF_CFG_BUF_CLR_START	24
#define L2_COMMON_BUF_CFG_BUF0_7_CLR_MASK	(0xFF << L2_COMMON_BUF_CFG_BUF_START)
#define L2_COMMON_BUF_CFG_BUF_VLD_START	16
#define L2_COMMON_BUF_CFG_BUF0_7_VLD_MASK	(0xFF << L2_COMMON_BUF_CFG_BUF_VLD_START
#define L2_COMMON_BUF_CFG_BUF_DMA_VLD_START	0
#define L2_COMMON_BUF_CFG_BUF_DIR_START	8

#define L2_UART_BUF_CFG_BUF_START		16

#define L2_UART_BUF_CFG_UART_EN_MASK	(0xF << 28)
#define L2_UART_BUF_CFG_UART_CLR_MASK	(0xFF << 16)
#define L2_UART_BUF_CFG_CPU_BUF_SEL_EN	(1 << 3)
#define L2_UART_BUF_CFG_CPU_BUF_NUM_START	4
#define L2_UART_BUF_CFG_CPU_BUF_NUM_MASK	(0xF << L2_UART_BUF_CFG_CPU_BUF_NUM_START)
#define L2_UART_BUF_CFG_CPU_BUF_SEL_START	0
#define L2_UART_BUF_CFG_CPU_BUF_SEL_MASK	(0x7 << L2_UART_BUF_CFG_CPU_BUF_SEL_START)

#define L2_DMA_INTR_ENABLE_BUF_START	9
#define L2_DMA_INTR_ENABLE_UART_BUF_START	1
#define L2_DMA_INTR_ENABLE_FRAC_INTR_EN	(1 << 0)

/*
 * ANYKA L2 buffer size(buffer 0 to buffer 7), all 512Bytes.
 * NOTE: L2 buffer 8 to 15 dedicate to UART 1 to 4 &  USB 2.0 Controller, 
 *           and the corresponding L2 buffer size could be 64, 128 and 256 Bytes.
 * See ANYKA Programmer's Guide for details (ANYKA preferably).
 */
#define L2_BUFFER_SIZE			512

/*
  * ANYKA L2 DMA waiting times in loop
  * TODO: Must be change to waiting time based on CPU frequency when frequency APIs are done.
  */
#define L2_MAX_DMA_WAIT_TIME	50 * 1000000UL

/*
 * ANYKA DMA size in bytes per transfer (Always 64Bytes)
 * L2 DMA transfer follows this definition.
 */
#define DMA_ONE_SHOT_LEN		64

/*
 * ANYKA L2 Buffer Status Multiply Ratio(64)
 * Buffer Status Register 1 & 2: The number of data = Bufn_sta * L2_BUF_STATUS_MULTIPLY_RATIO
 */
#define L2_BUF_STATUS_MULTIPLY_RATIO	64

/*
 * L2 Buffer ID Assignment:
 * 0 - 7: L2 common buffer, could be used by different peripherals
 * 8 - 15: Dedicate L2 buffer for UART
 * 16 - 18: Dedicate L2 buffer for USB
 */
#define L2_COMMON_BUFFER_NUM	8
#define L2_UART_BUFFER_NUM		4

#define L2_UART_BUFFER_INDEX	L2_COMMON_BUFFER_NUM
#define L2_USB_HOST_BUFFER_INDEX	(L2_UART_BUFFER_INDEX + L2_UART_BUFFER_NUM)


#define L2_COMMON_BUFFER_LEN	512
#define L2_UART_BUFFER_LEN		128

#define L2_COMMON_BUFFER_OFFSET		0
#define L2_UART_BUFFER_OFFSET		(L2_COMMON_BUFFER_LEN * L2_COMMON_BUFFER_NUM)

/*
 * L2 controller working clock define
 */
#define L2_CLOCK_EN				(0x1 << 9)
#define L2_CLOCK_REG			(AK_VA_SYSCTRL + 0x1C)

/*
 * AK39XX L2 device list which may use L2 memory.
 * The devices are defined according to L2 Buffer Assignement 1 & 2 register bit sequence.
 */
typedef enum {
	ADDR_USB_EP2 = 0,		/* USB 2.0 HS Controller: Endpoint 2 */
	ADDR_USB_EP3,			/* USB 2.0 HS Controller: Endpoint 3 */
	ADDR_USB_EP4,			/* USB 2.0 HS Controller: Endpoint 4 */
	ADDR_RESERVED,			/* Reserved */
	ADDR_MMC_SD,			/* MMC/SD interface */
	ADDR_SDIO,				/* SDIO interface */
	ADDR_SPI1_RX = 7,		/* Rx buffer of SPI1 Controller */
	ADDR_SPI1_TX,			/* Tx buffer of SPI1 Controller */
	ADDR_DAC,				/* DAC control module */	
	ADDR_SPI2_RX,			/* Rx buffer of SPI2 Controller */
	ADDR_SPI2_TX,			/* Tx buffer of SPI2 Controller */
	ADDR_GPS,				/* GPS interface */
	ADDR_PCM_TX,			/* Tx buffer of PCM Controller */
	ADDR_ADC,				/* ADC2 */
	ADDR_USB_EP5,			/* USB 2.0 HS Controller: Endpoint 5 */
} l2_device_t;


#define BUF_NULL					0xFF	/* Invalid L2 buffer ID */
#define L2_UART_BUF_START_ID		L2_COMMON_BUFFER_NUM	/* UART used buffer ID from 8 */

/*
 * Maximum L2 DMA status value (The value in CPU-Controlled Buffer and Buffer8 ~ Buffer15 Configuration Register)
 * The maximum DMA transfer bytes = MAX_L2_DMA_STATUS_VALUE * 64
 */
#define MAX_L2_DMA_STATUS_VALUE		0x8
#define MAX_L2_BUFFER_USED_TIMES	0xFFFF

/*
 * Data transfer direction between L2 memory and external RAM
 */
typedef enum {
	BUF2MEM = 0,	/* Data transfer from L2 buffer to external RAM */
	MEM2BUF,	/* Data transfer from external RAM to L2 buffer */
} l2_dma_transfer_direction_t;

/*
 * Callback function when L2 DMA/fraction DMA interrupt handler is done
 */
typedef void (*l2_callback_func_t)(unsigned long data);

/*
 * L2 buffer status
 */
typedef enum {
	L2_STAT_USED = 0,	/* Current L2 buffer is used by some device */
	L2_STAT_IDLE,		/* Current L2 buffer is NOT used by some device, thus could be allocated */
} l2_buffer_status_t;

/*
 * L2 buffer information
 */
typedef struct {
	u8 id;				/* L2 buffer ID (0~17) */
	l2_buffer_status_t usable;	/* L2 buffer status(used or idle) */
	u16 used_time;			/* Counter on L2 buffer used times */
} l2_buffer_info_t;

/*
 * Information on device which use L2 memory
 */
typedef struct {
	l2_device_t device;	/* Device ID */
	u8 id;				/* TODO: Remove id in the future since array index already represent buffer id */
} l2_device_info_t;

/*
 * L2 DMA usage information (including DMA/fraction DMA/external RAM/Callback function)
 */
typedef struct {
	bool dma_start;
	bool intr_enable;
	l2_dma_transfer_direction_t direction;
	void *dma_addr;
	u32  dma_op_times;
	bool need_frac;
	bool dma_frac_start;
	void *dma_frac_addr;
	u32  dma_frac_offset;
	u32  dma_frac_data_len;
	l2_callback_func_t callback_func;
	unsigned long data;
} l2_dma_info_t;

/**
 * l2_init - Initialize linux kernel L2 memory support
 */
void __init l2_init(void);

/**
 * l2_alloc - Allocate a common L2 buffer for given device
 *  @device:	Device ID which need common L2 buffer
 *  Return L2 buffer ID (0 ~ 7)
 *
 *  Only common L2 buffers(ID 0 ~ 7) could be allocated by l2_alloc.
 *  Other L2 buffers (UART/USB used) is handled by corresponding devices directly.
 */
u8 l2_alloc(l2_device_t device);

/**
 * l2_alloc_nowait - Allocate a common L2 buffer for given device
 *  @device:	Device ID which need common L2 buffer
 *  Return L2 buffer ID (0 ~ 7)
 *
 *  Only common L2 buffers(ID 0 ~ 7) could be allocated by l2_alloc_nowait.
 *  Other L2 buffers (UART/USB used) is handled by corresponding devices directly.
 *  return BUF_NULL immediately if no buf was alloc .
 */
u8 l2_alloc_nowait(l2_device_t device);

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
void l2_free(l2_device_t device);

/**
 * l2_set_dma_callback - Set callback function when L2 DMA/fraction DMA interrupt handler is done
 *  @id:		L2 buffer ID
 *  @func:	Callback function
 *  @data:	Arguments used by callback function
 *  Return true(Always)
 *  
 *  NOTE: Caller MUST guarantee that L2 buffer ID is valid. And since the callback function is called
 *  in interrupt handler, it MUST NOT call any functions which may sleep.
 */
bool l2_set_dma_callback(u8 id, l2_callback_func_t func, unsigned long data);

/**
 * l2_combuf_dma - Start data tranferring between memory and l2 common buffer in DMA mode
 *  @ram_addr:		External RAM address(Physical)
 *  @id:			L2 buffer ID involved in DMA transfer
 *  @bytes:		Data transfer size
 *  @direction:		Data transfer direction between L2 memory and external RAM 
 *  @intr_enable:		Open interrupt for this L2 buffer or not
 */
void l2_combuf_dma(unsigned long ram_addr, u8 id, unsigned int bytes, l2_dma_transfer_direction_t direction, bool intr_enable);

/**
 * l2_combuf_wait_dma_finish - Wait for L2 DMA finish
 *  @id:	L2 buffer ID involved in DMA transfer
 *  Return true: DMA transfer finished successfully.
 *            false: DMA transfer failed.
 *  NOTE: DMA transfer is started by l2_combuf_dma.
 */
bool l2_combuf_wait_dma_finish(u8 id);

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
void l2_combuf_cpu(unsigned long ram_addr, u8 id, unsigned int bytes, l2_dma_transfer_direction_t direction);

/**
 * l2_get_status - Get L2 buffer status
 *  @id:	L2 buffer ID
 */
u8 l2_get_status(u8 id);

/**
 * l2_clr_status - Clear L2 buffer status
 *  @id:	L2 buffer ID
 */
void l2_clr_status(u8 id);

/**
 * l2_set_status - Clear L2 buffer status
 *  @id:	L2 buffer ID
 *  @status:	Status to be set (0 ~ 8)
 */
void l2_set_status(u8 id, u8 status);

static inline void l2_enable_clock(int enable)
{
	if (enable)
		REG32(L2_CLOCK_REG) &= ~L2_CLOCK_EN;
	else
		REG32(L2_CLOCK_REG) |= L2_CLOCK_EN;
	return;
}

#ifdef L2_DEBUG
#define L2_PRINT_FUNCLINES() do { printk("%s(): line: %d\n", __func__, __LINE__); } while (0)

static inline void l2_dump_registers(void)
{
	printk("ANYKA L2 Register Dumping Begin:\n");

	printk("  rL2_DMAREQ(C080)        = 0x%08X, rL2_FRACDMAADDR(C084)     = 0x%0X\n",
		(unsigned int)rL2_DMAREQ, (unsigned int)rL2_FRACDMAADDR);
	printk("  rL2_CONBUF0_7(C088) = 0x%08X, rL2_CONBUF8_15(C08C) = 0x%0X\n",
		(unsigned int)rL2_CONBUF0_7, (unsigned int)rL2_CONBUF8_15);
	printk("  rL2_BUFASSIGN1(C090)    = 0x%08X, rL2_BUFINTEN(C09C)  = 0x%0X\n",
		(unsigned int)rL2_BUFASSIGN1, (unsigned int)rL2_BUFINTEN);
	printk("  rL2_BUFSTAT1(C0A0)      = 0x%08X, rL2_BUFSTAT2(C0A8)    = 0x%0X\n",
		(unsigned int)rL2_BUFSTAT1, (unsigned int)rL2_BUFSTAT2);

	printk("ANYKA L2 Register Dumping End.\n");
}

static inline void l2_print_array(const char *name, unsigned char *array, int len)
{
	int i;

	printk("%s[%d] = {\n ", name, len);
	for (i = 0; i < len; i++) {
		printk(" 0x%02X,", array[i]);
		if (i % 16 == 15)
			printk("\n ");
	}
	printk("};\n");

}

#else
#define L2_PRINT_FUNCLINES() do { } while (0)

static inline void l2_dump_registers(void)
{
}
static inline void l2_print_array(const char *name, unsigned int *array, int len)
{
}
#endif

#endif	/* __ASM_ARCH_L2_H */

