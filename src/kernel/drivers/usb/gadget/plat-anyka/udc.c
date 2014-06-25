/*
 * akudc_udc -- driver for anyka USB peripheral controller
 * Features
 * The USB 2.0 HS OTG has following features:
 *      â€?  compliant with USB Specification Version 2.0 (HS) and On-The-Go supplement to
 *	  the USB 2.0 specification
 *      â€?  operating as the host in point-to-point communications with another USB function
 *	  or as a function controller for a USB peripheral
 *      â€?  supporting UTMI+ Level 2 Transceiver Interface
 *      â€?  4 Transmit/Receive endpoints in addition to Endpoint 0
 *      â€?  3 DMA channels
 * AUTHOR ANYKA Zhang Jingyuan
 * 09-11-14 10:45:03
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/dma-mapping.h>

#include <plat/l2.h>
#include <plat-anyka/udc.h>
#include <plat-anyka/notify.h>
#include <mach/reg.h>
#include <mach/reset.h>
#include <mach/l2cache.h>

#define	DRIVER_VERSION	"30-May-2013"
#define	DRIVER_DESC	"ANYKA AK39 USB Device Controller driver"

#if 0
#define dbg(fmt, arg...) printk("%s(%d): " fmt "\n", __func__, __LINE__, ##arg)
#else
#define dbg(fmt, arg...)
#endif

static const char ep0name[] = "ep0";
static const char driver_name[] = "ak-hsudc";

#define udc_readb(reg)			__raw_readb(udc->baseaddr + (reg))
#define udc_readw(reg)			__raw_readw(udc->baseaddr + (reg))
#define udc_readl(reg)			__raw_readl(udc->baseaddr + (reg))
#define udc_writeb(reg, val) 	__raw_writeb(val, udc->baseaddr + (reg))
#define udc_writew(reg, val) 	__raw_writew(val, udc->baseaddr + (reg))
#define udc_writel(reg, val) 	__raw_writel(val, udc->baseaddr + (reg))

static struct akudc_udc controller;
struct workqueue_struct *ep_wqueue; //the workqueue for non-ep0 endpoint transmission

volatile int usb_detect;

unsigned int dma_rx;
unsigned int dma_tx;
dma_addr_t phys_rx;
dma_addr_t phys_tx;

static volatile char flag = 0;
static atomic_t udc_clk = ATOMIC_INIT(0); //the clk condition for udc 
static atomic_t usb_enable_flag = ATOMIC_INIT(0); //the current status of udc

#ifdef CONFIG_USB_AKUDC_DEBUG_FS
#include <linux/seq_file.h>
#include <linux/debugfs.h>

#define akudc_udc_read(udc, reg) \
	__raw_readl((udc)->baseaddr + (reg))
#define akudc_udc_write(udc, reg, val) \
	__raw_writel((val), (udc)->baseaddr + (reg))

static const char debug_filename[] = "driver/udc";

static char *parse_linestate(unsigned long value)
{
	char *str;
	
	switch(value & USB_LINESTATE_WP) {
		case 0:
			str = "SE0";
			break;
		case 1:
			str = "'J'State";
			break;
		case 2:
			str = "'K'State";
			break;
		case 3:
			str = "SE1";
			break;
	}
	return str;
}

static int parse_state(unsigned int val1, unsigned int val2)
{
	if (val1 == val2)
		return 1;
	return 0;
}

static int udc_seq_show(struct seq_file *s, void *data)
{
	struct akudc_udc *udc = s->private;
	unsigned long flags;
	int i;
	unsigned long tmp;
	
	seq_printf(s, "%s: version %s\n", "akudc", DRIVER_VERSION);
	
	local_irq_save(flags);
	tmp = __raw_readl(USB_OP_MOD_REG);
	seq_printf(s, "usb opmod control:0x%08x, linestate:%s,%s,%s,%s,%s\n\n", tmp,
		parse_linestate(tmp),
		(tmp & USB_DP_PU_EN)	? " DP en pullup" : " DP dis pullup",
		parse_state(((tmp & USB_ID_CFG) >> 12), USB_ID_CLIENT) ? " slave" : " host",
		parse_state(((tmp & USB_PHY_CFG) >> 6), USB_PHY_CLIENT) ? " slave" : " host",
		(tmp & USB_SUSPEND_EN)	? "en suspend controller & transceiver" : " en suspend transceiver");

	tmp = akudc_udc_read(udc, USB_POWER_CTRL);
	seq_printf(s, "usb power control:0x%08x %s,%s,%s,%s,%s,%s\n\n", tmp,
		(tmp & USB_ISO_UPDATE)	? " ISO" : " invalid",
		(tmp & USB_HSPEED_EN)	? " Hspeed mode" : " Fspeed mode",
		(tmp & USB_HSPEED_MODE)	? " HSmode success" : "",
		(tmp & USB_RESUME_EN)	? " en resume" : " invalid",
		(tmp & USB_SUSPEND_MODE)? " en suspend controller & transceiver" : "",
		(tmp & USB_SUSPENDM_EN)	? " en suspendm" : " invalid");

	tmp = akudc_udc_read(udc, USB_FRAME_NUM);
	seq_printf(s, "frame=%d\n", tmp);

	tmp = akudc_udc_read(udc, USB_FUNCTION_ADDR);
	seq_printf(s, "faddr=0x%p\n", tmp);

	tmp = __raw_readl(AK_VA_SYSCTRL + 0x1C);
	seq_printf(s, "usb clock:%s\n", 
		(tmp & (1<<15)) ? " disable" : " enable");
	
	local_irq_restore(flags);
	return 0;
}
static int udc_debugfs_open(struct inode *inode, struct file *file)
{
	single_open(file, udc_seq_show, PDE(inode)->data);
}

static const struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.open = udc_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void create_debugfs_files(struct akudc_udc *udc)
{
	udc->pde = proc_create_data(debug_filename, S_IRUGO, NULL, &proc_ops, udc);
}

void remove_debugfs_files(struct akudc_udc *udc)
{
	if(udc->pde)
		remove_proc_entry(debug_filename, NULL);
}
#else
static inline void create_debugfs_files(struct akudc_udc *udc) {}
static inline void remove_debugfs_files(struct akudc_udc *udc) {}
#endif

static void udc_reg_writel(unsigned long __iomem *__reg, 
			unsigned long value, int bits, int index)
{
	unsigned long tmp;
	tmp = __raw_readl(__reg);
	tmp &= ~(((1 << bits)-1) << index);
	tmp |= (value << index);
	__raw_writel(tmp, __reg);
}

#if 0
static unsigned long udc_reg_readl(unsigned long __iomem *__reg, 
			int bits, int index)
{
	unsigned long tmp;
	tmp = __raw_readl(__reg);
	tmp = (tmp&(((1 << bits)-1) << index)) >> index;
	return tmp;
}
#endif

static inline int whether_enable(void)
{
	/* 
	* if all of the udc condition is 1 and it is disable, 
	* return 1 
	*/
	if ((atomic_read(&udc_clk) == 1)
		&& (atomic_read(&usb_enable_flag) == 0)) {
		
		atomic_set(&usb_enable_flag, 1);
		return 1;
	}

	return 0;
}

static inline int whether_disable(void)
{
	/* 
	* if any of the udc condition is 0 and it is enable, 
	* return 1 
	*/
	if ((atomic_read(&udc_clk) == 0) 
		&& (atomic_read(&usb_enable_flag) == 1)) {
		
		atomic_set(&usb_enable_flag, 0);	
	return 1;
	}

	return 0;
}

/* enable the ep interrupt */
static void ep_irq_enable(struct usb_ep *_ep)
{
	static struct akudc_udc *udc = &controller;

	/* enable ep1 rx */
	if (!strcmp(_ep->name, udc->ep[1].ep.name))
		//udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) | (0x1<<1));
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) | (0x1<<1));

	/* enable ep2 tx */
	if (!strcmp(_ep->name, udc->ep[2].ep.name)) 
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) | (0x1<<2));

	/* enable ep3 rx */
	if (!strcmp(_ep->name, udc->ep[3].ep.name)) 
		udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) | (0x1<<3));

	/* enable ep4 tx */
	if (!strcmp(_ep->name, udc->ep[4].ep.name)) 
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) | (0x1<<4));

	/* enable ep5 rx */
	if (!strcmp(_ep->name, udc->ep[5].ep.name)) 
		udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) | (0x1<<5));
}

/* disable the ep interrupt */
static void ep_irq_disable(struct usb_ep *_ep)
{
	static struct akudc_udc *udc = &controller;

	/* disable ep1 rx */
	if (!strcmp(_ep->name, udc->ep[1].ep.name))
		//udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) & ~(0x1<<1));
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) & ~(0x1<<1));

	/* disable ep2 tx */
	if (!strcmp(_ep->name, udc->ep[2].ep.name)) 
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) & ~(0x1<<2));

	/* disable ep3 rx */
	if (!strcmp(_ep->name, udc->ep[3].ep.name)) 
		udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) & ~(0x1<<3));

	/* disable ep4 tx */
	if (!strcmp(_ep->name, udc->ep[4].ep.name)) 
		udc_writew(USB_INTERRUPT_TX, udc_readw(USB_INTERRUPT_TX) & ~(0x1<<4));

	/* disable ep5 rx */
	if (!strcmp(_ep->name, udc->ep[5].ep.name)) 
		udc_writew(USB_INTERRUPT_RX, udc_readw(USB_INTERRUPT_RX) & ~(0x1<<5));
}


/* 
  * enable or disable the udc controller
  * @enable: if 1, enable. if 0, disable
  */
void usbcontroller_enable(int enable)
{
	struct akudc_udc *udc = &controller;
	if (enable) {
		clk_enable(udc->clk); //enable clk for udc
		/* reset usb phy and suspend transceiver, and power up(ak98) */
		udc_reg_writel(USB_OP_MOD_REG, 0x6, 3, 0);
		set_usb_as_slave();
		
		udc_writeb(USB_POWER_CTRL, 1<<5); 	// enable high speed negotiation
		udc_writeb(USB_INTERRUPT_TX, 0x1<<0);	// enable ep0 interrupt and disable other tx endpoint 
		udc_writeb(USB_INTERRUPT_RX, 0);		// disable rx endpoint inttrupt 
	} else {
		clk_disable(udc->clk); //disable clk for udc
		/* reset usb phy */
		udc_reg_writel(USB_OP_MOD_REG, 0x1, 3, 0);
		udc_writeb(USB_POWER_CTRL, 0);
		ak_soft_reset(AK_SRESET_USBHS); //reset the udc controller
	}
}

