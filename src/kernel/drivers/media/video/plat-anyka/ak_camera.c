/*
  * @file ak camera.c
  * @camera host driver for ak
  * @Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co
  * @author wu_daochao
  * @date 2011-04
  * @version 
  * @for more information , please refer to AK980x Programmer's Guide Mannul
  */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>

#include <asm/io.h>
#include <asm/cacheflush.h>

#include <media/soc_camera.h>
#include <media/videobuf-core.h>
#include <media/videobuf-dma-contig.h>
#include <media/soc_mediabus.h>

#include <mach/gpio.h>
#include <mach/clock.h>
#include <mach/reset.h>
#include <plat-anyka/ak_camera.h>
#include <plat-anyka/aksensor.h>
#include <plat-anyka/isp_interface.h>

#include "ak39_isp.h"


//#define CAMIF_DEBUG
#ifdef CAMIF_DEBUG
#define isp_dbg(fmt...)			printk(KERN_INFO " ISP: " fmt)
#else
#define isp_dbg(fmt, args...)	do{}while(0)
#endif 

#define CAMDBG(fmt, args...)	do{}while(0)

struct ak_buffer {
	struct videobuf_buffer vb;
	enum v4l2_mbus_pixelcode	code;
	int				inwork;
};

struct ak_camera_dev {
	struct soc_camera_host soc_host;
	struct soc_camera_device *icd;
	struct ak_camera_pdata		*pdata;

	void __iomem	*base;		// mapped baseaddress for CI register(0x2000c000)
	struct resource *res;
	struct clk	*clk;		// camera controller clk. it's parent is vclk defined in clock.c
	struct clk	*cis_sclk;		// cis_sclk clock for sensor
	unsigned int	irq;
	unsigned int dma_running;
	/* members to manage the dma and buffer*/
	struct list_head capture;	
	struct ak_buffer	*active;
	spinlock_t		lock;  /* for videobuf_queue , passed in init_videobuf */
	spinlock_t		rfled_lock;
	
	/* personal members for platform relative */
	unsigned long mclk;

	struct isp_struct isp;
	enum isp_working_mode def_mode;
	unsigned char *osd_swbuff;
	unsigned char *osd_buff;
	
	struct timer_list timer;
};

struct ak_camera_cam {
	/* Client output, as seen by the CEU */
	unsigned int width;
	unsigned int height;
};

#define	ISP_TIMEOUT				(1)	//unit: s
#define EMPTY_FRAME_NUM			(2)

static const char *ak_cam_driver_description = "AK_Camera";
static int irq_buf_empty_flag = 0;
static int irq_need_baffle = 0;

extern void *getRecordSyncSamples(void);

/**
 * @brief:  for ak_videobuf_release, free buffer if camera stopped.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *vq: V4L2 buffer queue information structure
 * @param [in] *buf: ak camera drivers structure, include struct videobuf_buffer 
 */
static void free_buffer(struct videobuf_queue *vq, struct ak_buffer *buf)
{
	struct soc_camera_device *icd = vq->priv_data;
	struct videobuf_buffer *vb = &buf->vb;
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;	

	isp_dbg("%s (vb=0x%p) buf[%d] 0x%08lx %d\n", 
			__func__, vb, vb->i, vb->baddr, vb->bsize);
	
	BUG_ON(in_interrupt());

	/* This waits until this buffer is out of danger, i.e., until it is no
	 * longer in STATE_QUEUED or STATE_ACTIVE */
	if (vb->state == VIDEOBUF_ACTIVE && !pcdev->dma_running) {
		printk("free_buffer: dma_running=%d, doesn't neee to wait\n", pcdev->dma_running);
		//vb->state = VIDEOBUF_ERROR;
		list_del(&vb->queue);
	} else {
		vb->state = VIDEOBUF_DONE;
		videobuf_waiton(vq, vb, 0, 0);
	}
	videobuf_dma_contig_free(vq, vb);

	vb->state = VIDEOBUF_NEEDS_INIT;
}
static void rfled_timer(unsigned long _data);

/**
 * @brief:  Called when application apply buffers, camera buffer initial.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *vq: V4L2 buffer queue information structure
 * @param [in] *count: buffer's number
 * @param [in] *size: buffer total size
 */
static int ak_videobuf_setup(struct videobuf_queue *vq, unsigned int *count, 
								unsigned int *size)
{
	struct soc_camera_device *icd = vq->priv_data;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						icd->current_fmt->host_fmt);

	bytes_per_line = icd->user_width * 3 /2;
	if (bytes_per_line < 0)
		return bytes_per_line;

	*size = bytes_per_line * icd->user_height;

	if (*count < 4) {
		printk("if use video mode, vbuf num isn't less than 4\n");
		*count = 4;
	}

	if (*size * *count > CONFIG_VIDEO_RESERVED_MEM_SIZE)
		*count = (CONFIG_VIDEO_RESERVED_MEM_SIZE) / *size;
	
	isp_dbg("%s count=%d, size=%d, bytes_per_line=%d\n",
			__func__, *count, *size, bytes_per_line);
	
	return 0;
}

/**
 * @brief: Called when application apply buffers, camera buffer initial.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *vq: V4L2  buffer queue information structure
 * @param [in] *vb: V4L2  buffer information structure
 * @param [in] field: V4L2_FIELD_ANY 
 */
static int ak_videobuf_prepare(struct videobuf_queue *vq,
			struct videobuf_buffer *vb, enum v4l2_field field)
{
	struct soc_camera_device *icd = vq->priv_data;
	struct ak_buffer *buf = container_of(vb, struct ak_buffer, vb);
	int ret;
	int bytes_per_line = soc_mbus_bytes_per_line(icd->user_width,
						icd->current_fmt->host_fmt);

	isp_dbg("%s (vb=0x%p) buf[%d] vb->baddr=0x%08lx vb->bsize=%d bytes_per_line=%d\n",
			__func__, vb, vb->i, vb->baddr, vb->bsize, bytes_per_line);

	bytes_per_line = icd->user_width * 3 /2;

	if (bytes_per_line < 0)
		return bytes_per_line;

	/* Added list head initialization on alloc */
	WARN_ON(!list_empty(&vb->queue));

#if 0
//#ifdef ISP_DEBUG	
	/*	 
	* This can be useful if you want to see if we actually fill	 
	* the buffer with something	 
	*/
	memset((void *)vb->baddr, 0xaa, vb->bsize);
#endif

	BUG_ON(NULL == icd->current_fmt);
	
	/* I think, in buf_prepare you only have to protect global data,
	 * the actual buffer is yours */
	buf->inwork = 1;
	
	if (buf->code	!= icd->current_fmt->code ||
	    vb->width	!= icd->user_width ||
	    vb->height	!= icd->user_height ||
	    vb->field	!= field) {
		buf->code	= icd->current_fmt->code;
		vb->width	= icd->user_width;
		vb->height	= icd->user_height;
		vb->field	= field;
		vb->state	= VIDEOBUF_NEEDS_INIT;
	}

	vb->size = bytes_per_line * vb->height;
	if (0 != vb->baddr && vb->bsize < vb->size) {
		ret = -EINVAL;
		goto out;
	}

