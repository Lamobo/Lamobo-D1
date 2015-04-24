
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>

#include <asm/unaligned.h>
#include <plat-anyka/usb-hc.h>
#include <mach/clock.h>
#include <mach/reset.h>

#ifdef AKOTG_HS_DEBUG
#define HDBG(stuff...) printk("USBHS: " stuff)
#else
#define HDBG(fmt, args...) do{}while(0)
#endif 

#ifdef AKOTG_HS_VERBOSE_DEBUG
#define VDBG	HDBG
#else
#define VDBG(fmt, args...) do{}while(0)
#endif 


static int period_epfifo;
struct workqueue_struct *g_otghc_wq;
static struct delayed_work	g_otg_rest;
//static char *trans_desc[4] = {"iso", "intterrup", "conroller", "bulk"};
static u8 ep_fifos[] = { USB_FIFO_EP0, USB_FIFO_EP1, USB_FIFO_EP2, USB_FIFO_EP3, USB_FIFO_EP4, USB_FIFO_EP5};
struct akotghc_epfifo_mapping akotg_epfifo_mapping;

static inline int get_ep_type(int pipe)
{
	int eptype = 0;

	switch(usb_pipetype(pipe)) {
		case PIPE_ISOCHRONOUS:
			eptype = 1;
			break;
		case PIPE_BULK:
			eptype = 2;
			break;
		case PIPE_INTERRUPT:
			eptype = 3;
			break;
	}
	return eptype;
}

void port_power(struct akotg_usbhc *akotghc, int is_on)
{
	/*Anyka usb host is self power currently.*/
	struct usb_hcd	*hcd = akotg_usbhc_to_hcd(akotghc);

	/* hub is inactive unless the port is powered */
	if (is_on) {
		if (akotghc->port_status & (1 << USB_PORT_FEAT_POWER))
			return;

		akotghc->port_status = (1 << USB_PORT_FEAT_POWER);
	} else {
		akotghc->port_status = 0;
		hcd->state = HC_STATE_HALT;
	}

	/* reset as thoroughly as we can */
	ak_soft_reset(AK_SRESET_USBHS);
}

/*-------------------------------------------------------------------------*/

/* This is a PIO-only HCD.  Queueing appends URBs to the endpoint's queue,
 * and may start I/O.  Endpoint queues are scanned during completion irq
 * handlers (one per packet: ACK, NAK, faults, etc) and urb cancellation.
 *
 * Using an external DMA engine to copy a packet at a time could work,
 * though setup/teardown costs may be too big to make it worthwhile.
 */

/* SETUP starts a new control request.  Devices are not allowed to
 * STALL or NAK these; they must cancel any pending control requests.
 */
static void setup_packet(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb
)
{
	int	i;
	unsigned int fifo_val;
	u8	len;
	unsigned char *buf = urb->setup_packet;

	HDBG("Packet: Setup Packet (EP %d)\n", ep->epnum);

	len = sizeof(struct usb_ctrlrequest);

	hc_index_writeb(0, 0, USB_REG_TXCSR1);
	hc_writeb(usb_pipedevice(urb->pipe), USB_REG_FADDR);
	for (i = 0; i < len; i += 4) {
		fifo_val = (*(buf+i) | ( *(buf+i+1)<<8 )
				   | ( *(buf+i+2)<<16 ) | ( *(buf+i+3)<<24 ));
		hc_writel(fifo_val, USB_FIFO_EP0);
	}
	hc_index_writeb(0, USB_TXCSR1_FLUSHFIFO |USB_TXCSR1_FIFONOTEMPTY, USB_REG_TXCSR1);

	ep->length = 0;
}

/* STATUS finishes control requests, often after IN or OUT data packets */
static void status_packet(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb
)
{
	int			is_in;

	HDBG("Packet: Status Packet (EP %d)\n", ep->epnum);

	is_in = urb->transfer_buffer_length && usb_pipein(urb->pipe);
	//if (!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, !usb_pipein(urb->pipe), &epfifo))
	//	BUG();	/* Impossible, USB Device Endpoint *MUST* have been mapped to EPFIFO */
	
	hc_writeb(usb_pipedevice(urb->pipe), USB_REG_FADDR);

	if (is_in) {
		/* send a state packet when IN transaction is finished */
		hc_index_writeb(ep->epfifo, 0x42, USB_REG_TXCSR1);
	} else {
		/* request a  state packet when OUT transaction is finished */
		hc_index_writeb(ep->epfifo, 0x60, USB_REG_TXCSR1);
	}
		
	ep->length = 0;
}

/* IN packets can be used with any type of endpoint. here we just
 * start the transfer, data from the peripheral may arrive later.
 * urb->iso_frame_desc is currently ignored here...
 */
static void in_packet(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb
)
{
	u16			len;
	u32         remain;

    bool        bDMA = false;

	HDBG("Packet: IN Packet (EP %d), Length=%d,ep->maxpacket=%d\n",
		ep->epnum, urb->transfer_buffer_length - urb->actual_length, ep->maxpacket);

	/* avoid losing data on overflow */
	len = ep->maxpacket;
    remain = urb->transfer_buffer_length - urb->actual_length;

    ep->length = min_t(u32, len, remain);

	hc_writeb(usb_pipedevice(urb->pipe), USB_REG_FADDR);
	if (ep->epnum == 0) {
		hc_index_writeb(0, USB_TXCSR1_H_RXSTALL, USB_REG_TXCSR1);
	} else {
		int eptype = 0;
		//if (!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, 0, &epfifo))
		//	BUG();	/* Impossible, USB Device Endpoint *MUST* have been mapped to EPFIFO */

		eptype = get_ep_type(urb->pipe);

        /**
            dma condition: 
          1. high speed and ep size >= 512
          2. data leng >= 512
          3. l2 buffer is alloced for epfifo
        */
#ifdef CONFIG_USB_AKOTG_DMA        
        if((len == 512) && (remain >= len))
        {
            struct akotg_dma *usbdma;
            u8 regvalue;
            u32 pktnum = remain / 512;
            u32 count = remain - remain%512;
            unsigned char *buf;
            dma_addr_t phyaddr;
            u8          l2buffer;

            if(count < remain){
                count += 512;
            }
            
            do
            {
                //map dma buffer
                buf = urb->transfer_buffer + urb->actual_length;
                phyaddr = dma_map_single(NULL, buf, count, DMA_FROM_DEVICE);
                if (phyaddr == 0) {
                    printk("tx dma_map_single error!\n");
                    break;
                }

                //alloc dma channel
                usbdma = akotg_dma_alloc(akotghc, ep->epfifo);
                if(!usbdma){
                    //printk("dma alloc fail for in_packet\n");
                    dma_unmap_single(NULL, phyaddr, count, DMA_FROM_DEVICE);                    
                    break;
                }
                
                l2buffer = usbdma->l2buffer;

                //config rx reg
                regvalue = hc_index_readb(ep->epfifo, USB_REG_RXCSR2);
                regvalue |= (USB_RXCSR2_AUTOCLEAR | USB_RXCSR2_AUTOREQ | USB_RXCSR2_DMAENAB | USB_RXCSR2_DMAMODE);
                hc_index_writeb(ep->epfifo, regvalue, USB_REG_RXCSR2);

                hc_writew(pktnum, USB_REG_REQPKTCNT(ep->epfifo));

                //config l2
                l2_clr_status(l2buffer);
                l2_combuf_dma(phyaddr, l2buffer, count, BUF2MEM, AK_FALSE);

                //config dma
                akotg_dma_set_struct(usbdma, USB_DIRECTION_RX, ep->epfifo, l2buffer, count, phyaddr);
                akotg_dma_config(usbdma);

                ep->length = count;

                bDMA = true;
            }
            while(0);

        }
#endif
        ep->bdma = bDMA;

		hc_index_writeb(ep->epfifo, USB_RXCSR1_H_REQPKT, USB_REG_RXCSR1);
	}
}