/* 
  * set the udc condition, and judge whether the udc 
  * can be enabled, if is , enable it
  */
void set_condition(atomic_t *cond) {
	atomic_set(cond, 1);
	if (whether_enable())
		usbcontroller_enable(1);
}

/* 
  * clear the udc condition, and judge whether the udc 
  * can be disabled, if is , disable it
  */
void clear_condition(atomic_t *cond) {
	atomic_set(cond, 0);
	if (whether_disable())
		usbcontroller_enable(0);
}

/*
  * enable the udc
  */

/* 
  * when the req is complete, call this function
  * @ep: which ep's request is complete
  * @req: the request which is complete
  * @status: the request's status
  */
static void done(struct akudc_ep *ep, struct akudc_request *req, int status)
{
	unsigned	 stopped = ep->stopped;

	/* delete req from ep queue */
	list_del_init(&req->queue);

	if (likely (req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	ep->stopped = 1;
	/* call complete callback before return */
	req->req.complete(&ep->ep, &req->req);
	ep->stopped = stopped;

	dbg("%s done, req.status(%d)", ep->ep.name, req->req.status);
}

/* 
  * send data to pc in ep0
  * @ep: point to ep0
  * @req: the request which is doing
  * return:  0 = still running, 1 = completed, negative = errno
  */
static int write_ep0_fifo(struct akudc_ep *ep, struct akudc_request *req)
{
	int i;
	unsigned total, count, is_last;
	struct akudc_udc *udc = ep->udc;
	unsigned char *buf;

	total = req->req.length - req->req.actual;

	if (ep->ep.maxpacket < total) { /* if this transition is not last */
		count = ep->ep.maxpacket;
		is_last = 0;
	} else { /* if this transition is last */
		count = total;
		/* 
		  * if the req zero flag not set, the "count == ep->ep.maxpacket"
		  * is the last
		  */
		is_last = (count < ep->ep.maxpacket) || !req->req.zero;
	}

	dbg("is_last(%d), count(%d), total(%d), actual(%d), length(%d)", 
			is_last, count, total, req->req.actual, req->req.length);

	udc_writeb(USB_EP_INDEX, 0);
	/* send zero packet */
	if (count == 0) {
		dbg(" count == 0 ");
		udc_writel(USB_EP0_NUM, count&0x7f);
		udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<1);
		udc->ep0state = EP0_IDLE;
		done(ep, req, 0);
		return 1;
	}

	buf = req->req.buf + req->req.actual;
	for (i = 0; i < count; i++) {
		/* write  data to the ep0 fifo */
		udc_writeb(USB_EP0_FIFO, buf[i]);
	}

	udc_writel(USB_EP0_NUM, count&0x7f); /* 7bits */ 
	if (is_last) /* if last, set  data end */
		udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<1);
	else
		udc_writeb(USB_CTRL_1, 0x1<<1);

	req->req.actual += count;
	if (is_last) { /* if last ,done the req with status 0 */
		udc->ep0state = EP0_IDLE;
		done(ep, req, 0);
	}

	ep->done = is_last;
	return is_last;
	/* return 1; */
}

/* 
  * read data from pc in ep0
  * @ep: point to ep0
  * @req: the request which is doing
  * return:  0 = still running, 1 = completed, negative = errno
  */
static int read_ep0_fifo(struct akudc_ep *ep, struct akudc_request *req)
{
	int i;
	unsigned total, count, is_last;
	struct akudc_udc *udc = ep->udc;
	unsigned char *buf;

	total = req->req.length - req->req.actual;

	if (ep->ep.maxpacket < total) {
		count = ep->ep.maxpacket;
		is_last = 0;
	} else {
		count = total;
		is_last = (count < ep->ep.maxpacket) || !req->req.zero;
	}

	dbg("is_last(%d), count(%d), total(%d), actual(%d), length(%d)", 
			is_last, count, total, req->req.actual, req->req.length);

	udc_writeb(USB_EP_INDEX, 0);
	/* read zero packet */
	if (count == 0) {
		dbg(" count == 0 ");
		udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<6);
		udc->ep0state = EP0_IDLE;
		done(ep, req, 0);
		return 1;
	}

	buf = req->req.buf + req->req.actual;
	for (i = 0; i < count; i++) {
		/* read data from ep0 fifo */
		buf[i] = udc_readb(USB_EP0_FIFO);
	}
 
	if (is_last)  /* if last, set data end */
		udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<6);
	else 
		udc_writeb(USB_CTRL_1, 0x1<<6);

	req->req.actual += count;
	if (is_last) { /* if last ,done the req with status 0 */
		udc->ep0state = EP0_IDLE;
		done(ep, req, 0);
	}

	ep->done = is_last;
	return is_last;
	/* return 1; */
}

/* the ep1 is not used */
static int write_ep1_fifo(struct akudc_ep *ep, struct akudc_request *req)
{
	struct akudc_udc *udc = ep->udc;
	unsigned total, count, is_last;
	unsigned char *buf;
	int dma = 0, i;

	total = req->req.length - req->req.actual;
	if (ep->ep.maxpacket <= total) {
		count = ep->ep.maxpacket;
		is_last = (total == ep->ep.maxpacket) && !req->req.zero;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed)
		dma = 1;
#endif
#endif
	} else {
		count = total;
		is_last = count < ep->ep.maxpacket;
		dma = 0;
	}
	dbg("total(%d), count(%d), is_last(%d)", total, count, is_last);

	if (count == 0) { /* not support command */ 
		dbg("count == 0\n");
		ep->done = 1;
		udc_writeb(USB_EP_INDEX, 1);
		udc_writeb(USB_CTRL_1, 0x1);
		return 0;
	}
	udc_writeb(USB_EP_INDEX, 1);

	buf = req->req.buf + req->req.actual;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	dbg("producer");
	if (udc->high_speed && (dma == 1)) {
		dma_tx = total - (total % ep->ep.maxpacket);
		/* map the dma buffer */
		phys_tx = dma_map_single(NULL, buf, dma_tx, DMA_TO_DEVICE);
		if (phys_tx == 0) {
			printk("tx dma_map_single error!\n");
			goto cpu;
		}

		udc_writeb(USB_CTRL_1_2, (1<<2) | (1<<4) | (1<<5) | (1<<7));
		
		//send data to l2
		l2_clr_status(ep->l2_buf_id);
		l2_combuf_dma(phys_tx, ep->l2_buf_id, dma_tx, MEM2BUF, false);
		udc_writel(USB_DMA_ADDR3, 0x72000000);
		udc_writel(USB_DMA_COUNT3, dma_tx);
		udc_writel(USB_DMA_CTRL3, (USB_ENABLE_DMA | USB_DIRECTION_TX | USB_DMA_MODE1 | USB_DMA_INT_ENABLE| (USB_EP4_INDEX<<4) | USB_DMA_BUS_MODE3));
		
		ep->done = 0;
		return 0;
	}
#endif
cpu:
#endif

	for (i = 0; i < count; i++) {
		/* send data to ep4 fifo without dma */
		udc_writeb(USB_EP1_FIFO, buf[i]);
	}

	udc_writeb(USB_CTRL_1, 0x1);

	req->req.actual += count;
	/* wait a tx complete int */  
	if (is_last)
		done(ep, req, 0);
         
	return is_last;
}

/* 
  * send data to pc in ep2
  * @ep: point to ep2
  * @req: the request which is doing
  */
static int write_ep2_fifo(struct akudc_ep *ep, struct akudc_request *req) /* ep2 */ 
{
	struct akudc_udc *udc = ep->udc;
	unsigned total, count, is_last;
	unsigned char *buf;
	int dma = 0, i;

	total = req->req.length - req->req.actual;
	if (ep->ep.maxpacket <= total) {
		count = ep->ep.maxpacket;
		is_last = (total == ep->ep.maxpacket) && !req->req.zero;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
		if (udc->high_speed)
			dma = 1;
#endif
#endif
	} else {
		count = total;
		is_last = count < ep->ep.maxpacket;
		dma = 0;
	}
	dbg("total(%d), count(%d), is_last(%d)", total, count, is_last);

	if (count == 0) { /* not support command */ 
		dbg("count == 0\n");
		ep->done = 1;
		udc_writeb(USB_EP_INDEX, 2);
		udc_writeb(USB_CTRL_1, 0x1);
		return 0;
	}
	udc_writeb(USB_EP_INDEX, 2);

	buf = req->req.buf + req->req.actual;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed && (dma == 1)) {
		dma_tx = total - (total % ep->ep.maxpacket);
		/* map the dma buffer */
		phys_tx = dma_map_single(NULL, buf, dma_tx, DMA_TO_DEVICE);
		if (phys_tx == 0) {
			printk("tx dma_map_single error!\n");
			goto cpu;
		}

		udc_writeb(USB_CTRL_1_2, (1<<2) | (1<<4) | (1<<5) | (1<<7));
		
		//send data to l2
		l2_clr_status(ep->l2_buf_id);
		l2_combuf_dma(phys_tx, ep->l2_buf_id, dma_tx, MEM2BUF, false);
		udc_writel(USB_DMA_ADDR1, 0x70000000);
		udc_writel(USB_DMA_COUNT1, dma_tx);
		udc_writel(USB_DMA_CTRL1, (USB_ENABLE_DMA | USB_DIRECTION_TX | USB_DMA_MODE1 | USB_DMA_INT_ENABLE| (USB_EP2_INDEX<<4) | USB_DMA_BUS_MODE3));
		
		ep->done = 0;
		return 0;
	}
#endif
cpu:
#endif

	for (i = 0; i < count; i++) {
		/* send data to ep2 fifo without dma */
		udc_writeb(USB_EP2_FIFO, buf[i]);		
	}
	udc_writeb(USB_CTRL_1, 0x1);

	req->req.actual += count;
	/* wait a tx complete int */
        /*
	 * if (is_last)
	 *         done(ep, req, 0);
         */

	/* return is_last; */
	ep->done = is_last;

	return 0;
}

/* 
  * read data from pc in ep3
  * @ep: point to ep3
  * @req: the request which is doing
  */