	if (vb->state == VIDEOBUF_NEEDS_INIT) {
		ret = videobuf_iolock(vq, vb, NULL);
		if (ret)
			goto fail;

		vb->state = VIDEOBUF_PREPARED;
	}

	buf->inwork = 0;

	return 0;
	
fail:
	free_buffer(vq, buf);
out:
	buf->inwork = 0;
	return ret;
}

/**
 * @brief: Called when application apply buffers, camera start data collection
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *vq: V4L2  buffer queue information structure
 * @param [in] *vb: V4L2  buffer information structure
 */
static void ak_videobuf_queue(struct videobuf_queue *vq, 
								struct videobuf_buffer *vb)
{
	struct soc_camera_device *icd = vq->priv_data;
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	struct ak_buffer *buf = container_of(vb, struct ak_buffer, vb);
	u32 yaddr_chl1, yaddr_chl2, size;
	static int ch2_sync = 0;
	
	isp_dbg("%s (vb=0x%p) buf[%d] baddr = 0x%08lx, bsize = %d\n",
			__func__,  vb, vb->i, vb->baddr, vb->bsize);

	list_add_tail(&vb->queue, &pcdev->capture);

	vb->state = VIDEOBUF_ACTIVE;
	size = vb->width * vb->height;
	yaddr_chl1 = videobuf_to_dma_contig(vb); /* for mater channel */
	yaddr_chl2 = yaddr_chl1 + size * 3 / 2; /* for secondary channel */
	
	switch(pcdev->isp.cur_mode) {
	case ISP_YUV_OUT:
	case ISP_YUV_BYPASS:
	case ISP_RGB_OUT:	
		/* for single mode */
		if (!pcdev->active) {
			pcdev->active = buf;
			pcdev->dma_running = 1;	
			
			isp_set_even_frame(&pcdev->isp, yaddr_chl1, yaddr_chl2);
			isp_apply_mode(&pcdev->isp);
			isp_start_capturing(&pcdev->isp);
			
			isp_dbg("queue[single]: vbuf[%d] start run.\n", vb->i);
		}
		break;

	case ISP_YUV_VIDEO_OUT:
	case ISP_YUV_VIDEO_BYPASS:
	case ISP_RGB_VIDEO_OUT:
		/* for continous mode */
		if (!pcdev->active) {
			pcdev->active = buf;
			pcdev->dma_running = 0;
			ch2_sync = 1;
			
			isp_set_even_frame(&pcdev->isp, yaddr_chl1, yaddr_chl2);
			isp_dbg("queue[continue]: vbuf1[%d]\n", vb->i);
			return; 		
		}

		if (!pcdev->dma_running) {
			pcdev->dma_running = 1;

			if (ch2_sync) {
				ch2_sync = 0;
				irq_buf_empty_flag = 0;
				
				isp_set_odd_frame(&pcdev->isp, yaddr_chl1, yaddr_chl2);
				isp_apply_mode(&pcdev->isp);
				isp_start_capturing(&pcdev->isp);
				
				isp_dbg("queue[continue]: vbuf2[%d] start.\n", vb->i);
				return;
			}

			// ensure that can update yaddr immediately
			if (isp_is_capturing_odd(&pcdev->isp)) 
				isp_set_even_frame(&pcdev->isp, yaddr_chl1, yaddr_chl2);
			else 
				isp_set_odd_frame(&pcdev->isp, yaddr_chl1, yaddr_chl2);
			
		}
		break;
	default:
		printk("The working mode of ISP hasn't been initialized.\n");
	}
	if (pcdev->pdata->rf_led.pin > 0)
		{
	rfled_timer(pcdev);
		}
}

/**
 * @brief:  for ak_videobuf_release, free buffer if camera stopped.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *vq: V4L2 buffer queue information structure
 * @param [in] *vb: V4L2  buffer information structure
 */
static void ak_videobuf_release(struct videobuf_queue *vq, 
					struct videobuf_buffer *vb)
{
	struct ak_buffer *buf = container_of(vb, struct ak_buffer, vb);	
	struct soc_camera_device *icd = vq->priv_data;
//	struct device *dev = icd->parent;
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	unsigned long flags;
	
	isp_dbg("%s (vb=0x%p) buf[%d] 0x%08lx %d\n", 
			__func__, vb, vb->i, vb->baddr, vb->bsize);

	spin_lock_irqsave(&pcdev->lock, flags);
	isp_clear_irq(&pcdev->isp);
	spin_unlock_irqrestore(&pcdev->lock, flags);

	switch (vb->state) {
	case VIDEOBUF_ACTIVE:
		CAMDBG("vb status: ACTIVE\n");
		break;
	case VIDEOBUF_QUEUED:
		CAMDBG("vb status: QUEUED\n");
		break;
	case VIDEOBUF_PREPARED:
		CAMDBG("vb status: PREPARED\n");
		break;
	default:
		CAMDBG("vb status: unknown\n");
		break;
	}

	free_buffer(vq, buf);
}

static struct videobuf_queue_ops ak_videobuf_ops = {
	.buf_setup      = ak_videobuf_setup,
	.buf_prepare    = ak_videobuf_prepare,
	.buf_queue      = ak_videobuf_queue,
	.buf_release    = ak_videobuf_release,
};

/**
 * @brief: irq handler function, camera start data collection
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *pcdev:ak camera drivers structure, include soc camera structure
 */
static int ak_camera_setup_dma(struct ak_camera_dev *pcdev)
{
	struct videobuf_buffer *vb_active = &pcdev->active->vb;
	struct videobuf_buffer *vb;
	struct list_head *next;
	unsigned long yaddr_chl1_active, yaddr_chl2_active; 
	unsigned long yaddr_chl1_next, yaddr_chl2_next;
	int size;

	size = vb_active->width * vb_active->height;
	yaddr_chl1_active = videobuf_to_dma_contig(vb_active);
	yaddr_chl2_active = yaddr_chl1_active + size * 3 / 2;

	/* for single mode */
	if (!isp_is_continuous(&pcdev->isp)) {
		isp_set_even_frame(&pcdev->isp, yaddr_chl1_active, yaddr_chl2_active);
		isp_update_regtable(&pcdev->isp, 1);
		isp_start_capturing(&pcdev->isp);
		return 0;
	}

	/* ISP is in the continuous mode */
	next = pcdev->capture.next;
	next = next->next;
	if (next == &pcdev->capture) {
		isp_dbg("irq: the next vbuf is empty.\n");
		//isp_stop_capturing(&pcdev->isp);
		irq_buf_empty_flag = 1;
		irq_need_baffle = 1;
		pcdev->dma_running = 0;
		goto out;
	} else 
		irq_buf_empty_flag = 0;
	
	vb = list_entry(next, struct videobuf_buffer, queue);

	/* setup the DMA address for transferring */
	yaddr_chl1_next = videobuf_to_dma_contig(vb);
	yaddr_chl2_next = yaddr_chl1_next + size * 3 / 2;
	if (isp_is_capturing_odd(&pcdev->isp))
		isp_set_even_frame(&pcdev->isp, yaddr_chl1_next, yaddr_chl2_next);
	else 
		isp_set_odd_frame(&pcdev->isp, yaddr_chl1_next, yaddr_chl2_next);
out:	
	isp_update_regtable(&pcdev->isp, 0);
	return 0;
}