/* OUT packets can be used with any type of endpoint.
 * urb->iso_frame_desc is currently ignored here...
 */
static void out_packet(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb
)
{
	int i;
	unsigned char		*buf;
	u16			        len;
	u32                 remain;

    bool                bDMA = false;

	HDBG("Packet: OUT Packet (EP %d), Length=%d\n", ep->epnum, urb->transfer_buffer_length - urb->actual_length);

	buf = (unsigned char *)(urb->transfer_buffer + urb->actual_length);
	prefetch(buf);

    remain = urb->transfer_buffer_length - urb->actual_length;
	len = min_t(u32, ep->maxpacket, remain);

	//if (!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, 1, &epfifo))
	//	BUG();	/* Impossible, USB Device Endpoint *MUST* have been mapped to EPFIFO */

    /**
        dma condition: 
      1. high speed and ep size >= 512
      2. data leng >= 512
      3. l2 buffer is alloced for epfifo
    */
#ifdef CONFIG_USB_AKOTG_DMA
    if((len == 512) && (remain >= len))
    {
        struct akotg_dma *usbdma;
        u8 regvalue;
        u32 count = remain - remain%512;
        unsigned char *buf;
        dma_addr_t phyaddr;
        u8 l2buffer;

        do
        {
            //map dma buffer
            buf = urb->transfer_buffer + urb->actual_length;
            phyaddr = dma_map_single(NULL, buf, count, DMA_TO_DEVICE);
            if (phyaddr == 0) {
                printk("rx dma_map_single error!\n");
                break;
            }

            //alloc dma channel
            usbdma = akotg_dma_alloc(akotghc, ep->epfifo);
            if(!usbdma){
                //printk("dma alloc fail for out_packet\n");
                dma_unmap_single(NULL, phyaddr, count, DMA_TO_DEVICE);                    
                break;
            }

            l2buffer = usbdma->l2buffer;


            //config tx reg
            regvalue = hc_index_readb(ep->epfifo, USB_REG_TXCSR2);
            regvalue |= (USB_TXCSR2_DMAMODE1|USB_TXCSR2_DMAENAB|USB_TXCSR2_MODE|USB_TXCSR2_AUTOSET);
            hc_index_writeb(ep->epfifo, regvalue, USB_REG_TXCSR2);

            //config l2
            l2_clr_status(l2buffer);
            l2_combuf_dma(phyaddr, l2buffer, count, MEM2BUF, AK_FALSE);

            //config dma
            akotg_dma_set_struct(usbdma, USB_DIRECTION_TX, ep->epfifo, l2buffer, count, phyaddr);
            akotg_dma_config(usbdma);

            ep->length = count;

            bDMA = true;
        }
        while(0);
        
    }
#endif
    ep->bdma = bDMA;

    if(!bDMA)
    {
    	hc_index_writeb(ep->epfifo, 0, USB_REG_TXCSR1);
    	hc_writeb(usb_pipedevice(urb->pipe), USB_REG_FADDR);
    	for (i = 0; i < len; i ++) {
    		hc_writeb(buf[i], ep_fifos[ep->epfifo]);
    	}
    	if (ep->epnum == 0) {
    		hc_index_writeb(0, 0x02, USB_REG_TXCSR1);
    	} else {

    		hc_index_writeb(ep->epfifo, USB_TXCSR1_TXPKTRDY, USB_REG_TXCSR1);
    	}

        ep->length = len;
    }
}

/*-------------------------------------------------------------------------*/

/* caller updates on-chip enables later */
static inline void sofirq_on(struct akotg_usbhc *akotghc)
{
    unsigned int regval;

    regval = hc_readb(USB_REG_INTRUSBE);
    regval |= USB_INTR_SOF;
    hc_writeb(regval, USB_REG_INTRUSBE);
}

static inline void sofirq_off(struct akotg_usbhc *akotghc)
{
    unsigned int regval;

    regval = hc_readb(USB_REG_INTRUSBE);
    regval &= ~USB_INTR_SOF;
    hc_writeb(regval, USB_REG_INTRUSBE);
}

/*-------------------------------------------------------------------------*/

static struct akotghc_ep *start_ep0(struct akotg_usbhc *akotghc)
{
	struct akotghc_ep	*ep;
	struct urb		*urb;
	
	/* use endpoint at schedule head */
	if (akotghc->next_async_ep0)
		ep = akotghc->next_async_ep0;
	else if (!list_empty(&akotghc->async_ep0)) {
		ep = container_of(akotghc->async_ep0.next,
				struct akotghc_ep, schedule);
	} else {
		/* could set up the first fullspeed periodic
		 * transfer for the next frame ...
		 */
		return NULL;
	}

	if (ep->schedule.next == &akotghc->async_ep0)
		akotghc->next_async_ep0 = NULL;
	else
		akotghc->next_async_ep0 = container_of(ep->schedule.next,
			struct akotghc_ep, schedule);

	if (unlikely(list_empty(&ep->hep->urb_list))) {
		HDBG("empty %p queue?\n", ep);
		return NULL;
	}

	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);

	switch (ep->nextpid) {
	case USB_PID_IN:
		in_packet(akotghc, ep, urb);
		break;
	case USB_PID_OUT:
		out_packet(akotghc, ep, urb);
		break;
	case USB_PID_SETUP:
		setup_packet(akotghc, ep, urb);
		break;
	case USB_PID_ACK:		/* for control status */
		status_packet(akotghc, ep, urb);
		break;
	default:
		HDBG("bad ep%p pid %02x\n", ep, ep->nextpid);
		ep = NULL;
	}
	return ep;
}


/* pick the next endpoint for a transaction, and issue it.
 * frames start with periodic transfers (after whatever is pending
 * from the previous frame), and the rest of the time is async
 * transfers, scheduled round-robin.
 */
static struct akotghc_ep *start_epx(struct akotg_usbhc *akotghc, int epfifo)
{
	struct akotghc_ep	*ep;
	struct urb		*urb;

	
	/* use endpoint at schedule head */
	if (akotghc->next_periodic) {
		ep = akotghc->next_periodic;
		akotghc->next_periodic = ep->next;
	} else {
		if (akotghc->next_async_epx[epfifo-1])
			ep = akotghc->next_async_epx[epfifo-1];
		else if (!list_empty(&akotghc->async_epx[epfifo-1])) {
			ep = container_of(akotghc->async_epx[epfifo-1].next,
					struct akotghc_ep, schedule);

		} else {
			/* could set up the first fullspeed periodic
			 * transfer for the next frame ...
			 */
			return NULL;
		}

		if (ep->schedule.next == &akotghc->async_epx[epfifo-1])
			akotghc->next_async_epx[epfifo-1] = NULL;
		else {
			akotghc->next_async_epx[epfifo-1] = container_of(ep->schedule.next,
					struct akotghc_ep, schedule);
		}
	}

	if (unlikely(list_empty(&ep->hep->urb_list))) {
		HDBG("empty %p queue?\n", ep);
		return NULL;
	}
	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
	switch (ep->nextpid) {
	case USB_PID_IN:
		in_packet(akotghc, ep, urb);
		break;
	case USB_PID_OUT:
		out_packet(akotghc, ep, urb);
		break;
	case USB_PID_SETUP:
		setup_packet(akotghc, ep, urb);
		break;
	case USB_PID_ACK:		
		status_packet(akotghc, ep, urb);
		break;
	default:
		HDBG("bad ep%p pid %02x\n", ep, ep->nextpid);
		ep = NULL;
	}
	return ep;
}