static int read_ep3_fifo(struct akudc_ep *ep, struct akudc_request *req) /* ep3 */ 
{
	struct akudc_udc *udc = ep->udc;
	unsigned char *buf;
	unsigned int csr, i;
	unsigned int count, bufferspace, is_done;

	if (flag == 1)
		return 0;

	bufferspace = req->req.length - req->req.actual;

	udc_writeb(USB_EP_INDEX, 3);
	csr = udc_readb(USB_CTRL_2);
	if ((csr & 0x1) == 0) {
		dbg("waiting bulkout data");
		return 0;
	}

	count = udc_readw(USB_EP_COUNT);
	if (count == 0) {
		dbg("USB_CTRL_2(0x%x), USB_EP_COUNT(%d), bufferspace(%d)",
				csr, count, bufferspace);
		goto stall;
	} else if (count > ep->ep.maxpacket)
		count = ep->ep.maxpacket;

	if (count > bufferspace) {
		dbg("%s buffer overflow\n", ep->ep.name);
		req->req.status = -EOVERFLOW;
		count = bufferspace;
	}
	dbg("USB_CTRL_2(0x%x), USB_EP_COUNT(%d), bufferspace(%d)", csr, count, bufferspace);

	buf = req->req.buf + req->req.actual;

	for (i = 0; i < count; i++) {
		/* read data from ep3 fifo without dma */
		buf[i] = udc_readb(USB_EP3_FIFO);
	}
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed) {
		dma_rx = bufferspace - count;
		if (dma_rx >= ep->ep.maxpacket) {
			dma_rx -= (dma_rx % ep->ep.maxpacket);
			
			buf = req->req.buf + req->req.actual + count;
			/* send data to ep2 fifo without dma */
			phys_rx = dma_map_single(NULL, buf, dma_rx, DMA_FROM_DEVICE);
			if (phys_rx == 0) {
				printk("rx dma_map_single error!\n");
				goto stall;
			}

			req->req.actual += count;
			flag = 1;
			udc_writeb(USB_CTRL_2_2, udc_readb(USB_CTRL_2_2) | (1<<3) | (1<<5) | (1<<7));
			l2_combuf_dma(phys_rx, ep->l2_buf_id, dma_rx, BUF2MEM, false);
			udc_writel(USB_DMA_ADDR2, 0x71000000);
			udc_writel(USB_DMA_COUNT2, dma_rx);
			udc_writel(USB_DMA_CTRL2, (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(USB_EP3_INDEX<<4)|USB_DMA_BUS_MODE3));
			udc_writeb(USB_EP_INDEX, 3);
			udc_writeb(USB_CTRL_2, csr & ~0x1);
			ep->done = 0;

			return 0;
		}
	}
#endif
#endif	
stall:
	udc_writeb(USB_CTRL_2, csr & ~0x1);

	req->req.actual += count;
	is_done = (count < ep->ep.maxpacket);
	if (count == bufferspace)
		is_done = 1;

	ep->done = is_done;
	if (is_done) {
		done(ep, req, 0);
	}

	return is_done;
}

/* 
  * send data to pc in ep4
  * @ep: point to ep4
  * @req: the request which is doing
  */
static int write_ep4_fifo(struct akudc_ep *ep, struct akudc_request *req) /* ep4 */ 
{
	struct akudc_udc *udc = ep->udc;
	unsigned total, count, is_last;
	unsigned char *buf;
	int dma = 0, i;

	total = req->req.length - req->req.actual;
	if (ep->ep.maxpacket <= total) {
		count = ep->ep.maxpacket;
		is_last = (total == ep->ep.maxpacket) && !req->req.zero;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed)
		dma = 1;
#endif
#endif
	} else {
		count = total;
		is_last = count < ep->ep.maxpacket;
		dma = 0;
	}
	dbg("total(%d), count(%d), is_last(%d)", total, count, is_last);

	if (count == 0) { /* not support command */ 
		dbg("count == 0\n");
		ep->done = 1;
		udc_writeb(USB_EP_INDEX, 4);
		udc_writeb(USB_CTRL_1, 0x1);
		return 0;
	}
	udc_writeb(USB_EP_INDEX, 4);

	buf = req->req.buf + req->req.actual;
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed && (dma == 1)) {
		dma_tx = total - (total % ep->ep.maxpacket);
		/* map the dma buffer */
		phys_tx = dma_map_single(NULL, buf, dma_tx, DMA_TO_DEVICE);
		if (phys_tx == 0) {
			printk("tx dma_map_single error!\n");
			goto cpu;
		}

		udc_writeb(USB_CTRL_1_2, (1<<2) | (1<<4) | (1<<5) | (1<<7));
		
		//send data to l2
		l2_clr_status(ep->l2_buf_id);
		l2_combuf_dma(phys_tx, ep->l2_buf_id, dma_tx, MEM2BUF, false);
		udc_writel(USB_DMA_ADDR3, 0x72000000);
		udc_writel(USB_DMA_COUNT3, dma_tx);
		udc_writel(USB_DMA_CTRL3, (USB_ENABLE_DMA | USB_DIRECTION_TX | USB_DMA_MODE1 | USB_DMA_INT_ENABLE| (USB_EP4_INDEX<<4) | USB_DMA_BUS_MODE3));
		
		ep->done = 0;
		return 0;
	}
#endif
cpu:
#endif

	for (i = 0; i < count; i++) {
		/* send data to ep4 fifo without dma */
		udc_writeb(USB_EP4_FIFO, buf[i]);
	}

	udc_writeb(USB_CTRL_1, 0x1);

	req->req.actual += count;
	/* wait a tx complete int */
     /*
	 * if (is_last)
	 *         done(ep, req, 0);
         */

	/* return is_last; */
	ep->done = is_last;

	return 0;
}

/* 
  * read data from pc in ep5
  * @ep: point to ep5
  * @req: the request which is doing
  */
static int read_ep5_fifo(struct akudc_ep *ep, struct akudc_request *req) /* ep5 */ 
{
	struct akudc_udc *udc = ep->udc;
	unsigned char *buf;
	unsigned int csr, i;
	unsigned int count, bufferspace, is_done;

	if (flag == 1)
		return 0;

	bufferspace = req->req.length - req->req.actual;

	udc_writeb(USB_EP_INDEX, 5);
	csr = udc_readb(USB_CTRL_2);
	if ((csr & 0x1) == 0) {
		dbg("waiting bulkout data");
		return 0;
	}

	count = udc_readw(USB_EP_COUNT);
	if (count == 0) {
		dbg("USB_CTRL_2(0x%x), USB_EP_COUNT(%d), bufferspace(%d)",
				csr, count, bufferspace);
		goto stall;
	} else if (count > ep->ep.maxpacket)
		count = ep->ep.maxpacket;

	if (count > bufferspace) {
		dbg("%s buffer overflow\n", ep->ep.name);
		req->req.status = -EOVERFLOW;
		count = bufferspace;
	}
	dbg("USB_CTRL_2(0x%x), USB_EP_COUNT(%d), bufferspace(%d)", csr, count, bufferspace);

	buf = req->req.buf + req->req.actual;

	for (i = 0; i < count; i++) {
		/* read data from ep5 fifo without dma */
		buf[i] = udc_readb(USB_EP5_FIFO);
	}
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	if (udc->high_speed) {
		dma_rx = bufferspace - count;
		if (dma_rx >= ep->ep.maxpacket) {
			dma_rx -= (dma_rx % ep->ep.maxpacket);
			
			buf = req->req.buf + req->req.actual + count;
			/* map the dma buffer */
			phys_rx = dma_map_single(NULL, buf, dma_rx, DMA_FROM_DEVICE);
			if (phys_rx == 0) {
				printk("rx dma_map_single error!\n");
				goto stall;
			}

			req->req.actual += count;
			flag = 1;
			udc_writeb(USB_CTRL_2_2, udc_readb(USB_CTRL_2_2) | (1<<3) | (1<<5) | (1<<7));
			l2_combuf_dma(phys_rx, ep->l2_buf_id, dma_rx, BUF2MEM, false);
			udc_writel(USB_DMA_ADDR4, 0x73000000);
			udc_writel(USB_DMA_COUNT4, dma_rx);
			udc_writel(USB_DMA_CTRL4, (USB_ENABLE_DMA|USB_DIRECTION_RX|USB_DMA_MODE1|USB_DMA_INT_ENABLE|(USB_EP5_INDEX<<4)|USB_DMA_BUS_MODE3));
			udc_writeb(USB_EP_INDEX, 5);
			udc_writeb(USB_CTRL_2, csr & ~0x1);
			ep->done = 0;

			return 0;
		}
	}
#endif
#endif

stall:	
	udc_writeb(USB_CTRL_2, csr & ~0x1);

	req->req.actual += count;
	is_done = (count < ep->ep.maxpacket);
	if (count == bufferspace)
		is_done = 1;

	ep->done = is_done;
	if (is_done) {
		done(ep, req, 0);
	}

	return is_done;
}


/* the funciton is not used */
static int akudc_get_frame(struct usb_gadget *gadget)
{
	dbg("");
	return 0;
}

/* the funciton is not used */
static int akudc_wakeup(struct usb_gadget *gadget)
{
	dbg("");
	return 0;
}

/* 
  * the function to enable or disable the udc  
  * @is_on: 1,enable 0,disable
  */
static int akudc_pullup(struct usb_gadget *gadget, int is_on)
{
	if (is_on) 
		set_condition(&udc_clk);
	else 
		clear_condition(&udc_clk);
	return 0;
}

/* the function is not used */
static int akudc_vbus_session(struct usb_gadget *gadget, int is_active)
{
	dbg("");
	return 0;
}

/* 
  * the function set the device power status 
  * @value: 1,selfpowered 0,powered by usb cable
  */
static int akudc_set_selfpowered(struct usb_gadget *gadget, int value)
{
	struct akudc_udc *udc = &controller;

	dbg("%d", value);
	if (value)
		udc->devstatus |= (1 << USB_DEVICE_SELF_POWERED);
	else
		udc->devstatus &= ~(1 << USB_DEVICE_SELF_POWERED);
	
	return 0;
}

static int akudc_start(struct usb_gadget_driver *driver,
		int (*bind)(struct usb_gadget *));
static int akudc_stop(struct usb_gadget_driver *driver);

/* the operation struct for gadget */
static const struct usb_gadget_ops akudc_udc_ops = {
	.get_frame		= akudc_get_frame,
	.wakeup			= akudc_wakeup,
	.set_selfpowered	= akudc_set_selfpowered,
	.vbus_session		= akudc_vbus_session,
	.pullup			= akudc_pullup,
	.start			= akudc_start,
	.stop			= akudc_stop,
};