/**
 * @brief: irq handler function, camera start data collection.
 * wake up wait queue.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *pcdev:ak camera drivers structure, include soc camera structure
 * @param [in] *vb: V4L2  buffer information structure
 * @param [in] *buf: ak camera drivers structure, include struct videobuf_buffer 
 */
static void ak_camera_wakeup(struct ak_camera_dev *pcdev,
			      struct videobuf_buffer *vb,
			      struct ak_buffer *buf)
{
	struct captureSync *adctime;
	struct timeval		cam_tv;
	unsigned long		adc_stamp;
	unsigned long		useconds;
	unsigned long long 	actuallyBytes = 0;
	
	isp_dbg("%s (vb=0x%p) buf[%d], baddr = 0x%08lx, bsize = %d\n",
			__func__,  vb, vb->i, vb->baddr, vb->bsize);

	do_gettimeofday(&cam_tv);
	
	adctime = getRecordSyncSamples();

	/* figure out the timestamp of frame */
	//adc_stamp = (unsigned long)(( adctime->adcCapture_bytes * 1000) / ( adctime->rate * ( adctime->frame_bits / 8 ) ) );
	actuallyBytes = adctime->adcCapture_bytes * (unsigned long long)1000;
	if ( actuallyBytes != 0 ) {
		do_div( actuallyBytes, ( adctime->rate * ( adctime->frame_bits / 8 ) ) );
		adc_stamp = actuallyBytes;
	}else { //if current no audio
		adc_stamp = 1000; //any value.
	}

	if (cam_tv.tv_sec > adctime->tv.tv_sec) {
		useconds = cam_tv.tv_usec + 1000000 - adctime->tv.tv_usec;
	} else {
		useconds = cam_tv.tv_usec - adctime->tv.tv_usec;
	}	

	vb->ts.tv_usec = (adc_stamp % 1000) * 1000 + useconds;

	if(vb->ts.tv_usec >= 1000000) {
		vb->ts.tv_sec = adc_stamp / 1000 + 1;
		vb->ts.tv_usec = vb->ts.tv_usec % 1000000;
	} else {
		vb->ts.tv_sec = adc_stamp / 1000;
	}	

	/* We don't have much to do if the capturing list is empty */
	if (list_empty(&pcdev->capture)) {
		pcdev->active = NULL;
		pcdev->dma_running = 0;
		
		//REG32(&pcdev->isp.base + ISP_PERI_PARA) &= ~(1 << 29);
		//REG32(&pcdev->isp.base + ISP_PERI_PARA) |= (1 << 28);
		
		isp_stop_capturing(&pcdev->isp);
		printk("isp-irq: vbuf queue is empty.\n");
		return;
	}

	if (!irq_buf_empty_flag) {
		list_del_init(&vb->queue);
		vb->state = VIDEOBUF_DONE;
		vb->field_count++;
		// here,  current frame commit to video_buffer layer
	 	wake_up(&vb->done);
		
		isp_dbg("wakeup (vb=0x%p) buf[%d], baddr = 0x%08lx, bsize = %d\n",
				vb, vb->i, vb->baddr, vb->bsize);
	}

   if(pcdev->isp.cur_mode_class == ISP_RGB_CLASS)
   	{
   	 
		/*33ms:read next frame params in 33ms later.*/
		schedule_delayed_work(&pcdev->isp.awb_work, msecs_to_jiffies(33));
		schedule_delayed_work(&pcdev->isp.ae_work, 0);
   	}

	pcdev->active = list_entry(pcdev->capture.next,
					   struct ak_buffer, vb.queue);

	ak_camera_setup_dma(pcdev);
}

/**
 * @brief: camera irq handler function, camera start data collection.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *data: struct ak_camera_dev 
 */
static irqreturn_t ak_camera_dma_irq(int channel, void *data)
{
	struct ak_camera_dev *pcdev = data;
	struct ak_buffer *buf;
	struct videobuf_buffer *vb;
	unsigned long flags;
	
	spin_lock_irqsave(&pcdev->lock, flags);

	/*Fixme: isp_check_irq must be improved */
	if (isp_check_irq(&pcdev->isp) < 0) 
		goto out;
	
    // throw the last irq when video mode working stopped.
	if (irq_need_baffle) {
		irq_need_baffle = 0;
		goto out;
	}

	vb = &pcdev->active->vb;
	buf = container_of(vb, struct ak_buffer, vb);
	//buf = pcdev->active;
	WARN_ON(buf->inwork || list_empty(&vb->queue));

	ak_camera_wakeup(pcdev, vb, buf);
	
out:
	isp_clear_irq_status(&pcdev->isp);
	spin_unlock_irqrestore(&pcdev->lock, flags);
	return IRQ_HANDLED;
}	

/**
 * @brief: infrared light timer handler function.
 * change mode between daylight and nightlight
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] _data: ak camera drivers structure, include soc camera structure
 */
static void rfled_timer(unsigned long _data)
{
	struct ak_camera_dev *pcdev = (struct ak_camera_dev *)_data;
	struct v4l2_ctrl wb_ctrl, colorfx_ctrl;
	
    if (pcdev->pdata->gpio_get(pcdev->pdata->rf_led.pin)) {
		wb_ctrl.val = ISP_MANU_WB_0;
		colorfx_ctrl.val = V4L2_COLORFX_NONE;
		pcdev->isp.rfled_ison = 1;
	} else {
		wb_ctrl.val = ISP_MANU_WB_3;
		colorfx_ctrl.val = V4L2_COLORFX_BW;
		pcdev->isp.rfled_ison = 0;
	}

	isp_manu_set_wb_param(&pcdev->isp, &wb_ctrl, 1);
	isp_set_uspecial_effect(&pcdev->isp, &colorfx_ctrl, 1);
}

/**
 * @brief: infrared light irq handler function. change irq polarity is here.
 * change mode between daylight and nightlight
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] channel: irq number
 * @param [in] *data: ak camera drivers structure, include soc camera structure
 */
static irqreturn_t ak_rfled_isr(int channel, void *data)
{
	struct ak_camera_dev *pcdev = data;
	
	spin_lock(&pcdev->rfled_lock);

	if (pcdev->pdata->gpio_get(pcdev->pdata->rf_led.pin)) 
		ak_gpio_set_intpol(pcdev->pdata->rf_led.pin, AK_GPIO_INT_LOWLEVEL);
	else
		ak_gpio_set_intpol(pcdev->pdata->rf_led.pin, AK_GPIO_INT_HIGHLEVEL);
	
	mod_timer(&pcdev->timer,
			jiffies + msecs_to_jiffies(100));

	spin_unlock(&pcdev->rfled_lock);
	
    return IRQ_HANDLED;
}

/**
 * @brief: delay work queue, update image effects entry.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *work: struct isp_struct
 */