#define MIN_JIFFIES	((msecs_to_jiffies(2) > 1) ? msecs_to_jiffies(2) : 2)

static inline void start_transfer_ep0(struct akotg_usbhc *akotghc)
{
	if (akotghc->port_status & (1 << USB_PORT_FEAT_SUSPEND))
		return;
	if (akotghc->active_ep0 == NULL) {
		akotghc->active_ep0 = start_ep0(akotghc);
		if (akotghc->active_ep0 != NULL)
			akotghc->jiffies_ep0 = jiffies + MIN_JIFFIES;
	}
}

static inline void start_transfer_epx(struct akotg_usbhc *akotghc, int epfifo)
{
	if (akotghc->port_status & (1 << USB_PORT_FEAT_SUSPEND))
		return;
	if (akotghc->active_epx[epfifo-1] == NULL) {
		akotghc->active_epx[epfifo-1] = start_epx(akotghc, epfifo);
		if (akotghc->active_epx[epfifo-1] != NULL)
			akotghc->jiffies_epx[epfifo-1] = jiffies + MIN_JIFFIES;
	}
}

static inline void start_transfer(struct akotg_usbhc *akotghc, int epfifo)
{

	if(epfifo == 0)
		start_transfer_ep0(akotghc);
	else
		start_transfer_epx(akotghc, epfifo);
	
}

static void finish_request_ep0(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb,
	int			status
) __releases(akotghc->lock) __acquires(akotghc->lock)
{
	VDBG("Finishing EP0 URB Request...\n");

	if (usb_pipecontrol(urb->pipe))
		ep->nextpid = USB_PID_SETUP;

	usb_hcd_unlink_urb_from_ep(akotg_usbhc_to_hcd(akotghc), urb);
	spin_unlock(&akotghc->lock);
	usb_hcd_giveback_urb(akotg_usbhc_to_hcd(akotghc), urb, status);
	spin_lock(&akotghc->lock);

	/* leave active endpoints in the schedule */
	if (!list_empty(&ep->hep->urb_list))
		return;

	/* async deschedule? */
	if (!list_empty(&ep->schedule)) {
		list_del_init(&ep->schedule);
		if (ep == akotghc->next_async_ep0)
			akotghc->next_async_ep0 = NULL;
		return;
	}
}


static void finish_request_epx(
	struct akotg_usbhc		*akotghc,
	struct akotghc_ep	*ep,
	struct urb		*urb,
	int			status
) __releases(akotghc->lock) __acquires(akotghc->lock)
{
	unsigned		i;
	int is_out;
	is_out = usb_pipeout(urb->pipe);

	VDBG("Finishing EPx URB Request...\n");

	if (usb_pipecontrol(urb->pipe))
		ep->nextpid = USB_PID_SETUP;

	usb_hcd_unlink_urb_from_ep(akotg_usbhc_to_hcd(akotghc), urb);
	spin_unlock(&akotghc->lock);
	usb_hcd_giveback_urb(akotg_usbhc_to_hcd(akotghc), urb, status);
	spin_lock(&akotghc->lock);

	/* leave active endpoints in the schedule */
	if (!list_empty(&ep->hep->urb_list))
		return;

	
	/* async deschedule? */
	//if(epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo) && !list_empty(&ep->schedule)) {
	if (!list_empty(&ep->schedule)) {
		list_del_init(&ep->schedule);
		if (ep == akotghc->next_async_epx[ep->epfifo-1]) {
			akotghc->next_async_epx[ep->epfifo-1] = NULL;
		}
		return;
	}

	switch(usb_pipetype(urb->pipe)) {
	case PIPE_ISOCHRONOUS:
	case PIPE_INTERRUPT:
		/* periodic deschedule */
		HDBG("%s(): deschedule qh%d/%p branch %d\n", 
				__func__, ep->period, ep, ep->branch);
		for (i = ep->branch; i < PERIODIC_SIZE; i += ep->period) {
			struct akotghc_ep	*temp;
			struct akotghc_ep	**prev = &akotghc->periodic[i];

			while (*prev && ((temp = *prev) != ep))
				prev = &temp->next;
			if (*prev)
				*prev = ep->next;
			akotghc->load[i] -= ep->load;
		}
		ep->branch = PERIODIC_SIZE;
		akotghc->periodic_count--;
		
		akotg_usbhc_to_hcd(akotghc)->self.bandwidth_allocated
					-= ep->load / ep->period;
		if (ep == akotghc->next_periodic)
			akotghc->next_periodic = ep->next;

		/* we might turn SOFs back on again for the async schedule */
		if (akotghc->periodic_count == 0)
			sofirq_off(akotghc);
	}
}