static void ep1_work(struct work_struct *work)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = &controller;
	struct akudc_ep *ep = &udc->ep[1];
	unsigned long flags;

	local_irq_save(flags);

	/* if the queue is not empty, get the head of the queue */
	if (!list_empty(&ep->queue)){
		req = list_entry(ep->queue.next, struct akudc_request, queue);
	}

	if (req)
		req->status = write_ep1_fifo(ep, req);
	else
		dbg("something happend");
	local_irq_restore(flags);
}

static void ep2_work(struct work_struct *work)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = &controller;
	struct akudc_ep *ep = &udc->ep[2];
	unsigned long flags;

	local_irq_save(flags);

	/* if the queue is not empty, get the head of the queue */
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct akudc_request, queue);

	if (req)
		req->status = write_ep2_fifo(ep, req);
	else
		dbg("something happend");
	local_irq_restore(flags);
}

static void ep3_work(struct work_struct *work)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = &controller;
	struct akudc_ep *ep = &udc->ep[3];
	unsigned long flags;

	local_irq_save(flags);
	
	/* if the queue is not empty, get the head of the queue */
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct akudc_request, queue);

	if (req)
		req->status = read_ep3_fifo(ep, req);
	else
		dbg("something happend");
	local_irq_restore(flags);
}

static void ep4_work(struct work_struct *work)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = &controller;
	struct akudc_ep *ep = &udc->ep[4];
	unsigned long flags;
	
	local_irq_save(flags);
	
	/* if the queue is not empty, get the head of the queue */
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct akudc_request, queue);

	if (req)
		req->status = write_ep4_fifo(ep, req);
	else
		dbg("something happend");
	local_irq_restore(flags);
}

static void ep5_work(struct work_struct *work)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = &controller;
	struct akudc_ep *ep = &udc->ep[5];
	unsigned long flags;
	
	local_irq_save(flags);

	/* if the queue is not empty, get the head of the queue */
	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next, struct akudc_request, queue);

	if (req)
		req->status = read_ep5_fifo(ep, req);
	else
		dbg("something happend");
	local_irq_restore(flags);
}

/* 
  * enable the ep, include enable the irq for the ep
  * init the fifo and toggle for ep 
  * @ep: the ep which is enable
  * @desc: the ep descriptor
  */
static int akudc_ep_enable(struct usb_ep *_ep,
				const struct usb_endpoint_descriptor *desc)
{
	struct akudc_ep *ep = container_of(_ep, struct akudc_ep, ep);
	struct akudc_udc *udc = ep->udc;
	int tmp, maxpacket;
	unsigned long flags;

	if (!_ep || !ep
			|| !desc || ep->desc
			|| _ep->name == ep0name
			|| desc->bDescriptorType != USB_DT_ENDPOINT
			|| (maxpacket = usb_endpoint_maxp(desc)) == 0
			|| maxpacket > ep->maxpacket) {
		dbg("bad ep or descriptor");
		dbg("%p, %p, %p, %p", _ep, ep, desc, ep->desc);
		return -EINVAL;
	}

	if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
		dbg("bogus udcice state\n");
		return -ESHUTDOWN;
	}
	
	local_irq_save (flags);

	tmp = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	switch (tmp) {
	case USB_ENDPOINT_XFER_CONTROL:
		dbg("only one control endpoint\n");
		return -EINVAL;
	case USB_ENDPOINT_XFER_INT:
		if (maxpacket > EP1_FIFO_SIZE)
			dbg("maxpacket too large");
		break;
	case USB_ENDPOINT_XFER_BULK:
		switch (maxpacket) {
		case 8:
		case 16:
		case 32:
		case 64:
		case 512: /* for usb20 */
			_ep->maxpacket = maxpacket & 0x7ff;
			break;
		default:
			dbg("bogus maxpacket %d\n", maxpacket);
			return -EINVAL;
		}
		break;
	case USB_ENDPOINT_XFER_ISOC:
		dbg("USB_ENDPOINT_XFER_ISOC");
		break;
	}
	ep->is_in = (desc->bEndpointAddress & USB_DIR_IN) != 0;
	ep->is_iso = (tmp == USB_ENDPOINT_XFER_ISOC);

	ep->stopped = 0;
	ep->desc = (struct usb_endpoint_descriptor *)desc;

	dbg("%s, maxpacket(%d), desc->bEndpointAddress (0x%x) is_in(%d)",
		       	_ep->name, maxpacket, desc->bEndpointAddress, ep->is_in);
	
	if (!strcmp(_ep->name, udc->ep[1].ep.name)) { /* ep1 -- int */ 
		dbg("");
		udc_writeb(USB_EP_INDEX, 1);
		udc_writeb(USB_CTRL_1, (1<<3) | (1<<6)); // flush fifo and clear toggle
		udc_writeb(USB_CTRL_1_2, 1<<5); //set tx
		udc_writew(USB_TX_MAX, 64); //set maxpacket size
	} else if (!strcmp(_ep->name, udc->ep[2].ep.name)) { /* ep2 -- tx */ 
		dbg("");
		udc_writeb(USB_EP_INDEX, 2);
		udc_writeb(USB_CTRL_1, (1<<3) | (1<<6)); // flush fifo and clear toggle
		udc_writeb(USB_CTRL_1_2, 1<<5); //set tx
		udc_writew(USB_TX_MAX, 512); //set maxpacket size
	} else if (!strcmp(_ep->name, udc->ep[3].ep.name)){ /* ep3 -- rx */ 
		udc_writeb(USB_EP_INDEX, 3);
		udc_writeb(USB_CTRL_1, 1<<6); // clear tx toggle
		udc_writeb(USB_CTRL_1_2, 0); // set rx
		udc_writew(USB_RX_MAX, 512); // set maxpacket size
		udc_writeb(USB_CTRL_2, udc_readb(USB_CTRL_2) & ~(0x1));
		udc_writeb(USB_CTRL_2, (1<<4) | (1<<7)); // flush fifo and clear rx toggle
		udc_writeb(USB_CTRL_2_2, 0); //enable rx endpint
	} else if (!strcmp(_ep->name, udc->ep[4].ep.name)) { /* ep4 -- tx */ 
		udc_writeb(USB_EP_INDEX, 4); 
		udc_writeb(USB_CTRL_1, (1<<3) | (1<<6)); // flush fifo and clear toggle
		udc_writeb(USB_CTRL_1_2, 1<<5); //set tx
		udc_writew(USB_TX_MAX, 512); //set maxpacket size
	} else if (!strcmp(_ep->name, udc->ep[5].ep.name)){ /* ep5 -- rx */ 
		udc_writeb(USB_EP_INDEX, 5);
		udc_writeb(USB_CTRL_1, 1<<6); // clear tx toggle
		udc_writeb(USB_CTRL_1_2, 0); // set rx
		udc_writew(USB_RX_MAX, 512); //set maxpacket size
		udc_writeb(USB_CTRL_2, udc_readb(USB_CTRL_2) & ~(0x1));
		udc_writeb(USB_CTRL_2, (1<<4) | (1<<7)); // flush fifo and clear rx toggle
		udc_writeb(USB_CTRL_2_2, 0); //enable rx endpint
	} else {
		printk("Invalid ep");
		return -EINVAL;
	}

	ep_irq_enable(_ep);
	local_irq_restore (flags);

	return 0;
}

/* 
  * disable the ep, include disable the irq for the ep
  * @ep: the ep which is disable
  */
static int akudc_ep_disable (struct usb_ep * _ep)
{
	struct akudc_ep *ep = container_of(_ep, struct akudc_ep, ep);
	struct akudc_request *req;
	unsigned long flags;

	if (!_ep || !ep->desc) {
		dbg("%s not enabled\n",
			_ep ? ep->ep.name : NULL);
		return -EINVAL;
	}
	
	local_irq_save(flags);
	dbg("%s", _ep->name);
	ep->desc = NULL;
	ep->stopped = 1;

	/* 
	  * delete every req in the endpoint queue 
	  * dont it by status -ESHUTDOWN
	  */
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct akudc_request, queue);
		done(ep, req, -ESHUTDOWN);
	}
	ep_irq_disable(_ep);
	local_irq_restore(flags);
	
	return 0;
}

/*
  * alloc the request for function driver
  * @ep: the ep which the request deal with
  */
static struct usb_request *
	akudc_ep_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct akudc_request *req;

	dbg("%s", _ep->name);
	req = kzalloc(sizeof (struct akudc_request), gfp_flags);
	if (!req)
		return NULL;

	INIT_LIST_HEAD(&req->queue);
	return &req->req;
}

/*
  * free the request for function driver
  * @ep: the ep which the request deal with
  */
