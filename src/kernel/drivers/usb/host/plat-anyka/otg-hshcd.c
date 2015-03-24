/*
 * Anyka OTG HS HCD (Full-Speed Host Controller Driver) for USB.
 *
 * Derived from the SL811 HCD, rewritten for AKOTG HS HCD.
 * Copyright (C) 2010 ANYKA LTD.
 *
 * Periodic scheduling is based on Roman's OHCI code
 * 	Copyright (C) 1999 Roman Weissgaerber
 *
 * The AK OTG HS Host controller handles host side USB. For Documentation,
 * refer to chapter 22 USB Controllers of Anyka chip Mobile Multimedia Application
 * Processor Programmer's Guide.
 *
 */

/*
 * Status:  Enumeration of USB Optical Mouse, USB Keyboard, USB Flash Disk, Ralink 2070/3070 USB WiFi OK.
 *          Pass basic test with USB Optical Mouse/USB Keyboard/USB Flash Disk.
 *          Ralink 2070/3070 USB WiFI Scanning/WEP basic test OK. Full Functions TBD.
 *
 * TODO:
 * - Use up to 6 active queues of FS HC(for now only 2 queues: EP0 & EPX(1-6))
 * - USB Suspend/Resume support
 * - Use urb->iso_frame_desc[] with ISO transfers
 * - Optimize USB FIFO data transfer/receive(4B->2B->1B)
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <plat-anyka/otg-hshcd.h>
#include <plat-anyka/usb-hc.h>

#include <mach/clock.h>

static char *host_sw = "onboard";
module_param(host_sw, charp, S_IRUGO);

static const char hcd_name[] = "usb-host";
extern struct akotghc_epfifo_mapping akotg_epfifo_mapping;;
extern struct workqueue_struct *g_otghc_wq;;


static struct hc_driver akhs_otg_driver = {
	.description		= hcd_name,
	.product_desc 		= "Anyka usb host controller",
	.hcd_priv_size 		= sizeof(struct akotg_usbhc),

	/*
	 * generic hardware linkage
	 */
	.irq			= akotg_usbhc_irq,
	.flags			= HCD_USB2 | HCD_MEMORY,

	/* Basic lifecycle operations */
	.start			= akotg_usbhc_start,
	.stop			= akotg_usbhc_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	
	.urb_dequeue		= akotg_usbhc_urb_dequeue,
	.urb_enqueue		= akotg_usbhc_urb_enqueue,
	.endpoint_reset		= akotg_usbhc_endpoint_reset,
	.endpoint_disable	= akotg_usbhc_endpoint_disable,

	/*
	 * periodic schedule support
	 */
	.get_frame_number	= akotg_usbhc_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= akotg_usbhc_hub_status_data,
	.hub_control		= akotg_usbhc_hub_control,
	#ifdef	CONFIG_PM
	.bus_suspend		= akotg_usbhc_bus_suspend,
	.bus_resume		= akotg_usbhc_bus_resume,
	#else
	.bus_suspend		= NULL,
	.bus_resume		= NULL,
	#endif
};

/*-------------------------------------------------------------------------*/

static void usb_poweron_for_device(struct akotghc_usb_platform_data *pdata)
{
	/* power on for usb device */
	if(pdata->gpio_pwr_on.pin >= 0) 
		pdata->gpio_init(&pdata->gpio_pwr_on);

	/* ust otg host port switch  */
	if((pdata->switch_onboard.pin >= 0)&&(pdata->switch_extport.pin >= 0)
		&& (pdata->switch_onboard.pin == pdata->switch_extport.pin)) {
		if (!strcmp(host_sw, "extport")) 
			pdata->gpio_init(&pdata->switch_extport);
		else 
			pdata->gpio_init(&pdata->switch_onboard);
		
	}
}

static void usb_poweroff_for_device(struct akotghc_usb_platform_data *pdata)
{
	/* ust otg host port switch  */
	if((pdata->switch_onboard.pin >= 0)&&(pdata->switch_extport.pin >= 0)
		&& (pdata->switch_onboard.pin == pdata->switch_extport.pin)) {
		if (!strcmp(host_sw, "onboard")) 
			pdata->gpio_init(&pdata->switch_extport);
		else 
			pdata->gpio_init(&pdata->switch_onboard);
	}

	/* power off for usb device */
	if(pdata->gpio_pwr_off.pin >= 0) 
		pdata->gpio_init(&pdata->gpio_pwr_off);
}

static int __devexit
akotg_hc_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct akotghc_usb_platform_data *pdata = NULL;

#ifdef CONFIG_USB_AKOTG_DMA        
	struct akotg_usbhc *akotghc;
#endif

    pdata = pdev->dev.platform_data;
    if (pdata != NULL)
    	/* hwinit power off */
	    usb_poweroff_for_device(pdata);
	
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

#ifdef CONFIG_USB_AKOTG_DMA        

    //free dma irq
	akotghc = hcd_to_akotg_usbhc(hcd);
    if(akotghc->dma_irq > 0)
    {
        free_irq(akotghc->dma_irq, hcd);
        akotghc->dma_irq = -1;
    }

#endif

	return 0;
}