static void
done(struct akotg_usbhc *akotghc, struct akotghc_ep *ep)
{
	int 			i;
	int			err_occurred = 0;
	int			epfifo;
	u8			status;
	u8			rxstatus;
	struct urb		*urb;
	unsigned int		pipe;
	int			is_out;
	int			urbstat = -EINPROGRESS;

	if (unlikely(!ep)) {
		return;
	}

	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
	pipe = urb->pipe;
	is_out = usb_pipeout(pipe);
	if (!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo))
		BUG();	/* Impossible, USB Device Endpoint *MUST* have been mapped to EPFIFO */

	status = hc_index_readw(epfifo, USB_REG_TXCSR1);
	rxstatus = hc_index_readw(epfifo, USB_REG_RXCSR1);

	//STALL
	if (((epfifo != 0) && ((status & (1 << 5)) || rxstatus & (1 << 6)))
		|| ((epfifo == 0) && (status & (1 << 2)))) {
		ep->error_count = 0;
		urbstat = -EPIPE;
		err_occurred = 1;
	}

    //NAK timeout
	if ((usb_pipebulk(pipe))
		&&((status & (1 << 7))||(rxstatus & (1 << 3)))) {
		if (!ep->period)
			ep->nak_count++;
		ep->error_count = 0;
		err_occurred = 1;
	}
	
    //Error, times expired
	if (((epfifo == 0) && (status & (1 << 4))) 
		||((epfifo != 0)&&((status || rxstatus) & (1 << 2))
		 &&(usb_pipeint(pipe)||usb_pipebulk(pipe)))) {
		urbstat = -EPROTO;
		ep->error_count = 0;
		err_occurred = 1;
	}

	if (err_occurred) {
		if (epfifo == 0)
			hc_index_writew(0, 0, USB_REG_TXCSR1);
		else if (is_out)
			if (urbstat == -EPIPE) 
				// clear stall bit and clear toggle
				hc_index_writew(epfifo, (1 << 6), USB_REG_TXCSR1);
			else
				hc_index_writew(epfifo, 0, USB_REG_TXCSR1);
		else
			if (urbstat == -EPIPE) 
				// clear stall bit and clear toggle
				hc_index_writew(epfifo, (1 << 7), USB_REG_RXCSR1);
			else
				hc_index_writew(epfifo, 0, USB_REG_RXCSR1);
	}
	else {
	
		struct usb_device	*udev = urb->dev;
		int			len;
		unsigned char		*buf;

		/* urb->iso_frame_desc is currently ignored here... */

		ep->nak_count = ep->error_count = 0;
		switch (ep->nextpid) {
		case USB_PID_OUT:
			urb->actual_length += ep->length;
			usb_dotoggle(udev, ep->epnum, 1);
			if (urb->actual_length == urb->transfer_buffer_length) {
				if (usb_pipecontrol(urb->pipe)) {
					VDBG("NEXT Packet: Status Packet.\n");
					ep->nextpid = USB_PID_ACK;
				}

				/* some bulk protocols terminate OUT transfers
				 * by a short packet, using ZLPs not padding.
				 */
				else if (ep->length < ep->maxpacket
						|| !(urb->transfer_flags & URB_ZERO_PACKET))
					urbstat = 0;
			}
			break;
		case USB_PID_IN:

#ifdef CONFIG_USB_AKOTG_DMA        
			if(ep->bdma){		        
                urb->actual_length += akotg_dma_get_trans_length(akotghc, epfifo);
                
                akotg_dma_clear(akotghc, epfifo);
                ep->bdma = false;
		    }
#endif

			buf = urb->transfer_buffer + urb->actual_length;
		    
			prefetchw(buf);
			len = hc_index_readw(epfifo, USB_REG_COUNT0);
			if (len > ep->length) {
				printk("   USB_PID_IN(OverFlow): len=%d, ep->length=%d\n",
					len, ep->length);
				len = ep->length;
				urbstat = -EOVERFLOW;
			}
			// read data from fifo
			urb->actual_length += len;
			for (i = 0; i < len; i++)
				*buf++ = hc_readb(ep_fifos[epfifo]);

			if (ep->epnum == 0) {
				u8 regval = hc_index_readb(epfifo, USB_REG_TXCSR1);
				regval &= ~USB_TXCSR1_TXPKTRDY;
				hc_index_writeb(epfifo, regval, USB_REG_TXCSR1);
			} else {
				u8 regval = hc_index_readb(epfifo, USB_REG_RXCSR1);
				regval &= ~USB_RXCSR1_RXPKTRDY;
				hc_index_writeb(epfifo, regval, USB_REG_RXCSR1);
			}
				
			usb_dotoggle(udev, ep->epnum, 0);
			if (urbstat == -EINPROGRESS &&
					(len < ep->maxpacket || 
					 urb->actual_length == urb->transfer_buffer_length)) {
				if (usb_pipecontrol(urb->pipe)) {
					VDBG("NEXT Packet: Status Packet.\n");
					ep->nextpid = USB_PID_ACK;
				}
				else
					urbstat = 0;
			}
			break;
		case USB_PID_SETUP:
			if (urb->transfer_buffer_length == urb->actual_length) {
				VDBG("NEXT Packet: Status Packet.\n");
				ep->nextpid = USB_PID_ACK;
			}
			else if (usb_pipeout(urb->pipe)) {
				VDBG("NEXT Packet: OUT Packet.\n");
				usb_settoggle(udev, 0, 1, 1);
				ep->nextpid = USB_PID_OUT;
			} else {
				VDBG("NEXT Packet: IN Packet.\n");
				usb_settoggle(udev, 0, 0, 1);
				ep->nextpid = USB_PID_IN;
			}
			break;
		case USB_PID_ACK:
			if (!is_out) {
				u8 regval = hc_index_readb(epfifo, USB_REG_RXCSR1);
				regval &= ~USB_RXCSR1_RXPKTRDY;
				hc_index_writeb(epfifo, regval, USB_REG_RXCSR1);
			}
			urbstat = 0;
			break;
		}
	}

	if (urbstat != -EINPROGRESS || urb->unlinked) {
		if (ep->epnum == 0)
			finish_request_ep0(akotghc, ep, urb, urbstat);
		else
			finish_request_epx(akotghc, ep, urb, urbstat);
	}
}

static int balance(struct akotg_usbhc *akotghc, u16 period, u16 load)
{
	int	i, branch = -ENOSPC;

	/* search for the least loaded schedule branch of that period
	 * which has enough bandwidth left unreserved.
	 */
	for (i = 0; i < period ; i++) {
		if (branch < 0 || akotghc->load[branch] > akotghc->load[i]) {
			int	j;

			for (j = i; j < PERIODIC_SIZE; j += period) {
				if ((akotghc->load[j] + load)
						> MAX_PERIODIC_LOAD)
					break;
			}
			if (j < PERIODIC_SIZE)
				continue;
			branch = i;
		}
	}
	return branch;
}

static void reset_otg(struct work_struct *work)
{
	unsigned long con;
	
	/* power up and set usb suspend bit */
	con = __raw_readl(USB_OP_MOD_REG);
	con &= ~(0x7 << 0);
	con |= (0x3 << 1);
	__raw_writel(con, USB_OP_MOD_REG);

	set_usb_as_host();	
	
	/* start fs host session*/ 
	hc_writeb(USB_DEVCTL_SESSION, USB_REG_DEVCTL);
}

#ifdef CONFIG_USB_AKOTG_DMA        

static bool handle_dma_irq(struct akotg_usbhc *akotghc,  struct akotg_dma *dma)
{
    u8      epfifo = dma->epfifo;
    u8      channel = dma->channel;
	u32     regtmp;
	u32     trans_len;
	int     urbstat = -EINPROGRESS;
	
    struct akotghc_ep *ep;
	struct urb		*urb;

    if(!akotghc || !dma)
        return false;
    
    if(USB_DIRECTION_RX == dma->dir){
        hc_index_writeb(epfifo, 0, USB_REG_RXCSR2);

        regtmp = hc_readl(USB_DMA_ADDR(channel));
        trans_len = regtmp - dma->addr;

        dma_unmap_single(NULL, dma->phyaddr, dma->count, DMA_FROM_DEVICE);
    }
    else{
        hc_index_writeb(epfifo, USB_TXCSR2_MODE, USB_REG_TXCSR2);

        trans_len = dma->count;

        dma_unmap_single(NULL, dma->phyaddr, dma->count, DMA_TO_DEVICE);
    }
    

    hc_writel(0, USB_DMA_COUNT(channel));

    //wait dma finish
    l2_combuf_wait_dma_finish(dma->l2buffer);

    //free l2 buffer
    akotg_free_l2_buffer(epfifo);
    dma->l2buffer = BUF_NULL;

    //change status
    dma->status = USB_DMA_CHANNEL_IDLE;

    //get urb
    ep = akotghc->active_epx[epfifo-1];

    ep->bdma = false;
	urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);

    urb->actual_length += trans_len;

    //done
    if((urb->actual_length == urb->transfer_buffer_length) || urb->unlinked)
    {
        urbstat = 0;
        finish_request_epx(akotghc, ep, urb, urbstat);
    }

    akotghc->active_epx[epfifo-1] = NULL;

    return true;

}

irqreturn_t akotg_dma_irq(int irqnum, void *__hcd)
{
	struct usb_hcd		*hcd = __hcd;
	unsigned long		flags;
	irqreturn_t		rc;

    local_irq_save(flags);

	if (unlikely(hcd->state == HC_STATE_HALT ||
		     !test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags))) {
		rc = IRQ_NONE;
	} else if (akotg_usbhc_irq(hcd) == IRQ_NONE) {
		rc = IRQ_NONE;
	} else {
		set_bit(HCD_FLAG_SAW_IRQ, &hcd->flags); // ???

		if (unlikely(hcd->state == HC_STATE_HALT))
			usb_hc_died(hcd);
		rc = IRQ_HANDLED;
	}

    //rc = akotg_usbhc_irq(hcd);

    local_irq_restore(flags);

    return rc;
}

#endif