static void akudc_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct akudc_request *req;

	dbg("%s", _ep->name);
	req = container_of(_req, struct akudc_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/*
  * add the request to the ep request queue
  * @ep: the ep which the request deal with
  */
static int akudc_ep_queue(struct usb_ep *_ep,
			struct usb_request *_req, gfp_t gfp_flags)
{
	struct akudc_request	*req;
	struct akudc_ep	*ep;
	struct akudc_udc	*udc;
	int			status = 0;
	unsigned long flags;

	req = container_of(_req, struct akudc_request, req);
	ep = container_of(_ep, struct akudc_ep, ep);
	
	if (!_ep || (!ep->desc && ep->ep.name != ep0name)) {
		dbg("invalid ep\n");
		return -EINVAL;
	}

	udc = ep->udc;
	if (!udc || !udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN) {
		printk("invalid device\n");
		return -EINVAL;
	}

	local_irq_save(flags);

	if (!_req || !_req->complete
			|| !_req->buf || !list_empty(&req->queue)) {
		printk("%s invalid request\n", _ep->name);
		local_irq_restore(flags);
		return -EINVAL;
	}

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	dbg("%s queue is_in(%d)", ep->ep.name, ep->is_in);

	/* 
	  * if the queue is empty and the ep is not stopped, 
	  * do the request 
	  */
	if (list_empty(&ep->queue) && !ep->stopped) {
		if (ep->ep.name == ep0name) {
			udc_writeb(USB_EP_INDEX, 0);
			
			switch (udc->ep0state) {
			case EP0_IN_DATA_PHASE:
				if ((udc_readb(USB_CTRL_1) & (1 << 1)) == 0
						&& write_ep0_fifo(ep, req)) {
					udc->ep0state = EP0_IDLE;
					req = NULL;
				}
				break;

			case EP0_OUT_DATA_PHASE:
				if ((!_req->length) ||
					((udc_readb(USB_CTRL_1) & (1 << 0))
					&& read_ep0_fifo(ep, req))) {
					udc->ep0state = EP0_IDLE;
					req = NULL;
				}
				break;
				
			default:
				local_irq_restore(flags);
				return -EL2HLT;
			}
			if (req)
				list_add_tail(&req->queue, &ep->queue);
		} else {
			list_add_tail(&req->queue, &ep->queue);
			if (!strcmp(_ep->name, udc->ep[1].ep.name)) { /* ep1 */ 
				dbg("ep1");
				//status = write_ep1_fifo(ep, req);
				udc_writeb(USB_EP_INDEX, 1);
				if ((udc_readb(USB_CTRL_1) & 1) == 0)
					queue_work(ep_wqueue, &ep->work);
			} else if (!strcmp(_ep->name, udc->ep[2].ep.name)) { /* ep2 */ 			
				udc_writeb(USB_EP_INDEX, 2);
				if ((udc_readb(USB_CTRL_1) & 1) == 0)
					queue_work(ep_wqueue, &ep->work);
			} else if (!strcmp(_ep->name, udc->ep[3].ep.name)){ /* ep3 */ 
				udc_writeb(USB_EP_INDEX, 3);
				if (udc_readb(USB_CTRL_2) & 1)
					queue_work(ep_wqueue, &ep->work);
			} else if (!strcmp(_ep->name, udc->ep[4].ep.name)) { /* ep4 */ 
				udc_writeb(USB_EP_INDEX, 4);
				if ((udc_readb(USB_CTRL_1) & 1) == 0)
					queue_work(ep_wqueue, &ep->work);
			} else if (!strcmp(_ep->name, udc->ep[5].ep.name)){ /* ep5 */ 
				udc_writeb(USB_EP_INDEX, 5);
				if (udc_readb(USB_CTRL_2) & 1)
					queue_work(ep_wqueue, &ep->work);
			} else {
				printk("Invalid ep");
				status = -EINVAL;
				goto stall;
			}
		}
	} else {
	/* 
	  * if the queue is not empty, add the req to the queue only
	  */
		status = 0;
		list_add_tail(&req->queue, &ep->queue);
		dbg("waiting for %s int", ep->ep.name);
	}

stall:
	local_irq_restore(flags);
	/* return (status < 0) ? status : 0; */
	return status;
}

/*
  * delete the request from the ep request queue
  * @ep: the ep which the request deal with
  */
static int akudc_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct akudc_ep	*ep;
	struct akudc_request	*req;
	unsigned long flags;
	struct akudc_udc *udc = &controller;
	
	dbg("dequeue");

	if (!udc->driver)
		return -ESHUTDOWN;

	ep = container_of(_ep, struct akudc_ep, ep);
	if (!_ep || !_req)
		return -EINVAL;
	
	local_irq_save(flags);
	/* find the req from head */
	list_for_each_entry (req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	/* if cannot find, return error */
	if (&req->req != _req) {
		local_irq_restore(flags);
		return -EINVAL;
	}
	/* delete the req and set the req status  -ECONNRESET */
	done(ep, req, -ECONNRESET);
	local_irq_restore(flags);
	
	return 0;
}

/* 
  * set or clear the stall function for ep
  * @ep: which ep is being setting
  * @value: 1 stall, 0 clear stall
  */
static int akudc_ep_set_halt(struct usb_ep *_ep, int value)
{	
	struct akudc_ep *ep = container_of(_ep, struct akudc_ep, ep);
	unsigned int csr = 0;
	unsigned long		flags;
	struct akudc_udc *udc = &controller;

	if (unlikely (!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		dbg("inval 2");
		return -EINVAL;
	}

	local_irq_save (flags);

	if ((ep->ep.name == ep0name) && value) {
		udc_writeb(USB_EP_INDEX, 0);
		udc_writeb(USB_CTRL_1, 1<<5);
		udc_writeb(USB_CTRL_1, 1<<6 | 1<<3);
	}else if(!strcmp(ep->ep.name, udc->ep[1].ep.name)){/* ep1 */
		udc_writeb(USB_EP_INDEX, 1);
		csr = udc_readw(USB_CTRL_1);
		if (value)
			udc_writew(USB_CTRL_1, csr | 1<<4);
		else {
			csr &= ~(1<<4 | 1<<5);
			udc_writew(USB_CTRL_1, csr);
			csr |= 1<<6;
			udc_writew(USB_CTRL_1, csr);
		}
	} else if (ep->is_in) { /* ep2 or ep4*/ 
		if (!strcmp(ep->ep.name, udc->ep[2].ep.name))
			udc_writeb(USB_EP_INDEX, 2);
		else
			udc_writeb(USB_EP_INDEX, 4);
		
		csr = udc_readw(USB_CTRL_1);
		if (value)
			udc_writew(USB_CTRL_1, csr | 1<<4);
		else {
			csr &= ~(1<<4 | 1<<5);
			udc_writew(USB_CTRL_1, csr);
			csr |= 1<<6;
			udc_writew(USB_CTRL_1, csr);
		}
	} else { /* ep3 or ep5*/
		if (!strcmp(ep->ep.name, udc->ep[3].ep.name))
			udc_writeb(USB_EP_INDEX, 3);
		else
			udc_writeb(USB_EP_INDEX, 5);
		
		csr = udc_readw(USB_CTRL_2);
		if (value)
			udc_writew(USB_CTRL_2, csr | 1<<5);
		else {
			csr &= ~(1<<5 | 1<<6);
			udc_writew(USB_CTRL_2, csr);
			csr |= 1<<7;
			udc_writew(USB_CTRL_2, csr);
		}
	}

	ep->stopped= value ? 1 : 0;
	local_irq_restore (flags);

	return 0;
}

/* the operation struct for ep */
static const struct usb_ep_ops akudc_ep_ops = {
	.enable		= akudc_ep_enable,
	.disable	= akudc_ep_disable,
	.alloc_request	= akudc_ep_alloc_request,
	.free_request	= akudc_ep_free_request,
	.queue		= akudc_ep_queue,
	.dequeue	= akudc_ep_dequeue,
	.set_halt	= akudc_ep_set_halt,
	// there's only imprecise fifo status reporting
};

static void nop_release(struct device *dev)
{
			/* nothing to free */
}

/* the core struct for udc that include many important struct */
static struct akudc_udc controller = {
	/* the gadget , include ep0 info*/
	.gadget = {
		.ops	= &akudc_udc_ops,
		.ep0	= &controller.ep[0].ep,
		.name	= driver_name,
		.dev	= {
		       .init_name = "gadget",
			   .release = nop_release,
		}
	},
	/* the array of endpoint */
	.ep[0] = {
		.ep = {
			.name		= ep0name,//ep_name[0],
			.ops		= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP0_FIFO_SIZE,
	},
	.ep[1] = {
		.ep = {
			.name	= "ep1-int",//ep_name[1],
			.ops	= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP1_FIFO_SIZE,
	},
	.ep[2] = {
		.ep = {
			.name	= "ep2in-bulk",//ep_name[2],
			.ops	= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP2_FIFO_SIZE,
	},
	.ep[3] = {
		.ep = {
			/* could actually do bulk too */
			.name	= "ep3out-bulk",//ep_name[3],
			.ops	= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP3_FIFO_SIZE,
	},
	.ep[4] = {
		.ep = {
			.name	= "ep4in-bulk",//ep_name[4],
			.ops	= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP4_FIFO_SIZE,
	},
	.ep[5] = {
		.ep = {
			.name	= "ep5out-bulk",//ep_name[5],
			.ops	= &akudc_ep_ops,
		},
		.udc		= &controller,
		.maxpacket	= EP5_FIFO_SIZE,
	},
};

/**
 * akudc_udc_get_status - process request GET_STATUS
 * @udc: The device state
 * @ctrl: USB control request
 */
static int akudc_udc_get_status(struct akudc_udc *udc,
					struct usb_ctrlrequest *ctrl)
{
	u16 status = 0;
	u8 ep_num = ctrl->wIndex & 0x7F;
	u8 is_in = ctrl->wIndex & USB_DIR_IN;

	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		status = udc->devstatus;
		break;

	case USB_RECIP_INTERFACE:
		/* currently, the data result should be zero */
		break;

	case USB_RECIP_ENDPOINT:
		if (ep_num > 5 || ctrl->wLength > 2)
			return 1;

		if (ep_num == 0) {
			udc_writeb(USB_EP_INDEX, 0);
			status = udc_readb(USB_CTRL_1);
			status = status & (1<<5);
		} else {
			udc_writeb(USB_EP_INDEX, ep_num);
			if (is_in) {
				status = udc_readb(USB_CTRL_1);
				status = status & (1<<4);
			} else {
				status = udc_readb(USB_CTRL_2);
				status = status & (1<<5);
			}
		}

		status = status ? 1 : 0;
		break;
	default:
		return 1;
	}

	udc_writeb(USB_EP0_FIFO, status & 0xff);
	udc_writeb(USB_EP0_FIFO, status >> 8);

	udc_writel(USB_EP0_NUM, 2); /* 7bits */ 
	udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<1);

	udc->ep0state = EP0_END_XFER;

	return 0;
}

/* function for ep0 setup phase */
static void akudc_udc_handle_ep0_idle(struct akudc_udc *udc,
					struct akudc_ep *ep, u32 ep0csr)
{
	struct usb_ctrlrequest crq;
	int i, len, ret, tmp, timeout;
	unsigned char *buf;

	/* start control request? */
	if (!(ep0csr & 1))
		return;

	len = udc_readw(USB_EP_COUNT);
	buf = (unsigned char *)&crq;
	if (len > sizeof(struct usb_ctrlrequest))
		len = sizeof(struct usb_ctrlrequest);
	for (i = 0; i < len; i++) {
		/* read the setup data from ep0 fifo */
		buf[i] = udc_readb(USB_EP0_FIFO);
	}
	if (len != sizeof(crq)) {
		dbg("setup begin: fifo READ ERROR"
			" wanted %d bytes got %d. Stalling out...",
			sizeof(crq), len);
		udc_writeb(USB_CTRL_1, 1<<5);
		return;
	}

	dbg("bRequest = %d bRequestType %d wLength = %d",
		crq.bRequest, crq.bRequestType, crq.wLength);

	/* cope with automagic for some standard requests. */
	udc->req_std = (crq.bRequestType & USB_TYPE_MASK)
		== USB_TYPE_STANDARD;
	udc->req_config = 0;
	udc->req_pending = 1;

	switch (crq.bRequest) {
	case USB_REQ_SET_CONFIGURATION:
		if (crq.bRequestType == USB_RECIP_DEVICE) {
			udc->req_config = 1;
			udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<6);
		}
		break;

	case USB_REQ_SET_INTERFACE:
		if (crq.bRequestType == USB_RECIP_INTERFACE) {
			udc->req_config = 1;
			udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<6);
		}
		break;

	case USB_REQ_SET_ADDRESS:
		if (crq.bRequestType == USB_RECIP_DEVICE) {
			tmp = crq.wValue & 0x7F;
			udc_writeb(USB_CTRL_1, 0x1<<3 | 0x1<<6);
			timeout = 20000;
			/* waiting for next interrupt */ 
			while (!(udc_readb(USB_INTERRUPT_1) & 0x1) && timeout) {timeout--;}
			udc_writeb(USB_FUNCTION_ADDR, tmp);
			udc->addr = tmp;
			return;
		}
		break;

	case USB_REQ_GET_STATUS:
		udc_writeb(USB_CTRL_1, 0x1<<6);

		if (udc->req_std) {
			if (!akudc_udc_get_status(udc, &crq)) {
				return;
			}
		}
		break;

	case USB_REQ_CLEAR_FEATURE:
		udc_writeb(USB_CTRL_1, 0x1<<6);

		if (crq.bRequestType != USB_RECIP_ENDPOINT)
			break;

		if (crq.wValue != USB_ENDPOINT_HALT || crq.wLength != 0)
			break;

		akudc_ep_set_halt(&udc->ep[crq.wIndex & 0x7f].ep, 0);
		udc_writeb(USB_CTRL_1, 0x1<<6 | 0x1<<3);
		
		return;

	case USB_REQ_SET_FEATURE:
		udc_writeb(USB_CTRL_1, 0x1<<6);

		if (crq.bRequestType != USB_RECIP_ENDPOINT)
			break;

		if (crq.wValue != USB_ENDPOINT_HALT || crq.wLength != 0)
			break;

		akudc_ep_set_halt(&udc->ep[crq.wIndex & 0x7f].ep, 1);
		udc_writeb(USB_CTRL_1, 0x1<<6 | 0x1<<3);
		return;

	default:
		udc_writeb(USB_CTRL_1, 0x1<<6);
		break;
	}

	/* set ep0state according to the command */
	if (crq.bRequestType & USB_DIR_IN)
		udc->ep0state = EP0_IN_DATA_PHASE;
	else
		udc->ep0state = EP0_OUT_DATA_PHASE;

	/* call the function drvier setup */
	ret = udc->driver->setup(&udc->gadget, &crq);
	/* 
	  * if the setup failed, sent stall ep0
	  */
	if (ret < 0) {
		if (udc->req_config) {
			dbg("config change %02x fail %d?",
				crq.bRequest, ret);
			return;
		}

		if (ret == -EOPNOTSUPP)
			dbg("Operation not supported");
		else
			dbg("udc->driver->setup failed. (%d)", ret);
		
		udc_writeb(USB_CTRL_1, 1<<5);
		udc_writeb(USB_CTRL_1, 1<<3 | 1<<6);
		udc->ep0state = EP0_IDLE;
		/* deferred i/o == no response yet */
	} else if (udc->req_pending) {
		dbg("dev->req_pending... what now?");
		udc->req_pending=0;
	}
}

/* deal with interrupt for ep0 */
static void handle_ep0(struct akudc_udc *udc)
{
	int csr, error = 0;
	struct akudc_ep *ep0 = &udc->ep[0];
	struct akudc_request *req = NULL;

	if (!list_empty(&ep0->queue)) 
		req = list_entry(ep0->queue.next, struct akudc_request, queue);
	
	udc_writeb(USB_EP_INDEX, 0);
	csr = udc_readb(USB_CTRL_1);
	dbg("csr(0x%x)", csr);

	if (csr & 0x1<<3) {
		dbg("data end");
	} 
	if (csr & 0x1<<4) {
		dbg("A control transaction ends before the DataEnd bit has been set");
		udc_writeb(USB_CTRL_1, 0x1<<7);
		/* do something else? */
		udc->ep0state = EP0_IDLE;
		error = 1;
	}
	if (csr & 0x1<<2) {
		udc_writeb(USB_CTRL_1, udc_readb(USB_CTRL_1) & ~(1 << 2));
		udc->ep0state = EP0_IDLE;
		error = 1;
	}
	switch (udc->ep0state) {
	case EP0_IDLE:
		akudc_udc_handle_ep0_idle(udc, ep0, csr);
		break;

	case EP0_IN_DATA_PHASE:			/* GET_DESCRIPTOR etc */
		dbg("EP0_IN_DATA_PHASE ... what now?");
		if (!(csr & (1<<1)) && req && error == 0)
			write_ep0_fifo(ep0, req);
		break;

	case EP0_OUT_DATA_PHASE:		/* SET_DESCRIPTOR etc */
		dbg("EP0_OUT_DATA_PHASE ... what now?");
		if ((csr & (1<<0)) && req)
			read_ep0_fifo(ep0, req);
		break;

	case EP0_END_XFER:
		dbg("EP0_END_XFER ... what now?");
		udc->ep0state = EP0_IDLE;
		break;

	case EP0_STALL:
		dbg("EP0_STALL ... what now?");
		udc->ep0state = EP0_IDLE;
		break;
	}
}

/* not ep0 */
static void handle_ep(struct akudc_ep *ep)
{
	struct akudc_request *req;
	struct akudc_udc *udc = ep->udc;
	unsigned int csr;

	if (!list_empty(&ep->queue))
		req = list_entry(ep->queue.next,
			struct akudc_request, queue);
	else {
		dbg("%s: no req waiting", ep->ep.name);
		req = NULL;
	}
	
	if (!strcmp(ep->ep.name, udc->ep[1].ep.name)) { /* ep1 */ 
		dbg("ep1");
		udc_writeb(USB_EP_INDEX, 1);

		/* read the epx status */
		csr = udc_readb(USB_CTRL_1);
		dbg("ep1 csr(0x%x), req(0x%p)", csr, req);

		if (csr & 0x1<<2) { /* clear underrun */
			udc_writeb(USB_CTRL_1, csr & ~(0x1<<2));
		}
		if (csr & 0x1<<5) { /* clear sentstall */
			udc_writeb(USB_CTRL_1, csr & ~(0x1<<5));
			return;
		}

		if (req) {
			/* 
			  * if the current req is complete, done it with status 0
			  * and if the next req is being, and the epx can be written
			  * data, do it with work queue
			  * else do the current req with work queue if the epx can be written
			  */
			if (ep->done) {
				done(ep, req, 0);
				if (!list_empty(&ep->queue)) {
					dbg("do next queue");
					req = list_entry(ep->queue.next, struct akudc_request, queue);
					if ((csr & 1) == 0)
						queue_work(ep_wqueue, &ep->work);
				}
			} else {
				if ((csr & 1) == 0)
					queue_work(ep_wqueue, &ep->work);
			}

		}
	} else if (ep->is_in) { /* ep2 or ep4*/ 

		if (!strcmp(ep->ep.name, udc->ep[2].ep.name))
			udc_writeb(USB_EP_INDEX, 2);
		else
			udc_writeb(USB_EP_INDEX, 4);
	
		/* read the epx status */
		csr = udc_readb(USB_CTRL_1);
		dbg("ep2 or ep4 csr(0x%x), req(0x%p)", csr, req);

		/* udc_writeb(USB_CTRL_1, 0x1); */
		if (csr & 0x1<<2) { /* clear underrun */
			udc_writeb(USB_CTRL_1, csr & ~(0x1<<2));
		}
		if (csr & 0x1<<5) { /* clear sentstall */
			udc_writeb(USB_CTRL_1, csr & ~(0x1<<5));
			return;
		}

		if (req) {
			/* 
			  * if the current req is complete, done it with status 0
			  * and if the next req is being, and the epx can be written
			  * data, do it with work queue
			  * else do the current req with work queue if the epx can be written
			  */
			if (ep->done) {
				done(ep, req, 0);
				if (!list_empty(&ep->queue)) {
					dbg("do next queue");
					req = list_entry(ep->queue.next, struct akudc_request, queue);
					if ((csr & 1) == 0)
						queue_work(ep_wqueue, &ep->work);
				}
			} else {
				if ((csr & 1) == 0)
					queue_work(ep_wqueue, &ep->work);
			}

		}
	} else { /* ep3 or ep 5*/ 
		if (!strcmp(ep->ep.name, udc->ep[3].ep.name))
			udc_writeb(USB_EP_INDEX, 3);
		else
			udc_writeb(USB_EP_INDEX, 5);
	
		/* read the epx status */
		csr = udc_readb(USB_CTRL_2);
		dbg("ep3 or ep5 csr(0x%x)", csr);

		if (csr & 0x1<<6) { /* clear sentstall */
			udc_writeb(USB_CTRL_2, csr & ~(0x1<<6));
		}
		/* 
		  * if the req is being and the data come to the endpoint fifo
		  * do the current req with work queue 
		  */
		if (req && (csr & 0x1<<0))
			queue_work(ep_wqueue, &ep->work);
	}
}

/* 
  * the function execute when the suspend signal come
  * or the system is being suspend
  */
static void udc_disconnect(struct akudc_udc *udc)
{
	struct usb_gadget_driver *driver = udc->driver;
	int i;

	if (udc->gadget.speed == USB_SPEED_UNKNOWN)
		driver = NULL;

	for (i = 0; i < ENDPOINTS_NUM; i++) {
		struct akudc_ep *ep = &udc->ep[i];
		struct akudc_request *req;

		/* sign every endpoint stop */
		ep->stopped = 1;

		// terminer chaque requete dans la queue
		if (list_empty(&ep->queue))
			continue;

		/* 
		  * delete every req in every endpoint queue 
		  * dont it by status -ESHUTDOWN
		  */
		while (!list_empty(&ep->queue)) {
			req = list_entry(ep->queue.next, struct akudc_request, queue);
			done(ep, req, -ESHUTDOWN);
		}
	}

	/* if the driver exist, call the function drvier disconnect */
	if (driver)
		driver->disconnect(&udc->gadget);

	/* init the udc */
	udc_reinit(udc);
}

#if 0
static void akudc_udc_fun(void *data)
{	
	struct akudc_udc *udc = data;

	msleep(500);
	clk_enable(udc->clk);
	udc_reg_writel(USB_OP_MOD_REG, 0x6, 3, 0);
	udc_writeb(USB_POWER_CTRL, 0x1<<5);
}
#endif

static void akudc_conctrol_reset(struct akudc_udc *udc)
{
	// reset the udc controller module
	ak_soft_reset(AK_SRESET_USBHS);
	REG32(AK_VA_SYSCTRL + 0x58) &= ~(0xff << 0);
}

/* the function execute when the reset signal come */
static void udc_reset(struct akudc_udc *udc)
{
	int i ,temp;
	
	udc_writeb(USB_FUNCTION_ADDR, 0); // set the usb device addr 0
	udc_writeb(USB_INTERRUPT_USB, ~(0x1<<3)); // clear SOF interrupt
	udc_writeb(USB_INTERRUPT_TX, 0x1<<0); // enable ep0 interrupt and disable other tx endpoint 
	udc_writeb(USB_INTERRUPT_RX, 0); // disable rx endpoint inttrupt 

	/* negotiated high speed */
	if ((udc_readb(USB_POWER_CTRL) & (0x1 << 4)) == (0x1 << 4)) {
		udc_writel(USB_MODE_STATUS, udc_readl(USB_MODE_STATUS) & (~0x1));
		/* enable high speed for udc */
		udc_writeb(USB_POWER_CTRL, 0x1<<5);
		udc->gadget.speed = USB_SPEED_HIGH;
		udc->high_speed = 1;
	} else {  /* negotiated full speed */
		udc_writeb(USB_POWER_CTRL, 0);
		udc_writel(USB_MODE_STATUS, udc_readl(USB_MODE_STATUS) | 0x1);
		udc->gadget.speed = USB_SPEED_FULL;
		udc->high_speed = 0;
	}

	/* read and clear the common interrupt status  */
	temp = udc_readw(USB_INTERRUPT_COMM);
	/* read and clear the ep0 and all tx ep interrupt status  */
	temp = udc_readw(USB_INTERRUPT_1);
	/* read and clear all rx ep interrupt status  */
	temp = udc_readw(USB_INTERRUPT_2);

	/* init the ep0 status */
	udc->ep0state = EP0_IDLE;

	for (i = 0; i < ENDPOINTS_NUM; i++) {
		struct akudc_ep *ep = &udc->ep[i];
		struct akudc_request *req;

		if (list_empty(&ep->queue))
			continue;

		/* 
		  * delete every req in every endpoint queue 
		  * dont it by status -ECONNRESET
		  */
		while (!list_empty(&ep->queue)) {
			req = list_entry(ep->queue.next, struct akudc_request, queue);
			done(ep, req, -ECONNRESET);
		}
	}
}


/* the udc irq irqhandler */
static irqreturn_t udc_irqhandler(int irq, void *_udc)
{
	struct akudc_udc *udc = _udc;
	short status_1, status_2;
	char status_int;

	status_int = udc_readb(USB_INTERRUPT_COMM);
	if (status_int & 0x1<<2) {
		/* dbg("status_int(0x%x), reset", status_int);	 */
		printk("\n\nstatus_int(0x%x), reset\n\n", status_int);
		if (usb_detect) {
			usb_detect = 0;
			if (!udc->driver) {
				panic("If you see this, come to find Zhang Jingyuan\n");
				akudc_disable(udc);
				return IRQ_HANDLED;
			}
		}
		udc_reset(udc);
		goto done;
	} else if(status_int & 0x1<<1) { /* resume */ 
		dbg("status_int(0x%x)", status_int);	
		goto done;
	} else if(status_int & 0x1<<0) { /* suspend */ 
		dbg("status_int(0x%x)", status_int);	
#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
		pm_power_off();
#endif
#endif
		udc_disconnect(udc);
		goto done;
	}

	status_1 = udc_readb(USB_INTERRUPT_1);
	status_2 = udc_readb(USB_INTERRUPT_2);
	/* ep0 */
	if (status_1 & 0x1<<0) {
		handle_ep0(udc);
	}
	/* ep2 */
	if (status_1 & 0x1<<2) {
		dbg("endpoint2");
		handle_ep(&udc->ep[2]);
	}
	/* ep4 */
	if (status_1 & 0x1<<4) {
		dbg("endpoint4");
		handle_ep(&udc->ep[4]);
	}

	/* ep1 */
	if (status_2 & 0x1<<1) {
		dbg("endpoint 1");
		handle_ep(&udc->ep[1]);
	}
	/* ep3 */
	if (status_2 & 0x1<<3) {
		dbg("endpoint3");
		handle_ep(&udc->ep[3]);
	}
	/* ep5 */
	if (status_2 & 0x1<<5) {
		dbg("endpoint5");
		handle_ep(&udc->ep[5]);
	}

done:
	return IRQ_HANDLED;
}

#if 0
/* the l2 buffer dma irqhandler */
static irqreturn_t udc_dmahandler(int irq, void *_udc)
{
	struct akudc_request *req = NULL;
	struct akudc_udc *udc = _udc;
	struct akudc_ep *ep;
	unsigned int is_done = 0;

	u32 usb_dma_int = udc_readl(USB_DMA_INTR);

	/*
	  * affirm which channel's interrupt 
	  * channel1 -- ep2
	  * channel2 -- ep3
	  * channel3 -- ep4
	  * channel4 -- ep5
	  */
	if ((usb_dma_int & DMA_CHANNEL1_INT) == DMA_CHANNEL1_INT) {
		ep = &udc->ep[2];
		udc_writeb(USB_EP_INDEX, USB_EP2_INDEX);
		udc_writeb(USB_CTRL_1_2, USB_TXCSR_MODE1);
        udc_writel(USB_DMA_CTRL1, 0);

        l2_combuf_wait_dma_finish(ep->l2_buf_id);

		/* if the queue is not empty, get the head req */
		if (!list_empty(&ep->queue))
			req = list_entry(ep->queue.next, struct akudc_request, queue);

		if (req) {
			/* 
			  * judge whether the req is complete 
			  * if it is, done the req and do next req transmission
			  * else do the current req transmission
			  */
			if (dma_tx == req->req.length - req->req.actual && !req->req.zero) 
				is_done = 1;
			req->req.actual += dma_tx;
			ep->done = is_done;
			if (is_done) {
				done(ep, req, 0);
				if (!list_empty(&ep->queue)) {
					dbg("do next queue");
					queue_work(ep_wqueue, &ep->work);
				}
			} else {
				queue_work(ep_wqueue, &ep->work);
			}
			/* unmap the buffer */
			dma_unmap_single(NULL, phys_tx, dma_tx, DMA_TO_DEVICE);
			dma_tx = 0;

			req->status = is_done;
		}
		else
			dbg("something happend");
	}
	
	if ((usb_dma_int & DMA_CHANNEL2_INT) == DMA_CHANNEL2_INT) {
		ep = &udc->ep[3];

		udc_writeb(USB_EP_INDEX, USB_EP3_INDEX);
		udc_writeb(USB_CTRL_2_2, 0);
		udc_writel(USB_DMA_CTRL2, 0);
		
		l2_combuf_wait_dma_finish(ep->l2_buf_id);

		req = NULL;
		is_done = 0;
		/* if the queue is not empty, get the head req */
		if (!list_empty(&ep->queue))
			req = list_entry(ep->queue.next, struct akudc_request, queue);

		if (req) {
			/* 
			  * judge whether the req is complete 
			  * if it is, done the req
			  */
			if (dma_rx == req->req.length - req->req.actual)
				is_done = 1;
			req->req.actual += dma_rx;
			ep->done = is_done;
			if (is_done)
				done(ep, req, 0);
			/* unmap the buffer */
			dma_unmap_single(NULL, phys_rx, dma_rx, DMA_FROM_DEVICE);
			req->status = is_done;
			dma_rx = 0;

			flag = 0;
		}
		else
			dbg("something happend");
	}
	
	if ((usb_dma_int & DMA_CHANNEL3_INT) == DMA_CHANNEL3_INT) {
		ep = &udc->ep[4];
		udc_writeb(USB_EP_INDEX, USB_EP4_INDEX);
		udc_writeb(USB_CTRL_1_2, USB_TXCSR_MODE1);
        udc_writel(USB_DMA_CTRL1, 0);

        l2_combuf_wait_dma_finish(ep->l2_buf_id);

		/* if the queue is not empty, get the head req */
		if (!list_empty(&ep->queue))
			req = list_entry(ep->queue.next, struct akudc_request, queue);
		
		if (req) {
			/* 
			  * judge whether the req is complete 
			  * if it is, done the req and do next req transmission
			  * else do the current req transmission
			  */
			if (dma_tx == req->req.length - req->req.actual && !req->req.zero) 
				is_done = 1;
			req->req.actual += dma_tx;
			ep->done = is_done;
			if (is_done) {
				done(ep, req, 0);
				if (!list_empty(&ep->queue)) {
					dbg("do next queue");
					queue_work(ep_wqueue, &ep->work);
				}
			} else {
				queue_work(ep_wqueue, &ep->work);
			}
			/* unmap the buffer */
			dma_unmap_single(NULL, phys_tx, dma_tx, DMA_TO_DEVICE);
			dma_tx = 0;

			req->status = is_done;
		}
		else
			dbg("something happend");
	}
	
	if ((usb_dma_int & DMA_CHANNEL4_INT) == DMA_CHANNEL4_INT) {
		ep = &udc->ep[5];

		udc_writeb(USB_EP_INDEX, USB_EP5_INDEX);
		udc_writeb(USB_CTRL_2_2, 0);
		udc_writel(USB_DMA_CTRL2, 0);
		
		l2_combuf_wait_dma_finish(ep->l2_buf_id);

		req = NULL;
		is_done = 0;
		/* if the queue is not empty, get the head req */
		if (!list_empty(&ep->queue))
			req = list_entry(ep->queue.next, struct akudc_request, queue);

		if (req) {
			/* 
			  * judge whether the req is complete 
			  * if it is, done the req
			  */
			if (dma_rx == req->req.length - req->req.actual)
				is_done = 1;
			req->req.actual += dma_rx;
			ep->done = is_done;
			if (is_done)
				done(ep, req, 0);
			/* unmap the buffer */
			dma_unmap_single(NULL, phys_rx, dma_rx, DMA_FROM_DEVICE);
			req->status = is_done;
			dma_rx = 0;

			flag = 0;
		}
		else
			dbg("something happend");
	}

	return IRQ_HANDLED;
}
#endif

void akudc_enable(struct akudc_udc *udc)
{
	set_condition(&udc_clk);
}

void akudc_disable(struct akudc_udc *udc)
{
	clear_condition(&udc_clk);
}

/*
  *	usb_gadget_register_driver for upper function driver
  */
static int akudc_start(struct usb_gadget_driver *driver,
		int (*bind)(struct usb_gadget *))
{
	struct akudc_udc *udc = &controller;
	int		retval;

	printk("akudc start() '%s'\n", driver->driver.name);
	
	if (!bind || !driver->setup
		|| driver->max_speed < USB_SPEED_FULL) {
		printk(KERN_ERR "Invalid driver: bind %p setup %p speed %d\n",
			bind, driver->setup, driver->max_speed);
		return -EINVAL;
	}

#if defined(MODULE)
	if (!driver->unbind) {
		printk(KERN_ERR "Invalid driver: no unbind method\n");
		return -EINVAL;
	}
#endif

	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;

	/* 
	  * bind the gadget to function driver 
	  * give the gadget point to function driver
	  */
	retval = bind(&udc->gadget);
	if (retval) {
		dbg("driver->bind() returned %d\n", retval);
		udc->driver = NULL;
		udc->gadget.dev.driver = NULL;
		return retval;
	}

	/* init work for every endpoint */ 
	INIT_WORK(&udc->ep[1].work, ep1_work);
	INIT_WORK(&udc->ep[2].work, ep2_work);
	INIT_WORK(&udc->ep[3].work, ep3_work);
	INIT_WORK(&udc->ep[4].work, ep4_work);
	INIT_WORK(&udc->ep[5].work, ep5_work);

	disable_irq(udc->mcu_irq);
	akudc_enable(udc);
	enable_irq(udc->mcu_irq);

	printk("binding gadget driver '%s'\n", driver->driver.name);
	return 0;
}


/*
  *	usb_gadget_unregister_driver for upper function driver
  */
static int akudc_stop(struct usb_gadget_driver *driver)
{
	struct akudc_udc *udc = &controller;

	printk("akudc_stop() '%s'\n", driver->driver.name);

	if (!driver || driver != udc->driver || !driver->unbind)
		return -EINVAL;

	/* report disconnect */
	if(driver->disconnect)
		driver->disconnect(&udc->gadget);

	/* unbind the gadget for function driver */
	driver->unbind(&udc->gadget);
	udc->gadget.dev.driver = NULL;
	//udc->gadget.dev.driver_data = NULL;
	udc->driver = NULL;
	
	disable_irq(udc->mcu_irq);
	
	akudc_disable(udc);

	/*cacel work for every endpoint */
	cancel_work_sync(&udc->ep[1].work);
	cancel_work_sync(&udc->ep[2].work);
	cancel_work_sync(&udc->ep[3].work);
	cancel_work_sync(&udc->ep[4].work);
	cancel_work_sync(&udc->ep[5].work);

	flush_workqueue(ep_wqueue);

	enable_irq(udc->mcu_irq);

	dbg("unbound from '%s'\n", driver->driver.name);
	udc_reinit(udc);
	return 0;
}

/* reinit == restore initial software state */
static void udc_reinit(struct akudc_udc *udc)
{
	u32 i;

	/* init non-ep0 endpint list and ep0 list */
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);
	udc->ep0state = EP0_IDLE;

	for (i = 0; i < ENDPOINTS_NUM; i++) {
		struct akudc_ep *ep = &udc->ep[i];
		/* add every non-ep0 endpoint to gadget endpoint list */
		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);
		ep->desc = NULL;
		ep->stopped = 0;
		ep->ep.maxpacket = ep->maxpacket;
		/* initialize one queue per endpoint */
		INIT_LIST_HEAD(&ep->queue);
	}
}

static int __init akudc_udc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct akudc_udc *udc = &controller;
	struct resource	*res;
	int retval;

	if (pdev->num_resources < 2) {
		dbg("invalid num_resources\n");
		return -ENODEV;
	}
	if ((pdev->resource[0].flags != IORESOURCE_MEM)
			|| (pdev->resource[1].flags != IORESOURCE_IRQ)) {
		dbg("invalid resource type\n");
		return -ENODEV;
	}

	/* get the mem resoure */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	if (!request_mem_region(res->start, resource_size(res),	driver_name)) {
		dbg("someone's using UDC memory\n");
		return -EBUSY;
	}

	/* map the physical mem to virtual for register */
	udc->baseaddr = ioremap_nocache(res->start, resource_size(res));
	if (!udc->baseaddr) {
		retval = -ENOMEM;
		goto fail0a;
	}
	/* init software state */
	udc->gadget.dev.parent = dev;	//is null,

	/* get interface and function clocks */
	udc->clk = clk_get(dev, "usb-slave");
	if (IS_ERR(udc->clk)) {
		dbg("clocks missing\n");
		retval = -ENODEV;
		/* NOTE: we "know" here that refcounts on these are NOPs */
		goto fail0b;
	}
	
	akudc_conctrol_reset(udc);
	udc_reinit(udc);
	
	retval = device_register(&udc->gadget.dev);
	if (retval < 0)
		goto fail0b;

	/* creat the work queue for every ep */
	ep_wqueue = create_workqueue("akudc");
	
	/* request UDC and maybe VBUS irqs */
	udc->mcu_irq = platform_get_irq(pdev, 0);
	/* register the udc irq handler */
	retval = request_irq(udc->mcu_irq, udc_irqhandler,
			0, driver_name, udc);
	if (retval < 0) {
		dbg("request irq %d failed\n", udc->mcu_irq);
		goto fail1;
	}

#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	/* request DMA irqs */
	udc->dma_irq = platform_get_irq(pdev, 1);
	retval = request_irq(udc->dma_irq, udc_dmahandler,
			0, driver_name, udc);
	if (retval < 0) {
		dbg("request irq %d failed\n", udc->dma_irq);
		goto fail1;
	}

	/* USB slave L2 buffer initialization */
	udc->ep[2].l2_buf_id = l2_alloc(ADDR_USB_EP2);
	udc->ep[3].l2_buf_id = l2_alloc(ADDR_USB_EP3);
	udc->ep[4].l2_buf_id = l2_alloc(ADDR_USB_EP4);
	udc->ep[5].l2_buf_id = l2_alloc(ADDR_USB_EP5);
#endif
#endif

	/* set the udc pointer to pdev private pointer */
	platform_set_drvdata(pdev, udc);

	create_debugfs_files(udc);
	
	retval = usb_add_gadget_udc(&pdev->dev, &udc->gadget);
	if (retval)
		goto fail1;

	return 0;

	free_irq(udc->mcu_irq, udc);
fail1:
	device_unregister(&udc->gadget.dev);
fail0b:
	iounmap(udc->baseaddr);
fail0a:
	release_mem_region(res->start, resource_size(res));
	
	return retval;
}