static void isp_awb_work(struct work_struct *work) 
{
	struct isp_struct *isp = container_of(work, struct isp_struct, awb_work.work);

	if (isp->auto_wb_param_en) //by xc
		isp_update_auto_wb_param(isp);
}


/**
 * @brief: delay work queue, update image effects entry.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *work: struct isp_struct
 */
static void isp_ae_work(struct work_struct *work) 
{
struct isp_struct *isp = container_of(work, struct isp_struct, ae_work.work);
//	struct isp_struct *isp = container_of(work, struct isp_struct, ae_work.work);

    isp_update_auto_exposure_param(isp);
	/*auto exposure.*/
}

static struct soc_camera_device *ctrl_to_icd(struct v4l2_ctrl *ctrl)
{
	return container_of(ctrl->handler, struct soc_camera_device,
							ctrl_handler);
}

/**
 * @brief: get function supported of camera, the function is image adjust, color effect...
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *ctrl: V4L2 image effect control information structure
 */
static int ak_camera_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
//	struct soc_camera_device *icd = ctrl_to_icd(ctrl);
//	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
//	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
//	struct ak_camera_dev *pcdev = ici->priv;
	
	isp_dbg("entry %s, ctrl->id=%x\n", __func__, ctrl->id);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		isp_dbg("%s(): V4L2_CID_BRIGHTNESS\n", __func__);
		break;
	case V4L2_CID_CONTRAST:
		isp_dbg("%s(): V4L2_CID_CONTRAST\n", __func__);
		break;
	case V4L2_CID_SATURATION:
		isp_dbg("%s(): V4L2_CID_SATURATION\n", __func__);
		break;
	case V4L2_CID_SHARPNESS:
		isp_dbg("%s(): V4L2_CID_SHARPNESS\n", __func__);
		break;
	case V4L2_CID_HUE:
		break;
	case V4L2_CID_HUE_AUTO:
		break;
	case V4L2_CID_COLORFX:
		isp_dbg("%s(): V4L2_CID_COLORFX\n", __func__);
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		isp_dbg("%s(): V4L2_CID_DO_WHITE_BALANCE\n", __func__);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		isp_dbg("%s(): V4L2_CID_AUTO_WHITE_BALANCE\n", __func__);
		break;
	}
	
	return 0;
}

/**
 * @brief: the isp standard control should be implemented here.
 * the function is image adjust, color effect...
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *ctrl: V4L2 image effect control information structure
 */
static int ak_camera_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_control control;
	struct soc_camera_device *icd = ctrl_to_icd(ctrl);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	
	isp_dbg("entry %s\n", __func__);

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		if (isp_set_brightness(&pcdev->isp, ctrl) == 0)
			return 0;
		break;
	case V4L2_CID_CONTRAST:
		if (isp_set_gamma(&pcdev->isp, ctrl) == 0)
			return 0;
		break;
	case V4L2_CID_SATURATION:
		if (isp_set_saturation(&pcdev->isp, ctrl) == 0)
			return 0;
		break;
	case V4L2_CID_SHARPNESS:
		if (isp_set_sharpness(&pcdev->isp, ctrl) == 0)
			return 0;
		break;
	case V4L2_CID_HUE:
		break;
	case V4L2_CID_HUE_AUTO:
		break;
	case V4L2_CID_COLORFX:
		if (isp_set_uspecial_effect(&pcdev->isp, ctrl, 0) == 0)
			return 0;
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		if (isp_manu_set_wb_param(&pcdev->isp, ctrl, 0) == 0)
			return 0;
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		if (isp_auto_set_wb_param(&pcdev->isp, ctrl) == 0)
			return 0;
		break;
	}
	
	control.id = ctrl->id;
	control.value = ctrl->val;
	v4l2_subdev_call(sd, core, s_ctrl, &control);
	
	return 0;
}


static const struct v4l2_ctrl_ops ak_camera_ctrl_ops = {
	.g_volatile_ctrl	= ak_camera_g_volatile_ctrl,
	.s_ctrl				= ak_camera_s_ctrl,
};

/**
 * @brief: set sensor clock
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] cis_sclk: sensor work clock
 */
static void set_sensor_cis_sclk(unsigned int cis_sclk)
{
	unsigned long regval;
	unsigned int cis_sclk_div;
	
	unsigned int peri_pll = ak_get_peri_pll_clk()/1000000;
	
	cis_sclk_div = peri_pll/cis_sclk - 1;

	regval = REG32(CLOCK_PERI_PLL_CTRL2);
	regval &= ~(0x3f << 10);
	regval |= (cis_sclk_div << 10);
	REG32(CLOCK_PERI_PLL_CTRL2) = (1 << 19)|regval;

	isp_dbg("%s() cis_sclk=%dMHz peri_pll=%dMHz cis_sclk_div=%d\n", 
			__func__, cis_sclk, peri_pll, cis_sclk_div);
}

/**
 * @brief: Called when the /dev/videox is opened. initial ISP and sensor device.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 */
static int ak_camera_add_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd); 
	struct ak_camera_cam *cam;
	int cis_sclk;
	
	CAMDBG("entry %s\n", __func__);

	/* The ak camera host driver only support one image sensor */
	if (pcdev->icd)
		return -EBUSY;

	dev_info(icd->parent, "AK Camera driver attached to camera %d\n",
		 icd->devnum);

	/* for debugging. Capture list should be empty when the video opened. */
	if (!list_empty(&pcdev->capture)) {
		printk("Bug: pcdev->capture is not empty\n");
		list_del_init(&pcdev->capture);
	}

	/********** config sensor module **********/
	//get sensor clk and  power up the sensor
	cis_sclk = v4l2_subdev_call(sd, core, init, 0);
	if (cis_sclk <= 0) {
		cis_sclk = 24;
	}
	
	//set cis_sclk, the sensor present working 24MHz
	clk_enable(pcdev->cis_sclk);
	set_sensor_cis_sclk(cis_sclk);
	
	// load the default setting for sensor
	v4l2_subdev_call(sd, core, load_fw);

	/********** config isp module **********/
	ak_soft_reset(AK_SRESET_CAMERA);
	//set shared GPIO to CAMERA function
	ak_group_config(ePIN_AS_CAMERA);
	// enable isp clock
	clk_enable(pcdev->clk);

	pcdev->icd = icd;
	
	/* FIXME Here, add out control */
	if (!icd->host_priv) {
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_BRIGHTNESS, ISP_BRIGHTNESS_0, 
			ISP_BRIGHTNESS_6, 1, ISP_BRIGHTNESS_3);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_CONTRAST, ISP_GAMMA_0, 
			ISP_GAMMA_6, 1, ISP_GAMMA_3);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_SATURATION, ISP_SATURATION_0, 
			ISP_SATURATION_6, 1, ISP_SATURATION_3);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_SHARPNESS, ISP_SHARPNESS_0, 
			ISP_SHARPNESS_6, 1, ISP_SHARPNESS_3);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_HUE, 0, 256, 1, 128);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_HUE_AUTO, 0, 1, 1, 0);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_DO_WHITE_BALANCE, ISP_MANU_WB_0, 
			ISP_MANU_WB_6, 1, ISP_MANU_WB_0);
		v4l2_ctrl_new_std(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_AUTO_WHITE_BALANCE, 0, 1, 1, 1);
		v4l2_ctrl_new_std_menu(&icd->ctrl_handler, &ak_camera_ctrl_ops,
			V4L2_CID_COLORFX, V4L2_COLORFX_VIVID, 0, V4L2_COLORFX_NONE);
		
		/* FIXME: subwindow is lost between close / open */
		cam = kzalloc(sizeof(*cam), GFP_KERNEL);
		if (!cam)
		return -ENOMEM;

		/* We are called with current camera crop, initialise subrect with it */
		icd->host_priv = cam;
	} else {
		cam = icd->host_priv;
	}
	
	return 0;
}