irqreturn_t akotg_usbhc_irq(struct usb_hcd *hcd)
{
	struct akotg_usbhc	*akotghc = hcd_to_akotg_usbhc(hcd);
	irqreturn_t	ret = IRQ_NONE;
	struct urb *urb;
	struct akotghc_ep *ep;
	int epnum[MAX_EP_NUM + 1] = { 0 };
	int i;
	unsigned index = 0;
	
	char rINTCOM;
	unsigned short rINTTX, rINTRX;

	u32 rINTDMA;

#ifdef CONFIG_USB_AKOTG_DMA        
	struct akotg_dma *dma;
#endif

	spin_lock(&akotghc->lock);

	/*Read & Clear all interrupt status.*/
	rINTCOM = hc_readb(USB_REG_INTRUSB);
	rINTTX =hc_readw(USB_REG_INTRTX);
	rINTRX = hc_readw(USB_REG_INTRRX);
	rINTDMA =hc_readl(USB_DMA_INTR);

	if (rINTTX &USB_INTR_EP0) {
		epnum[0] = 1;
		done(akotghc, akotghc->active_ep0);
		akotghc->active_ep0 = NULL;
	}

#ifdef CONFIG_USB_AKOTG_DMA        
	if(rINTDMA)
	{
	    //printk("|%x|", rINTDMA);
        //printk("C[%x,%x]", hcd, m_hcd);
	    for(i = 0; i < USBDMA_CHANNEL_NUM; i++)
	    {
	        if(rINTDMA & (1<<i))
	        {
	            dma = akotg_dma_get_struct(akotghc, i);
	            handle_dma_irq(akotghc, dma);
	        }
	    }
	}
#endif

	for(i=0; i<MAX_EP_NUM; i++)
		if((rINTTX & (1<<(i+1))) || (rINTRX & (1<<(i+1)))) {
			epnum[i + 1] = 1;
			done(akotghc, akotghc->active_epx[i]);
			akotghc->active_epx[i] = NULL;
		}

	if (rINTCOM & USB_INTR_SOF) { 
		index = akotghc->frame++ & (PERIODIC_SIZE - 1);
		

		/* be graceful about almost-inevitable periodic schedule
		 * overruns:  continue the previous frame's transfers iff
		 * this one has nothing scheduled.
		 */
	
		if (akotghc->periodic[index]) {
			akotghc->next_periodic = akotghc->periodic[index];
		}
	}

	/* manages debouncing and wakeup */
	if(rINTCOM & (USB_INTR_CONNECT | USB_INTR_DISCONNECT)) {

		/* most stats are reset for each VBUS session */

		/* usbcore nukes other pending transactions on disconnect */
		if (akotghc->active_ep0) {
			VDBG("Finishing EP0 Active URBs...\n");
			ep = akotghc->active_ep0;
			hc_index_writeb(ep->epfifo, 0, USB_REG_TXCSR1);
			hc_index_writeb(ep->epfifo, USB_CSR02_FLUSHFIFO, USB_REG_TXCSR2);

			finish_request_ep0(akotghc, akotghc->active_ep0,
					container_of(akotghc->active_ep0->hep->urb_list.next, struct urb, urb_list), 
					-ESHUTDOWN);
			akotghc->active_ep0 = NULL;
		}

		for(i=0; i<MAX_EP_NUM; i++) 
			if (akotghc->active_epx[i]) {
				VDBG("Finishing EPx Active URBs...\n");
				ep = akotghc->active_epx[i];
				urb = container_of(ep->hep->urb_list.next, struct urb, urb_list);
				//if (!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo))
				//	BUG();
				if (usb_pipeout(urb->pipe)) {
					hc_index_writeb(ep->epfifo, USB_TXCSR1_FLUSHFIFO, USB_REG_TXCSR1);
				} else {
					hc_index_writeb(ep->epfifo, USB_RXCSR1_FLUSHFIFO, USB_REG_RXCSR1);
				}

				finish_request_epx(akotghc, akotghc->active_epx[i],
						container_of(akotghc->active_epx[i]->hep->urb_list.next, struct urb, urb_list),
						-ESHUTDOWN);
				akotghc->active_epx[i] = NULL;
			}

		/* port status seems weird until after reset, so
		 * force the reset and make khubd clean up later.
		 */
		if (rINTCOM & USB_INTR_CONNECT) {
			akotghc->port_status |= 1 << USB_PORT_FEAT_CONNECTION;
			akotghc->port_status |= 1 << USB_PORT_FEAT_C_CONNECTION;
		} else {

#ifdef CONFIG_USB_AKOTG_DMA        
            //clear all dma
            akotg_dma_clear(akotghc, 0);
#endif
			period_epfifo = 0;
			akotghc->port_status &= ~(1 << USB_PORT_FEAT_CONNECTION);
			akotghc->port_status |= 1 << USB_PORT_FEAT_C_CONNECTION;
			init_epfifo_mapping(&akotg_epfifo_mapping);
			reset_endpoints();
			REG32(USB_OP_MOD_REG) &= ~(0xff << 6);
			REG32(USB_OP_MOD_REG) |= (0x1f << 6);			
			hc_writeb(0x0, USB_REG_DEVCTL);
			hc_writeb(0x0, USB_REG_FADDR);
	
			queue_delayed_work(g_otghc_wq, &g_otg_rest, 0);
		}
	}

	if (rINTCOM & USB_INTR_RESUME) {
		if (akotghc->port_status & (1 << USB_PORT_FEAT_SUSPEND)) {
			HDBG("wakeup\n");
			akotghc->port_status |= 1 << USB_PORT_FEAT_C_SUSPEND;
			
		}
		rINTCOM &= ~(USB_INTR_RESUME);
	}
	
	if (akotghc->port_status & (1 << USB_PORT_FEAT_ENABLE)) {
		if (epnum[0]) {
			start_transfer(akotghc, 0);
			ret = IRQ_HANDLED;
		}
		if((rINTCOM & USB_INTR_SOF) && akotghc->periodic[index] && period_epfifo){
			start_transfer(akotghc, period_epfifo);
			ret = IRQ_HANDLED;
		}
		for(i = 0; i < MAX_EP_NUM; i++) {
			if(epnum[i + 1]) {
				start_transfer(akotghc, i + 1);
				ret = IRQ_HANDLED;
			}
		}
		
#ifdef CONFIG_USB_AKOTG_DMA        
        for(i = 0; i < USBDMA_CHANNEL_NUM; i++)
        {
            if(rINTDMA & (1<<i))
            {
                dma = akotg_dma_get_struct(akotghc, i);
                start_transfer(akotghc, dma->epfifo);
                ret = IRQ_HANDLED;
            }
        }
#endif
	}

	if(akotghc->periodic_count == 0 && list_empty(&akotghc->async_ep0)) {
		for(i = 0; i < MAX_EP_NUM; i++) {
			if(!list_empty(&akotghc->async_epx[i]))
				break;
		}
		if(i == MAX_EP_NUM)
			sofirq_off(akotghc);
	}
	
	spin_unlock(&akotghc->lock);
	
	return ret;
}
EXPORT_SYMBOL(akotg_usbhc_irq);