static int __devinit
akotg_hc_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct akotg_usbhc *akotghc;
	struct resource	*res;
	struct akotghc_usb_platform_data *pdata = NULL;
	int			irq;
	int			retval = 0;
	unsigned long		irqflags;
	int i, j;

	pdata = pdev->dev.platform_data;
	if (pdata == NULL) 
		return -ENODEV;
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) 
		return -ENODEV;

	/* allocate and initialize hcd */
	hcd = usb_create_hcd(&akhs_otg_driver, &pdev->dev, akhs_otg_driver.product_desc);
	if (!hcd) {
		retval = -ENOMEM;
		goto err_nomem;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);
	akotghc = hcd_to_akotg_usbhc(hcd);
	
	usb_poweron_for_device(pdata);
	
	akotghc->clk = clk_get(&pdev->dev, "usb-host");
	if (IS_ERR(akotghc->clk)) {
		dbg("usb otg hs clocks missing\n");
		akotghc->clk = NULL;
		goto err_nomem;
	}

	/* basic sanity checks first.  board-specific init logic should
	 * have initialized these three resources and probably board
	 * specific platform_data.  we don't probe for IRQs, and do only
	 * minimal sanity checking.
	 */
	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev,	"Found HC with no IRQ. Check %s setup!\n", 
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto err_nodev;
	}
	akotghc->mcu_irq = irq;

	spin_lock_init(&akotghc->lock);
	INIT_LIST_HEAD(&akotghc->async_ep0);
	for(i=0; i<MAX_EP_NUM; i++)
		INIT_LIST_HEAD(&akotghc->async_epx[i]);

	akotghc->active_ep0 = NULL;
	for(j=0; j<MAX_EP_NUM; j++)
		akotghc->active_epx[j] = NULL;

	init_timer(&akotghc->timer);
	akotghc->timer.function = akotg_usbhc_timer;
	akotghc->timer.data = (unsigned long) akotghc;
	
	hcd->speed = HCD_USB2;
	//hcd->power_budget = 0; //or get from platfrom data

	/* The chip's IRQ is level triggered, active high.  A requirement
	 * for platform device setup is to cope with things like signal
	 * inverters (e.g. CF is active low) or working only with edge
	 * triggers (e.g. most ARM CPUs).  Initial driver stress testing
	 * was on a system with single edge triggering, so most sorts of
	 * triggering arrangement should work.
	 *
	 * Use resource IRQ flags if set by platform device setup.
	 */
	irqflags = IRQF_SHARED;
	retval = usb_add_hcd(hcd, irq, IRQF_DISABLED | irqflags);
	if (retval != 0)
		goto err_addhcd;

	init_epfifo_mapping(&akotg_epfifo_mapping);

#ifdef CONFIG_USB_AKOTG_DMA        

	akotg_dma_init(akotghc);

    //request dma irq
    irq = platform_get_irq(pdev, 1);
	retval = request_irq(irq, akotg_dma_irq, IRQF_DISABLED, "otgdma", hcd);
	if (retval < 0){
    	akotghc->dma_irq = -1;
		printk("request irq %d failed\n", irq);
	}
    else {
    	akotghc->dma_irq = irq;
    }

#endif
    
	printk("Usb otg-hs controller driver initialized\n");
	return retval;

 err_addhcd:
	usb_put_hcd(hcd);
 err_nomem:
 err_nodev:
	
	return retval;
}

#ifdef	CONFIG_PM

/* for this device there's no useful distinction between the controller
 * and its root hub, except that the root hub only gets direct PM calls
 * when CONFIG_USB_SUSPEND is enabled.
 */
static int
akotg_hc_suspend(struct platform_device *pdev, pm_message_t state)
{
	int		retval = 0;

	switch (state.event) {
	case PM_EVENT_FREEZE:
		break;
	case PM_EVENT_SUSPEND:
	case PM_EVENT_HIBERNATE:
	case PM_EVENT_PRETHAW:		/* explicitly discard hw state */
		break;
	}
	return retval;
}

static int
akotg_hc_resume(struct platform_device *dev)
{
	return 0;
}

#else
#define	akotg_hc_suspend	NULL
#define	akotg_hc_resume	NULL
#endif


struct platform_driver akotg_hc_driver = {
	.probe =	akotg_hc_probe,
	.remove =	__devexit_p(akotg_hc_remove),

	.suspend =	akotg_hc_suspend,
	.resume =	akotg_hc_resume,
	.driver = {
		.name =	(char *) hcd_name,
		.owner = THIS_MODULE,
	},
};

/*-------------------------------------------------------------------------*/

static int __init akotg_hc_init(void)
{

	if (usb_disabled())
		return -ENODEV;

	return platform_driver_register(&akotg_hc_driver);
}
module_init(akotg_hc_init);

static void __exit akotg_hc_cleanup(void)
{
	platform_driver_unregister(&akotg_hc_driver);
	flush_workqueue(g_otghc_wq);
	destroy_workqueue(g_otghc_wq);
	g_otghc_wq = NULL;
}
module_exit(akotg_hc_cleanup);
MODULE_DESCRIPTION("Anyka Host Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ak_hcd");