/**
 * @brief: Called when the /dev/videox is close. close ISP and sensor device.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 */
static void ak_camera_remove_device(struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd); 

	CAMDBG("entry %s\n", __func__);

	BUG_ON(icd != pcdev->icd);

	v4l2_subdev_call(sd, core, reset, 0);

	isp_clear_irq(&pcdev->isp);
	isp_stop_capturing(&pcdev->isp);

	/* disable sensor clk */
	clk_disable(pcdev->cis_sclk);
	
	/* disable the clock of isp module */
	clk_disable(pcdev->clk);

	//ak_soft_reset(AK_SRESET_CAMERA);

	dev_info(icd->parent, "AK Camera driver detached from camera %d\n",
		 icd->devnum);	

	pcdev->active = NULL;   
	pcdev->icd = NULL;

	CAMDBG("Leave %s\n", __func__);	
}

/**
 * @brief: set camera capability, used for query by user.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *ici: soc_camera_host information structure. 
 * @param [in] *cap: v4l2_capability information structure.
 */
static int ak_camera_querycap(struct soc_camera_host *ici,
			       struct v4l2_capability *cap)
{
	isp_dbg("entry %s\n", __func__);
	
	/* cap->name is set by the friendly caller:-> */
	strlcpy(cap->card, ak_cam_driver_description, sizeof(cap->card));
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	
	return 0;	
}

static int ak_camera_cropcap(struct soc_camera_device *icd, 
					struct v4l2_cropcap *crop)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	
	isp_dbg("enter %s\n", __func__);

	if (crop->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	// isp support crop, need complete. 
	return v4l2_subdev_call(sd, video, cropcap, crop);
}

static int ak_camera_get_crop(struct soc_camera_device *icd,
			       struct v4l2_crop *crop)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	//struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	//struct ak_camera_dev *pcdev = ici->priv;

	isp_dbg("entry %s\n", __func__);
	
	return v4l2_subdev_call(sd, video, g_crop, crop);
}

static int ak_camera_set_crop(struct soc_camera_device *icd,
			       struct v4l2_crop *crop)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	int ret, width, height;

	isp_dbg("entry %s\n", __func__);
	
	if (pcdev->dma_running) {
		/* make sure streaming is not started */
		v4l2_err(&ici->v4l2_dev,
			"Cannot change crop when streaming is ON\n");
		return -EBUSY;
	}

	width = crop->c.width - crop->c.left;
	height = crop->c.height - crop->c.top;
	if ((crop->c.top < 0 || crop->c.left < 0)
		||(((width * 3) < 18) || (height * 3) < 18)
		||((width > 1280) || (height > 720))) {
		v4l2_err(&ici->v4l2_dev,
			"doesn't support negative values for top & left\n");
		return -EINVAL;
	}

	if ((ret = isp_set_crop(&pcdev->isp, crop->c)) < 0)
		ret = v4l2_subdev_call(sd, video, s_crop, crop);

	return ret;
}

/**
 * @brief: setting image format information, Called before ak_camera_set_fmt.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *f: image format
 */
static int ak_camera_try_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	int ret;
	/* TODO: limit to ak hardware capabilities */
	CAMDBG("entry %s\n", __func__);

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_warn(icd->parent, "Format %x not found\n",
			 pix->pixelformat);
		return -EINVAL;
	}

	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;

	/* limit to sensor capabilities */
	ret = v4l2_subdev_call(sd, video, try_mbus_fmt, &mf);
	if (ret < 0) {
		return ret;
	}	

	pix->width	= mf.width;
	pix->height = mf.height;
	pix->field	= mf.field;
	pix->colorspace = mf.colorspace;

	return 0;
}

/**
 * @brief: setting image format information
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *f: image format
 */
static int ak_camera_set_fmt(struct soc_camera_device *icd,
			      struct v4l2_format *f)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;		
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_mbus_framefmt mf;
	struct v4l2_cropcap cropcap;
	int ret, buswidth;

	isp_dbg("entry %s\n", __func__);

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_warn(icd->parent, "Format %x not found\n",
			 pix->pixelformat);
		return -EINVAL;
	}

	//if (YUV_OUT)
		
	buswidth = xlate->host_fmt->bits_per_sample;
	if (buswidth > 10) {
		dev_warn(icd->parent,
			 "bits-per-sample %d for format %x unsupported\n",
			 buswidth, pix->pixelformat);
		return -EINVAL;
	}

	mf.width	= pix->width;
	mf.height	= pix->height;
	mf.field	= pix->field;
	mf.colorspace	= pix->colorspace;
	mf.code		= xlate->code;
	icd->current_fmt = xlate;

	v4l2_subdev_call(sd, video, cropcap, &cropcap);
	if ((mf.width > cropcap.bounds.width) 
		|| (mf.height > cropcap.bounds.height)) {
		/* D1 get from:
		  if cropcap.bounds = 720P, output by scale down
		  if cropcap.bounds = VGA, output by scale up 
		*/
		mf.width = cropcap.defrect.width;
		mf.height = cropcap.defrect.height;
	} else if ((mf.width < cropcap.defrect.width)
		|| (mf.height < cropcap.defrect.height)) {
		/*
		  hypothesis, sensor output least size is VGA. the defrect size is VGA
		  D2 and QVGA get from VGA scale down
		*/
		mf.width = cropcap.defrect.width;
		mf.height = cropcap.defrect.height;
	} 

	isp_dbg("%s. mf.width = %d, mf.height = %d\n", 
			__func__, mf.width, mf.height);

	ret = v4l2_subdev_call(sd, video, s_mbus_fmt, &mf);
	if (ret < 0) 
		return ret;
	
	if (mf.code != xlate->code) 
		return -EINVAL;

	/* recored the VGA size used for check to enable isp ch2 when RGB input*/
	pcdev->isp.fmt_def_width = mf.width;
	pcdev->isp.fmt_def_height = mf.height;
	
	/*
	  * @fmt_width and fmt_height is the input image size of sensor.
	  * @chl1_width and chl1_height is the output image size for user.
	  */
	pcdev->isp.fmt_width = pix->width;
	pcdev->isp.fmt_height = pix->height;
	pcdev->isp.chl1_width = pix->width;
	pcdev->isp.chl1_height= pix->height;
	
	isp_set_cutter_window(&pcdev->isp, 0, 0, mf.width, mf.height);
	isp_set_channel1_scale(&pcdev->isp, pix->width, pix->height);
	
	isp_dbg("%s: chl1_width=%d, chl1_height=%d\n", __func__, 
					pcdev->isp.chl1_width, pcdev->isp.chl1_height);
	
	return ret;
}