int akotg_usbhc_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status)
{
	struct akotg_usbhc		*akotghc = hcd_to_akotg_usbhc(hcd);
	struct usb_host_endpoint *hep;
	unsigned long		flags;
	struct akotghc_ep	*ep;
	int			retval, i;
	int			is_out = usb_pipeout(urb->pipe);
	
	HDBG("Dequeue: Direction=%s, Type=%s, EP Num=%d, urb=%p\n",
		is_out ? "OUT" : "IN", trans_desc[usb_pipetype(urb->pipe)], 
		usb_pipeendpoint(urb->pipe), urb);
	
	spin_lock_irqsave(&akotghc->lock, flags);

	retval = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (retval) {
		printk("Dequeue: check and unlink urb failed!\n");
		goto fail;
	}

	hep = urb->hcpriv;
	ep = hep->hcpriv;
	if (ep) {
		/* finish right away if this urb can't be active ...
		 * note that some drivers wrongly expect delays
		 */
		if (ep->hep->urb_list.next != &urb->urb_list) {
			/* not front of queue?  never active */

		/* for active transfers, we expect an IRQ */
		} else if (akotghc->active_ep0 == ep) {
			if (time_before_eq(akotghc->jiffies_ep0, jiffies)) {
				hc_index_writeb(0, 0, USB_REG_TXCSR1);
				hc_index_writeb(0, 1 << 0, USB_REG_TXCSR2);
				akotghc->active_ep0 = NULL;
			} else
				urb = NULL;
			
		} else {
			for(i=0; i<MAX_EP_NUM; i++) {
				if(akotghc->active_epx[i] == ep) {
					if(time_before_eq(akotghc->jiffies_epx[i], jiffies)) {
						//if(!epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo))
						//	BUG();
						if (is_out) {
							hc_index_writeb(ep->epfifo, USB_TXCSR1_FLUSHFIFO, USB_REG_TXCSR1);
						} else {
							hc_index_writeb(ep->epfifo, USB_RXCSR1_FLUSHFIFO, USB_REG_RXCSR1);
						}
						akotghc->active_epx[i] = NULL;
					} else
						urb = NULL;
					
				}
			}
			/* front of queue for inactive endpoint */
		}

		if (urb) {
			if (akotghc->active_ep0 == ep)
				finish_request_ep0(akotghc, ep, urb, 0);
			else
				finish_request_epx(akotghc, ep, urb, 0);
		} else {
			HDBG("dequeue, urb %p active, wait for irq.\n", urb);
		}
	} else
		retval = -EINVAL;
 fail:
	spin_unlock_irqrestore(&akotghc->lock, flags);
	
	return retval;
}



int 
akotg_usbhc_urb_enqueue(
	struct usb_hcd		*hcd,
	struct urb		*urb,
	gfp_t			mem_flags
) 
{
	struct akotg_usbhc		*akotghc = hcd_to_akotg_usbhc(hcd);
	struct usb_device	*udev = urb->dev;
	int			is_out = usb_pipeout(urb->pipe);
	int			type = usb_pipetype(urb->pipe);
	int			epnum = usb_pipeendpoint(urb->pipe);
	int			epfifo = 0;

	struct akotghc_ep	*ep = NULL;
	unsigned long		flags;
	int			i;
	int			retval;
	struct usb_host_endpoint *hep = urb->ep;

	HDBG("Enqueue: Direction=%s, Type=%s, EP Num=%d, urb=%p"
		" urb->transfer_buffer_length=%d\n",
		is_out ? "OUT" : "IN", trans_desc[type], epnum, urb,
		urb->transfer_buffer_length);
	
	if (usb_pipeisoc(urb->pipe))
		return -ENOSPC;

	spin_lock_irqsave(&akotg_epfifo_mapping.lock,flags);
	if (!__is_epnum_mapped(&akotg_epfifo_mapping, epnum, is_out)) {
		if (!__map_epnum_to_epfifo(&akotg_epfifo_mapping, epnum, is_out, &epfifo)) {
			spin_unlock_irqrestore(&akotg_epfifo_mapping.lock, flags);
			return -ENOSPC;
		}
		
		if (epnum != 0) {

			int eptype = get_ep_type(urb->pipe);
			
			if (is_out) {
				disable_epx_tx_interrupt(epfifo);
				set_epx_tx_type(epfifo, epnum, eptype);
				hc_index_writew(epfifo, hep->desc.wMaxPacketSize, USB_REG_TXMAXP);
				hc_index_writeb(epfifo, 16, USB_REG_TXINTERVAL);	/* Tx Interval */
				clear_epx_tx_data_toggle(epfifo);
				set_epx_tx_mode(epfifo);
				flush_epx_tx_fifo(epfifo);
				enable_epx_tx_interrupt(epfifo);
			} else {
				disable_epx_rx_interrupt(epfifo);
				set_epx_rx_type(epfifo, epnum, eptype);
				hc_index_writew(epfifo, hep->desc.wMaxPacketSize, USB_REG_RXMAXP);
				if (usb_endpoint_xfer_isoc(&hep->desc) || usb_endpoint_xfer_int(&hep->desc))
					hc_index_writeb(epfifo, hep->desc.bInterval, USB_REG_RXINTERVAL);
				else
					hc_index_writeb(epfifo, 0, USB_REG_RXINTERVAL);
				set_epx_rx_mode(epfifo);
				clear_epx_rx_data_toggle(epfifo);
				flush_epx_rx_fifo(epfifo);
				enable_epx_rx_interrupt(epfifo);
			}
		}
	} else {
		if(!epnum_to_epfifo(&akotg_epfifo_mapping, epnum, is_out, &epfifo))
			BUG();
	}

	spin_unlock_irqrestore(&akotg_epfifo_mapping.lock, flags);

	spin_lock_irqsave(&akotghc->lock, flags);

	/* don't submit to a dead or disabled port */
	if (!(akotghc->port_status & (1 << USB_PORT_FEAT_ENABLE))
		|| !HC_IS_RUNNING(hcd->state)) {
		retval = -ENODEV;
		goto fail_not_linked;
	}

	retval = usb_hcd_link_urb_to_ep(hcd, urb);
	if (retval) {
		goto fail_not_linked;
	}
	
	/* avoid all allocations within spinlocks */
	if (hep->hcpriv) {
		ep = hep->hcpriv;
	} else {
		ep = kzalloc(sizeof *ep, mem_flags);
		if (ep == NULL) {
			retval = -ENOMEM;
			goto fail;
		}
		
		INIT_LIST_HEAD(&ep->schedule);
		ep->udev = udev;
		ep->epnum = epnum;
		ep->epfifo = epfifo;
		ep->maxpacket = usb_maxpacket(udev, urb->pipe, is_out);
		
		usb_settoggle(udev, epnum, is_out, 0);

		if (type == PIPE_CONTROL)
			ep->nextpid = USB_PID_SETUP;
		else if (is_out)
			ep->nextpid = USB_PID_OUT;
		else
			ep->nextpid = USB_PID_IN;

		if (ep->maxpacket > H_MAXPACKET) {
			/* iso packets up to 240 bytes could work... */
			HDBG("dev %d ep%d maxpacket %d\n",
				udev->devnum, epnum, ep->maxpacket);			
			retval = -EINVAL;
			goto fail;
		}

		switch (type) {
		case PIPE_ISOCHRONOUS:
		case PIPE_INTERRUPT:
			if (urb->interval > PERIODIC_SIZE)
				urb->interval = PERIODIC_SIZE;
			ep->period = urb->interval;
			ep->branch = PERIODIC_SIZE;
			ep->load = usb_calc_bus_time(udev->speed, !is_out,
				(type == PIPE_ISOCHRONOUS),
				usb_maxpacket(udev, urb->pipe, is_out))	/ 1000;
			period_epfifo = epfifo;
			break;
		}

		ep->hep = hep;
		hep->hcpriv = ep;
	}

	/* maybe put endpoint into schedule */
	switch (type) {
	case PIPE_CONTROL:
	case PIPE_BULK:
		if (list_empty(&ep->schedule)) {
			if (epnum == 0) 
				list_add_tail(&ep->schedule, &akotghc->async_ep0);
			else 
				list_add_tail(&ep->schedule, &akotghc->async_epx[epfifo-1]);
		}
		break;
	case PIPE_ISOCHRONOUS:
	case PIPE_INTERRUPT:
		urb->interval = ep->period;
		if (ep->branch < PERIODIC_SIZE) {
			/* NOTE:  the phase is correct here, but the value
			 * needs offsetting by the transfer queue depth.
			 * All current drivers ignore start_frame, so this
			 * is unlikely to ever matter...
			 */
			urb->start_frame = (akotghc->frame & (PERIODIC_SIZE - 1))
						+ ep->branch;
			break;
		}

		retval = balance(akotghc, ep->period, ep->load);
		if (retval < 0)
			goto fail;
		ep->branch = retval;
		retval = 0;
		urb->start_frame = (akotghc->frame & (PERIODIC_SIZE - 1))
					+ ep->branch;

		/* sort each schedule branch by period (slow before fast)
		 * to share the faster parts of the tree without needing
		 * dummy/placeholder nodes
		 */
		VDBG("schedule qh%d/%p branch %d\n", ep->period, ep, ep->branch);
		for (i = ep->branch; i < PERIODIC_SIZE; i += ep->period) {
			struct akotghc_ep	**prev = &akotghc->periodic[i];
			struct akotghc_ep	*here = *prev;

			while (here && ep != here) {
				if (ep->period > here->period)
					break;
				prev = &here->next;
				here = *prev;
			}
			if (ep != here) {
				ep->next = here;
				*prev = ep;
			}
			akotghc->load[i] += ep->load;
		}
		akotghc->periodic_count++;
		hcd->self.bandwidth_allocated += ep->load / ep->period;
		sofirq_on(akotghc);
	}

	urb->hcpriv = hep;
	start_transfer(akotghc, epfifo);
  
fail:
	if (retval)
		usb_hcd_unlink_urb_from_ep(hcd, urb);
fail_not_linked:
	spin_unlock_irqrestore(&akotghc->lock, flags);
	return retval;
}