static int __exit akudc_udc_remove(struct platform_device *pdev)
{
	struct akudc_udc *udc = platform_get_drvdata(pdev);
	struct resource *res;

	if (udc->driver)
		return -EBUSY;
	
	usb_del_gadget_udc(&udc->gadget);

	/* clear the udc_clk condition for udc */
	clear_condition(&udc_clk);

	create_debugfs_files(udc);

	free_irq(udc->mcu_irq, udc);
	device_unregister(&udc->gadget.dev);

	destroy_workqueue(ep_wqueue);

	iounmap(udc->baseaddr);

#if 0
#ifdef CONFIG_USB_AKUDC_PRODUCER
	free_irq(udc->dma_irq, udc);

	/* USB slave L2 buffer initialization */
	l2_free(ADDR_USB_EP2);
	l2_free(ADDR_USB_EP3);
	l2_free(ADDR_USB_EP4);
	l2_free(ADDR_USB_EP5);
#endif
#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	return 0;
}

static void akudc_udc_shutdown(struct platform_device *dev)
{
}

#ifdef CONFIG_PM
static int akudc_udc_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	struct akudc_udc *udc = platform_get_drvdata(pdev);

	/*cacel work for every endpoint */
	cancel_work_sync(&udc->ep[1].work);
	cancel_work_sync(&udc->ep[2].work);
	cancel_work_sync(&udc->ep[3].work);
	cancel_work_sync(&udc->ep[4].work);
	cancel_work_sync(&udc->ep[5].work);

	flush_workqueue(ep_wqueue);

	/* enable usb vbus wakeup function before enter standby */
	usb_vbus_wakeup(true);

	/* clear the udc_clk condition for udc */
	clear_condition(&udc_clk);

	udc_disconnect(udc);
	
	return 0;
}