/**
 * @brief: getting image format information
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *f: image format
 */
static int ak_camera_get_formats(struct soc_camera_device *icd, unsigned int idx,
				     struct soc_camera_format_xlate *xlate)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct device *dev = icd->parent;
	struct soc_camera_host *ici = to_soc_camera_host(dev);
	struct ak_camera_dev *pcdev = ici->priv;
	int ret, formats = 0;
	enum v4l2_mbus_pixelcode code;
	const struct soc_mbus_pixelfmt *fmt;

	CAMDBG("entry %s\n", __func__);
	ret = v4l2_subdev_call(sd, video, enum_mbus_fmt, idx, &code);
	if (ret < 0)
		/* No more formats */
		return 0;

	/*
	  * @Note: ISP only support yuv420 output and jpeg out.
	  *	FIXME1: We miss jpeg here.
	  *  FIXME2: the output squence of YUV is actually UYVY.
	  */
	fmt = soc_mbus_get_fmtdesc(V4L2_MBUS_FMT_YUYV8_2X8);
	if (!fmt) {
		dev_warn(dev, "unsupported format code #%u: %d\n", idx, code);
		return 0;
	}
	CAMDBG("get format %s code=%d from sensor\n", fmt->name, code);
	
	/* Generic pass-through */
	formats++;
	if (xlate) {
		xlate->host_fmt	= fmt;
		xlate->code	= code;
		xlate++;

		/*
		  * @decide the default working mode of isp
		  * @prefer RGB mode
		  */
		if (code < V4L2_MBUS_FMT_Y8_1X8) {
			pcdev->def_mode = ISP_RGB_VIDEO_OUT;
			//pcdev->def_mode = ISP_RGB_OUT;
		} 
		
		if ((pcdev->def_mode != ISP_RGB_VIDEO_OUT)
			&& (pcdev->def_mode != ISP_RGB_OUT)) {
			pcdev->def_mode = ISP_YUV_VIDEO_BYPASS;
			//pcdev->def_mode = ISP_YUV_BYPASS;
		}
		pcdev->isp.cur_mode = pcdev->def_mode;
		update_cur_mode_class(&pcdev->isp);
		
		dev_dbg(dev, "Providing format %s in pass-through mode\n",
			fmt->name);
	}

	return formats;
}

static void ak_camera_put_formats(struct soc_camera_device *icd)
{
	CAMDBG("entry %s\n", __func__);
	kfree(icd->host_priv);
	icd->host_priv = NULL;
	CAMDBG("leave %s\n", __func__);
}

/**
 * @brief: setting sensor register.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: isp_config_sensor_reg structure
 */
static int isp_set_sensor_param(struct isp_struct *isp, struct isp_config_sensor_reg *ctrl)
{
	aksensor_set_param(ctrl->cmd, ctrl->data);
	return 0;
}

/**
 * @brief: getting sensor register.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: isp_config_sensor_reg structure
 */
static unsigned int isp_get_sensor_param(struct isp_struct *isp, struct isp_config_sensor_reg *ctrl)
{
	return aksensor_get_param(ctrl->cmd);
}

/**
 * @brief: The private interface of ISP for application user.
 * setting ISP controller for image information
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *a: v4l2_streamparm structure, also used for user space.
 */
 static int ak_camera_set_parm(struct soc_camera_device *icd,
			      struct v4l2_streamparm *a)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;		
	struct isp_mode_info *mode_info;
	int retval = 0, *parm_type;
	
	CAMDBG("entry %s\n", __func__);

	parm_type = (int *)a->parm.raw_data;
	switch(*parm_type) {
	case ISP_PARM_MODE:
		mode_info = (struct isp_mode_info *)parm_type;
		if (!pcdev->dma_running) {
			pcdev->isp.cur_mode = mode_info->mode;

			update_cur_mode_class(&pcdev->isp);
			isp_apply_mode(&pcdev->isp);
		
			printk("%s: change working mode to %d\n", __func__, mode_info->mode);
		} else {
			printk("%s: working mode can not be changed when dma is running\n", __func__);
			retval = -EBUSY;
		}
		break;
	case ISP_PARM_CHANNEL2:
		retval = isp_set_channel2(&pcdev->isp, (struct isp_channel2_info *)parm_type);
		break;
	case ISP_PARM_OSD:
		retval = isp_set_osd(&pcdev->isp, (struct isp_osd_info *)parm_type);
		break;
	case ISP_PARM_OCCLUSION:
		retval = isp_set_occlusion_area(&pcdev->isp, (struct isp_occlusion_info *)parm_type);
		break;
	case ISP_PARM_OCCLUSION_COLOR:
		retval = isp_set_occlusion_color(&pcdev->isp, (struct isp_occlusion_color *)parm_type);
		break;
	case ISP_PARM_ZOOM:
		retval = isp_set_zoom(&pcdev->isp, (struct isp_zoom_info *)parm_type);
		break;
	case ISP_CID_BLACK_BALANCE:
		retval = isp_set_black_balance(&pcdev->isp, (struct isp_black_balance *)parm_type);
		break;
	case ISP_CID_LENS:
		retval = isp_set_lens_correct(&pcdev->isp, (struct isp_lens_correct *)parm_type);
		break;
	case ISP_CID_DEMOSAIC:
		retval = isp_set_demosaic(&pcdev->isp, (struct isp_demosaic *)parm_type);
		break;
	case ISP_CID_RGB_FILTER:
		retval = isp_set_rgb_filter(&pcdev->isp, (struct isp_rgb_filter *)parm_type);
		break;
	case ISP_CID_UV_FILTER:
		retval = isp_set_uv_iso_filter(&pcdev->isp, (struct isp_uv_filter *)parm_type);
		break;
	case ISP_CID_DEFECT_PIXEL:
		retval = isp_set_defect_pixel(&pcdev->isp, (struct isp_defect_pixel *)parm_type);
		break;
	case ISP_CID_WHITE_BALANCE:
		retval = isp_set_manu_wb(&pcdev->isp, (struct isp_white_balance *)parm_type);
		break;
	case ISP_CID_AUTO_WHITE_BALANCE:
		retval = isp_set_auto_wb(&pcdev->isp, (struct isp_auto_white_balance *)parm_type);
		break;
	case ISP_CID_COLOR:
		retval = isp_set_color_correct(&pcdev->isp, (struct isp_color_correct *)parm_type);
		break;
	case ISP_CID_GAMMA:
		retval = isp_set_gamma_calc(&pcdev->isp, (struct isp_gamma_calculate *)parm_type);
		break;
	case ISP_CID_BRIGHTNESS_ENHANCE:
		retval = isp_set_brightness_enhance(&pcdev->isp, (struct isp_brightness_enhance *)parm_type);
		break;
	case ISP_CID_SATURATION:
		retval = isp_set_uv_saturation(&pcdev->isp, (struct isp_saturation *)parm_type);
		break;
	case ISP_CID_HISTOGRAM:
		retval = isp_set_histogram(&pcdev->isp, (struct isp_histogram *)parm_type);
		break;
	case ISP_CID_SPECIAL_EFFECT:
		retval = isp_set_special_effect(&pcdev->isp, (struct isp_special_effect *)parm_type);
		break;
	case ISP_CID_SET_SENSOR_PARAM:
		retval = isp_set_sensor_param(&pcdev->isp, (struct isp_config_sensor_reg *)parm_type);
		break;
	case ISP_CID_GET_SENSOR_PARAM:
		retval = isp_get_sensor_param(&pcdev->isp, (struct isp_config_sensor_reg *)parm_type);
		break;
// ycx
	case ISP_CID_AE_CTRL_PARAM:
		retval = isp_set_ae_attr(&pcdev->isp, (struct isp_ae_attr*)parm_type);
		break;
	case ISP_CID_CC_AWB_PARAM:
		retval = isp_set_cc_with_awb(&pcdev->isp, (struct isp_color_correct_awb*)parm_type);
		break;
	default:
		retval = -EINVAL;
		printk("%s: private control encounter unknown type\n", __func__);
	}
 
	CAMDBG("leave %s\n", __func__);

	return retval;
}

 /**
  * @brief: The private interface of ISP for application user.
  * getting ISP controller for image information
  * 
  * @author: caolianming
  * @date: 2014-01-06
  * @param [in] *icd: soc_camera_device information structure, 
  * akcamera depends on the soc driver.
  * @param [in] *a: v4l2_streamparm structure, also used for user space.
  */