void
akotg_usbhc_endpoint_reset(struct usb_hcd *hcd, struct usb_host_endpoint *hep)
{
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);
	int			epnum = usb_endpoint_num(&hep->desc);
	unsigned long		flags;

	spin_lock_irqsave(&akotghc->lock, flags);

	HDBG("Resetting EP %d, Type=%s, Dir=%s\n",
		epnum, xfer_name[usb_endpoint_type(&hep->desc)], usb_endpoint_dir_out(&hep->desc)? "OUT" : "IN");

	if (epnum == 0) {
		hc_index_writeb(0, 0, USB_REG_TXINTERVAL);
		hc_index_writew(0, 0, USB_REG_TXCSR1);
		flush_ep0_fifo();
		enable_ep0_interrupt();
	} else {

	}

	spin_unlock_irqrestore(&akotghc->lock, flags);
}

void
akotg_usbhc_endpoint_disable(struct usb_hcd *hcd, struct usb_host_endpoint *hep)
{
	struct akotghc_ep	*ep = hep->hcpriv;

	int			epnum = usb_endpoint_num(&hep->desc);
	int			is_out = usb_endpoint_dir_out(&hep->desc);

	int			epfifo = 0;


	if (!ep) {
		return;
	}

	if (is_epnum_mapped(&akotg_epfifo_mapping, epnum, is_out)) {

		disable_ep_interrupt(epfifo);
		flush_ep_fifo(epfifo);
	}
	/* assume we'd just wait for the irq */
	if (!list_empty(&hep->urb_list))
		msleep(3);
	if (!list_empty(&hep->urb_list))
		HDBG("ep %p not empty?\n", ep);

	kfree(ep);
	hep->hcpriv = NULL;

}

int
akotg_usbhc_get_frame(struct usb_hcd *hcd)
{
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);

	/* wrong except while periodic transfers are scheduled;
	 * never matches the on-the-wire frame;
	 * subject to overruns.
	 */
	return akotghc->frame;
}


/*-------------------------------------------------------------------------*/

/* the virtual root hub timer IRQ checks for hub status */
int
akotg_usbhc_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);
	unsigned long flags;

	/* non-SMP HACK: use root hub timer as i/o watchdog
	 * this seems essential when SOF IRQs aren't in use...
	 */
	local_irq_save(flags);
	if (!timer_pending(&akotghc->timer)) {
		if (akotg_usbhc_irq( /* ~0, */ hcd) != IRQ_NONE)
			;//akotghc->stat_lost++;
	}
	local_irq_restore(flags);

	if (!(akotghc->port_status & (0xffff << 16))) {
		return 0;
	}

	/* tell khubd port 1 changed */
	*buf = (1 << 1);

	return 1;
}

void
akotg_usbhc_hub_descriptor (
	struct akotg_usbhc *akotghc,
	struct usb_hub_descriptor	*desc
) {
	u16		temp = 0;
	
	desc->bDescriptorType = 0x29;
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = 1;
	desc->bDescLength = 9;

	/* per-port power switching (gang of one!), or none */
	desc->bPwrOn2PwrGood = 0;

	/* no over current errors detection/handling */
	temp |= HUB_CHAR_COMMON_LPSM|HUB_CHAR_NO_OCPM; 

	desc->wHubCharacteristics = cpu_to_le16(temp);

	/* two bitmaps:  ports removable, and legacy PortPwrCtrlMask */
	desc->u.hs.DeviceRemovable[0] = 0 << 1;
	desc->u.hs.DeviceRemovable[1] = ~0;
}

void
akotg_usbhc_timer(unsigned long _akotghs)
{
	struct akotg_usbhc *akotghc = (void *) _akotghs;
	unsigned long	flags;
	const u32	mask = (1 << USB_PORT_FEAT_CONNECTION)
				| (1 << USB_PORT_FEAT_ENABLE);
	

	spin_lock_irqsave(&akotghc->lock, flags);

	if (akotghc->port_status & USB_PORT_STAT_RESET) {
		akotghc->port_status = (1 << USB_PORT_FEAT_C_RESET)
				| (1 << USB_PORT_FEAT_POWER);
		akotghc->port_status |= mask;
		
		if (akotghc->port_status & (1 << USB_PORT_FEAT_CONNECTION)) {
			if ((hc_readb(USB_REG_DEVCTL) & USB_DEVCTL_FSDEV)) {
				akotghc->port_status |= USB_PORT_STAT_HIGH_SPEED; 
			} else {
				/* Plug-in & plug-out quickly could lead to this... */
				akotghc->port_status &= ~mask;
			}
		}
	} else {
		/* NOT IMPLEMENTED YET */
		BUG();
	}
	

	spin_unlock_irqrestore(&akotghc->lock, flags);

}