static int akudc_udc_resume(struct platform_device *pdev)
{
	struct akudc_udc *udc = platform_get_drvdata(pdev);
	
	/* disable usb vbus wakeup function after wakeup */
	usb_vbus_wakeup(false);

	/* if we have the function driver, we set the udc_clk condition */
	if (udc->driver)
		set_condition(&udc_clk);
	
	return 0;
}
#else
#define	akudc_udc_suspend	NULL
#define	akudc_udc_resume	NULL
#endif

static struct platform_driver akudc_udc_driver = {
	.remove		= __exit_p(akudc_udc_remove),
	.shutdown	= akudc_udc_shutdown,
	.suspend	= akudc_udc_suspend,
	.resume		= akudc_udc_resume,
	.driver		= {
		.name	= (char *) driver_name,
		.owner	= THIS_MODULE,
	},
};

static int __init udc_init_module(void)
{
	printk("udc driver initialize, (c) 2013 Anyka\n");

	return platform_driver_probe(&akudc_udc_driver, akudc_udc_probe);
}
module_init(udc_init_module);

static void __exit udc_exit_module(void)
{
	platform_driver_unregister(&akudc_udc_driver);
}
module_exit(udc_exit_module);

MODULE_DESCRIPTION("Anyka udc driver");
MODULE_AUTHOR("Anyka");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform: usb-slave");