static int ak_camera_get_parm(struct soc_camera_device *icd,
			      struct v4l2_streamparm *a)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;		
	struct isp_channel2_info *chl2_info;
	struct isp_white_balance *co_rgb_info;
	struct isp_mode_info *mode_info;
	int retval = 0, *parm_type;
	
	parm_type = (int *)a->parm.raw_data;
	
	switch(*parm_type) {
	case ISP_PARM_MODE:
		mode_info = (struct isp_mode_info *)parm_type;
		mode_info->mode = pcdev->isp.cur_mode;
		break;
	case ISP_PARM_CHANNEL2:
		chl2_info = (struct isp_channel2_info *)parm_type;
		chl2_info->width = pcdev->isp.chl2_width;
		chl2_info->height = pcdev->isp.chl2_height;
		chl2_info->enable = pcdev->isp.chl2_enable;
		break;
	case ISP_CID_WHITE_BALANCE:
	case ISP_CID_AUTO_WHITE_BALANCE:
		co_rgb_info = (struct isp_white_balance *)parm_type;
		co_rgb_info->co_r = pcdev->isp.wb_param.co_r;
		co_rgb_info->co_g = pcdev->isp.wb_param.co_g;
		co_rgb_info->co_b = pcdev->isp.wb_param.co_b;
		break;
	default:
		retval = -EINVAL;
		printk("%s: private control encounter unknown type\n", __func__);
		break;
	}
 
	CAMDBG("leave %s\n", __func__);

	return retval;
}


/* Maybe belong platform code fix me */
static int ak_camera_set_bus_param(struct soc_camera_device *icd)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};	
	unsigned long common_flags;
	int ret;

	CAMDBG("entry %s\n", __func__);

	/* AK39 supports 8bit and 10bit buswidth */
	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		common_flags = soc_mbus_config_compatible(&cfg, CSI_BUS_FLAGS);
		if (!common_flags) {
			dev_warn(icd->parent,
				 "Flags incompatible: camera 0x%x, host 0x%x\n",
				 cfg.flags, CSI_BUS_FLAGS);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	} else {
		common_flags = CSI_BUS_FLAGS;
	}

	/* Make choises, based on platform choice */
	if ((common_flags & V4L2_MBUS_VSYNC_ACTIVE_HIGH) &&
		(common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)) {
			if (!pcdev->pdata ||
			     pcdev->pdata->flags & AK_CAMERA_VSYNC_HIGH)
				common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_LOW;
			else
				common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_HIGH;
	}

	if ((common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING) &&
		(common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)) {
			if (!pcdev->pdata ||
			     pcdev->pdata->flags & AK_CAMERA_PCLK_RISING)
				common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_FALLING;
			else
				common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_RISING;
	}

	if ((common_flags & V4L2_MBUS_DATA_ACTIVE_HIGH) &&
		(common_flags & V4L2_MBUS_DATA_ACTIVE_LOW)) {
			if (!pcdev->pdata ||
			     pcdev->pdata->flags & AK_CAMERA_DATA_HIGH)
				common_flags &= ~V4L2_MBUS_DATA_ACTIVE_LOW;
			else
				common_flags &= ~V4L2_MBUS_DATA_ACTIVE_HIGH;
	}

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_dbg(icd->parent, "camera s_mbus_config(0x%lx) returned %d\n",
			common_flags, ret);
		return ret;
	}

	CAMDBG("leave %s\n", __func__);
	
	return 0;
}

/**
 * @brief: register video buffer by video sub-system
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *q: V4L2  buffer queue information structure
 */
static void ak_camera_init_videobuf(struct videobuf_queue *q,
			struct soc_camera_device *icd)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ak_camera_dev *pcdev = ici->priv;

	CAMDBG("entry %s\n", __func__);

	videobuf_queue_dma_contig_init(q, &ak_videobuf_ops, icd->parent,
				&pcdev->lock, V4L2_BUF_TYPE_VIDEO_CAPTURE,
				V4L2_FIELD_NONE,
				sizeof(struct ak_buffer), icd, &icd->video_lock);

	CAMDBG("leave %s\n", __func__);
}

/**
 * @brief: request video buffer.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *icd: soc_camera_device information structure, 
 * akcamera depends on the soc driver.
 * @param [in] *q: V4L2  buffer queue information structure
 */
static int ak_camera_reqbufs(struct soc_camera_device *icd, 
				struct v4l2_requestbuffers *p)
{
	int i;

	CAMDBG("entry %s\n", __func__);

	/* This is for locking debugging only. I removed spinlocks and now I
	 * check whether .prepare is ever called on a linked buffer, or whether
	 * a dma IRQ can occur for an in-work or unlinked buffer. Until now
	 * it hadn't triggered */
	for (i = 0; i < p->count; i++) {
		struct ak_buffer *buf = container_of(icd->vb_vidq.bufs[i],
						      struct ak_buffer, vb);
		buf->inwork = 0;
		INIT_LIST_HEAD(&buf->vb.queue);
	}
	
	CAMDBG("leave %s\n", __func__);
	
	return 0;
}

/* platform independent */
static unsigned int ak_camera_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;
	struct ak_buffer *buf;

	buf = list_entry(icd->vb_vidq.stream.next, struct ak_buffer,
			 vb.stream);

	poll_wait(file, &buf->vb.done, pt);

	if (buf->vb.state == VIDEOBUF_DONE ||
	    buf->vb.state == VIDEOBUF_ERROR) {
		return POLLIN | POLLRDNORM;
	}	
	
	return 0;
}