int
akotg_usbhc_hub_control(
	struct usb_hcd	*hcd,
	u16		typeReq,
	u16		wValue,
	u16		wIndex,
	char		*buf,
	u16		wLength
) {
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);
	int		retval = 0;
	unsigned long	flags;
	char reg8val;
	
	HDBG("%s(): typeReq=0x%x, wValue=%d, wIndex=%d, wLength=%d\t",
			__func__, typeReq, wValue, wIndex, wLength);

	spin_lock_irqsave(&akotghc->lock, flags);
	
	switch (typeReq) {
	case ClearHubFeature:
	case SetHubFeature:
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
		case C_HUB_LOCAL_POWER:
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		if (wIndex != 1 || wLength != 0)
			goto error;
		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			HDBG("ClearPortFeature: USB_PORT_FEAT_ENABLE\n");
			akotghc->port_status &= (1 << USB_PORT_FEAT_POWER);
			break;
		case USB_PORT_FEAT_SUSPEND:
			HDBG("ClearPortFeature: USB_PORT_FEAT_SUSPEND\n");
			if (!(akotghc->port_status & (1 << USB_PORT_FEAT_SUSPEND)))
				break;

			/* 20 msec of resume/K signaling, other irqs blocked */
			HDBG("    start resume...\n");
			hc_writeb(0x0, USB_REG_INTRUSBE); 
			reg8val = hc_readb(USB_REG_POWER); 
			reg8val |= USB_POWER_RESUME;
			hc_writeb(reg8val, USB_REG_POWER);
			
			mod_timer(&akotghc->timer, jiffies + msecs_to_jiffies(20));
			break;
		case USB_PORT_FEAT_POWER:
			port_power(akotghc, 0);
			break;
		case USB_PORT_FEAT_C_ENABLE:
		case USB_PORT_FEAT_C_SUSPEND:
		case USB_PORT_FEAT_C_CONNECTION:
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
		case USB_PORT_FEAT_C_RESET:
			break;
		default:
			goto error;
		}
		akotghc->port_status &= ~(1 << wValue);
		break;
	case GetHubDescriptor:
		akotg_usbhc_hub_descriptor(akotghc, (struct usb_hub_descriptor *) buf);
		break;
	case GetHubStatus:
		put_unaligned_le32(0, buf);
		break;
	case GetPortStatus:
		if (wIndex != 1)
			goto error;
		put_unaligned_le32(akotghc->port_status, buf);
		if (*(u16*)(buf+2)) /* only if wPortChange is interesting */
			HDBG("	GetPortStatus 0x%04x\n", akotghc->port_status);
		break;
	case SetPortFeature:
		if (wIndex != 1 || wLength != 0)
			goto error;
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			HDBG("SetPortFeature: USB_PORT_FEAT_SUSPEND\n");
			if (akotghc->port_status & (1 << USB_PORT_FEAT_RESET))
				goto error;
			if (!(akotghc->port_status & (1 << USB_PORT_FEAT_ENABLE)))
				goto error;
			/*to suspend the usb host controller.*/
			reg8val = hc_readb(USB_REG_POWER);
			reg8val |= USB_POWER_SUSPENDM;
			hc_writeb(reg8val, USB_REG_POWER);
			break;
		case USB_PORT_FEAT_POWER: 
			HDBG("SetPortFeature: USB_PORT_FEAT_POWER\n");
			port_power(akotghc, 1);
			break;
		case USB_PORT_FEAT_RESET:
			HDBG("SetPortFeature: USB_PORT_FEAT_RESET, Port Status=0x%04x\n", 
					akotghc->port_status);
			if (akotghc->port_status & (1 << USB_PORT_FEAT_SUSPEND)) {
				HDBG("    USB_PORT_FEAT_SUSPEND ....\n");
				goto error;
			}
			if (!(akotghc->port_status & (1 << USB_PORT_FEAT_POWER))) {
				HDBG("    USB_PORT_FEAT_POWER NOT SET.\n");
				break;
			}
			clear_all_interrupts();

			/* 50 msec of reset/SE0 signaling, irqs blocked */
			/*reset device.*/
			reg8val = hc_readb(USB_REG_POWER);
			reg8val |= USB_POWER_RESET;
			hc_writeb(reg8val, USB_REG_POWER);
			mdelay(30);
			reg8val &= ~USB_POWER_RESET;
			hc_writeb(reg8val, USB_REG_POWER);

			hc_writeb(0xF7, USB_REG_INTRUSBE);
			
			akotghc->port_status |= (1 << USB_PORT_FEAT_RESET);
			mod_timer(&akotghc->timer, jiffies + msecs_to_jiffies(50));
			break;
		default:
			goto error;
		}

		akotghc->port_status |= 1 << wValue;
		break;
error:
		/* "protocol stall" on error */
		retval = -EPIPE;
	}

	spin_unlock_irqrestore(&akotghc->lock, flags);
	return retval;
}


 int
akotg_usbhc_bus_suspend(struct usb_hcd *hcd)
{
	u8 reg;
	unsigned long flags;
	msleep(10);
	local_irq_save(flags);
	reg = hc_readb(USB_REG_POWER);
	reg |= USB_POWER_SUSPENDM;
	hc_writeb(reg, USB_REG_POWER);
	local_irq_restore(flags);
	msleep(20);
	return 0;
}

 int
akotg_usbhc_bus_resume(struct usb_hcd *hcd)
{
	u8 reg;
	unsigned long flags;
	local_irq_save(flags);
	reg = hc_readb(USB_REG_POWER);
	reg |= USB_POWER_RESUME;
	hc_writeb(reg, USB_REG_POWER);
	local_irq_restore(flags);
	msleep(20);
	local_irq_save(flags);
	reg = hc_readb(USB_REG_POWER);
	reg &= ~USB_POWER_RESUME;
	hc_writeb(reg, USB_REG_POWER);
	local_irq_restore(flags);
	msleep(100);
	return 0;
}

void akotg_usbhc_stop(struct usb_hcd *hcd)
{
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);
	unsigned long	flags;
	
	del_timer_sync(&hcd->rh_timer);
	clk_disable(akotghc->clk);
	
	/* reset usb phy */
	usb_reset_phy(akotghc);

	//rMULFUN_CON2 &= ~(0x1 << 18);	//iddig is invalid
	//REG32(USB_OP_MOD_REG) |= (0x3 << 12);
		
	spin_lock_irqsave(&akotghc->lock, flags);
	port_power(akotghc, 0);
	spin_unlock_irqrestore(&akotghc->lock, flags);
}

static void usb_hwinit_control(struct akotg_usbhc *otghc)
{
	unsigned long flags;
	
	spin_lock_irqsave(&otghc->lock, flags);

	port_power(otghc, 1);
	set_usb_as_host();
	/* reset usb phy */
	usb_reset_phy(otghc);

	clear_all_interrupts();
	reset_endpoints();
	hc_writeb(0x0, USB_REG_FADDR);
	
	usb_power_up(otghc);
	
	hc_writeb(USB_POWER_ENSUSPEND|USB_HOSG_HIGH_SPEED, USB_REG_POWER);
	hc_writeb(0xF7, USB_REG_INTRUSBE);
		
	spin_unlock_irqrestore(&otghc->lock, flags);
}

int akotg_usbhc_start(struct usb_hcd *hcd)
{
	struct akotg_usbhc *akotghc = hcd_to_akotg_usbhc(hcd);
	
	g_otghc_wq = create_singlethread_workqueue("usb_otg_wq");
	if (!g_otghc_wq)
		goto err_otghc_queue;
	INIT_DELAYED_WORK(&g_otg_rest, reset_otg);
	/*after reset usbhc, set stat to running.*/
	hcd->state = HC_STATE_RUNNING;
	
	/* chip has been reset, VBUS power is off */
	disable_irq(akotghc->mcu_irq);
	//disable_irq(akotghc->dma_irq);

	clk_enable(akotghc->clk);

	usb_hwinit_control(akotghc);
	
	/* start host session*/ // is end session ?
	hc_writeb(USB_DEVCTL_SESSION, USB_REG_DEVCTL);

	enable_irq(akotghc->mcu_irq);
	//enable_irq(akotghc->dma_irq);

	return 0;
err_otghc_queue:
	printk(KERN_ERR "akotg_usbhc couldn't create workqueue\n");
	return -ENOMEM;		
}