static struct soc_camera_host_ops ak_soc_camera_host_ops = {
	.owner		= THIS_MODULE,
	.add			= ak_camera_add_device,
	.remove			= ak_camera_remove_device,
	.get_formats	= ak_camera_get_formats,
	.put_formats	= ak_camera_put_formats,
	.set_bus_param	= ak_camera_set_bus_param,
	.cropcap		= ak_camera_cropcap,
	.get_crop		= ak_camera_get_crop,
	.set_crop		= ak_camera_set_crop,
	.set_fmt		= ak_camera_set_fmt,
	.try_fmt		= ak_camera_try_fmt,
	.init_videobuf	= ak_camera_init_videobuf,
	.reqbufs		= ak_camera_reqbufs,
	.poll			= ak_camera_poll,
	.querycap	= ak_camera_querycap,
	.set_parm	= ak_camera_set_parm,
	.get_parm	= ak_camera_get_parm,
};

static int ak_camera_probe(struct platform_device *pdev)
{
	struct ak_camera_dev *pcdev;
	struct resource *res;
	struct clk *clk, *cis_sclk;
	void __iomem *base;
	unsigned int irq, rfled_irq;
	int err = 0;

	CAMDBG("entry %s\n", __func__);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!res || irq < 0) {
		printk("platform_get_irq | platform_get_resource\n");
		err = -ENODEV;
		goto exit;
	}

	/*
	  * @get isp working clock 
	  */
	clk = clk_get(&pdev->dev, "camera");
	if (IS_ERR(clk)) {
		err = PTR_ERR(clk);
		goto exit;
	}

	/*
	  * @get cis_sclk for sensor
	  */
	cis_sclk = clk_get(&pdev->dev, "sensor");
	if (IS_ERR(cis_sclk)) {
		err = PTR_ERR(cis_sclk);
		goto exit_put_clk;
	}

	/* 
	** @allocate memory to struct ak_camera, including struct soc_camera_host
	** @and struct v4l2_device 
	*/
	pcdev = kzalloc(sizeof(*pcdev), GFP_KERNEL);
	if (!pcdev) {
		err = -ENOMEM;
		goto exit_put_cisclk;
	}

	/* @initailization for struct pcdev */
	pcdev->res = res;
	pcdev->clk = clk;
	pcdev->cis_sclk = cis_sclk;
	pcdev->dma_running = 0;

	pcdev->pdata = pdev->dev.platform_data;
	if (!pcdev->pdata) {
		err = -ENODEV;
		goto exit_put_cisclk;
	}
	
	if (pcdev->mclk > 0)
		pcdev->mclk = pcdev->pdata->mclk;
	else {
		dev_warn(&pdev->dev,
			 "Platform mclk == 0! Please, fix your platform data. "
			 "Using default 24MHz\n");
		pcdev->mclk = 24;
	}

	if (pcdev->pdata->rf_led.pin > 0) 
		pcdev->pdata->gpio_set(&pcdev->pdata->rf_led);
		
	INIT_LIST_HEAD(&pcdev->capture);
	spin_lock_init(&pcdev->lock);

	/*
	 * Request the regions.
	 */
	if (!request_mem_region(res->start, resource_size(res), AK_CAM_DRV_NAME)) {
		err = -EBUSY;
		goto exit_kfree;
	}

	base = ioremap_nocache(res->start, resource_size(res));
	if (!base) {
		err = -ENOMEM;
		goto exit_release;
	}
	pcdev->irq = irq;	
	pcdev->base = base;

	/*
	  * @initialize pcdev->isp_struct
	  */
	pcdev->isp.base = base;
	if (isp_module_init(&pcdev->isp) < 0) {
		err = -ENOMEM;
		goto exit_iounmap;
	}

	if (pcdev->pdata->rf_led.pin > 0) {
		setup_timer(&pcdev->timer, rfled_timer, (unsigned long)pcdev);
		rfled_irq = ak_gpio_to_irq(pcdev->pdata->rf_led.pin);
		err = request_irq(rfled_irq, ak_rfled_isr, 
				IRQF_DISABLED, "rfled", pcdev);
		if (err) {
			err = -ENODEV;
			goto exit_iounmap;
		}
	}
	
	/*
	  * request irq 
	  */	
	err = request_irq(irq, ak_camera_dma_irq, IRQF_DISABLED, "ak_camera", pcdev);
	if (err) {
		err = -EBUSY;
		goto exit_freeisp;
	}
	/* init auto white balance*/
	INIT_DELAYED_WORK(&pcdev->isp.awb_work, isp_awb_work);
	/* init auto exposure*/
	INIT_DELAYED_WORK(&pcdev->isp.ae_work, isp_ae_work);
	
	/*
	** @register soc_camera_host
	*/
	pcdev->soc_host.drv_name	= AK_CAM_DRV_NAME;
	pcdev->soc_host.ops		= &ak_soc_camera_host_ops;
	pcdev->soc_host.priv		= pcdev;
	pcdev->soc_host.v4l2_dev.dev	= &pdev->dev;
	pcdev->soc_host.nr		= pdev->id;

	err = soc_camera_host_register(&pcdev->soc_host);
	if (err) {
		goto exit_freeirq;
	}

	dev_info(&pdev->dev, "AK Camera driver loaded\n");

	return 0;
	
exit_freeirq:
	free_irq(irq, pcdev);
exit_freeisp:
	isp_module_fini(&pcdev->isp);
exit_iounmap:
	iounmap(base);
exit_release:
	release_mem_region(res->start, resource_size(res));
exit_kfree:
	kfree(pcdev);
exit_put_cisclk:
	clk_put(cis_sclk);
exit_put_clk:
	clk_put(clk);
exit:
	return err;
}

static int ak_camera_remove(struct platform_device *pdev)
{

	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct ak_camera_dev *pcdev = container_of(soc_host,
					struct ak_camera_dev, soc_host);
	struct resource *res;

	CAMDBG("entry %s\n", __func__);

	/* free irq */
	free_irq(pcdev->irq, pcdev);

	/* free clk */
	clk_put(pcdev->clk);
	clk_put(pcdev->cis_sclk);

	soc_camera_host_unregister(soc_host);

	iounmap(pcdev->base);

	res = pcdev->res;
	release_mem_region(res->start, resource_size(res));

	/*
	  * @deconstruct the isp object.
	  */
	isp_module_fini(&pcdev->isp);

	kfree(pcdev);

	dev_info(&pdev->dev, "AK Camera driver unloaded\n");
	
	return 0;
}

static struct platform_driver ak_camera_driver = {
	.probe		= ak_camera_probe,
	.remove		= ak_camera_remove, 	
	.driver		= {
		.name = AK_CAM_DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ak_camera_init(void)
{
	CAMDBG("entry %s\n", __func__);

	return platform_driver_register(&ak_camera_driver);
}

static void __exit ak_camera_exit(void)
{
	CAMDBG("entry %s\n", __func__);

	platform_driver_unregister(&ak_camera_driver);
}

module_init(ak_camera_init);
module_exit(ak_camera_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wu_daochao <wu_daochao@anyka.oa>");
MODULE_DESCRIPTION("Driver for ak Camera Interface");

