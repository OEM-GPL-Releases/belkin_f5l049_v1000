/*
 * Atheros AR9130 EHCI Host Controller Driver
 *
 * Copyright (C) 2008 Atheros Communications, Inc.
 * Copyright (C) 2009 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <asm/mach-ar7100/ar9130_defs.h>

#ifdef CONFIG_USB_AR9130_OTG_MODULE
#warning "OTG enabled in host"
#define CONFIG_USB_AR9130_OTG
#endif

#ifdef	CONFIG_USB_AR9130_OTG
static int ar9130_start_hc(struct ehci_hcd *ehci, struct device *dev)
{
printk ("ar9130_start_hc %p, %p\n", ehci_to_hcd(ehci), &ehci_to_hcd(ehci)->self);
	if (ehci->transceiver) {
		int status = otg_set_host(ehci->transceiver,
					&ehci_to_hcd(ehci)->self);
		dev_info(dev, "init %s transceiver, status %d\n",
				ehci->transceiver->label, status);
		if (status) {
			if (ehci->transceiver)
				put_device(ehci->transceiver->dev);
		}
		return status;
	} else {
		dev_err(dev, "can't find transceiver\n");
		return -ENODEV;
	}
}
#endif

#ifdef	CONFIG_USB_OTG
void start_hnp(struct ehci_hcd *ehci)
{
	unsigned long	flags;
	otg_start_hnp(ehci->transceiver);

	local_irq_save(flags);
	ehci->transceiver->state = OTG_STATE_A_SUSPEND;
	local_irq_restore(flags);
}
#endif

static int ar9130_ehci_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval;

	ehci->caps = hcd->regs + 0x100;
	ehci->regs = hcd->regs + 0x100 +
		HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ar9130_debug_dev("HCS Params %x \n\n", ehci->hcs_params);
	ar9130_debug_dev("HCC Params %x \n",
		ehci_readl(ehci, &ehci->caps->hcc_params));

	ar9130_debug_dev("hcd->regs %p \n", hcd->regs);
	ar9130_debug_dev("Host Capability Reg %p \n", ehci->caps);
	ar9130_debug_dev("Host Operational Reg %p \n", ehci->regs);

	retval = ehci_init(hcd);
	if (retval < 0)
		return retval;

	ehci->is_tdi_rh_tt = 1;
	ehci->sbrn = 0x20;

	ehci_reset(ehci);
	ehci_port_power(ehci, 0);

	return retval;
}

static struct hc_driver ehci_hc_ar9130_driver = {
	.description        =   hcd_name,
	.product_desc       =   "ATH EHCI",
	.hcd_priv_size      =   sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
#ifndef CONFIG_USB_AR9130_OTG
	.irq                =   ehci_irq,
#endif
	.flags              =   HCD_MEMORY | HCD_USB2,
	/*
	 * basic lifecycle operations
	 */
	.reset              =   ar9130_ehci_init,
	.start              =   ehci_run,
#ifdef CONFIG_PM
	.suspend            =   ehci_bus_suspend,
	.resume             =   ehci_bus_resume,
#endif
	.stop               =   ehci_stop,
	.shutdown           =   ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue        =   ehci_urb_enqueue,
	.urb_dequeue        =   ehci_urb_dequeue,
	.endpoint_disable   =   ehci_endpoint_disable,
	/*
	 * scheduling support
	 */
	.get_frame_number   =   ehci_get_frame,
	/*
	 * root hub support
	 */
	.hub_status_data    =   ehci_hub_status_data,
	.hub_control        =   ehci_hub_control,
	.bus_suspend        =   ehci_bus_suspend,
	.bus_resume         =   ehci_bus_resume,
	.relinquish_port    =   ehci_relinquish_port,
};

#ifndef CONFIG_USB_AR9130_OTG

static void ar9130_usb_setup(struct usb_hcd *hcd)
{
	/* USB PHY suspend is controlled from the core */
	ar9130_reg_rmw_set(AR9130_RESET, AR9130_RESET_USBSUS_OVRIDE);
	mdelay(10);

	/* Reset the USB host controller */
	ar9130_reg_wr(AR9130_RESET,
		((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_HOST)) |
		AR9130_RESET_USBSUS_OVRIDE));
	mdelay(10);

	/* Reset the USB PHYs */
	ar9130_reg_wr(AR9130_RESET,
		((ar9130_reg_rd(AR9130_RESET) & ~(AR9130_RESET_USB_PHY)) |
		AR9130_RESET_USBSUS_OVRIDE));
	mdelay(10);
}

static int ehci_drv_ar9130_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct usb_hcd *hcd;
	void __iomem *regs;
	int retval;
	int irq;

	if (usb_disabled()) {
		ar9130_error("USB_DISABLED\n");
		retval = -ENODEV;
		goto err;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_dbg(&pdev->dev, "HC with no IRQ\n");
		retval = -ENODEV;
		goto err;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "HC with no resources\n");
		retval = -ENODEV;
		goto err;
	}

	if (!request_mem_region(res->start, res->end - res->start + 1,
		ehci_hc_ar9130_driver.description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto err;
	}

	regs = ioremap(res->start, res->end - res->start + 1);
	if (!regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto err1;
	}

	hcd = usb_create_hcd(&ehci_hc_ar9130_driver,
		&pdev->dev, pdev->dev.bus_id);
	if (!hcd) {
		retval = -ENOMEM;
		goto err2;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len   = res->end - res->start + 1;
	hcd->regs = regs;

	ar9130_usb_setup(hcd);

	retval = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (retval < 0)
		goto err3;

	return 0;

err3:
	usb_put_hcd(hcd);
err2:
	iounmap(regs);
err1:
	release_mem_region(res->start, res->end - res->start + 1);
err:
	dev_err(&pdev->dev, "init %s fail, %d \n", pdev->dev.bus_id, retval);
	return retval;
}

static int ehci_drv_ar9130_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	return 0;
}

MODULE_ALIAS("ar9130-ehci");

static struct platform_driver ehci_hcd_ar9130_driver = {
	.probe = ehci_drv_ar9130_probe,
	.remove = ehci_drv_ar9130_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver.name = "ar7100-ehci",
};

#else

int usb_otg_ar9130_probe(struct hc_driver *driver)
{
    struct ar9130_otg *ar9130_otg;
    struct usb_hcd *hcd;
    struct ehci_hcd *ehci;
    struct device *dev;
    int ret;

    ar9130_otg = ar9130_get_otg();
    if (ar9130_otg == NULL) {
        return -EINVAL;
    }
    dev = ar9130_otg->dev;

    hcd = usb_create_hcd(driver, dev, dev->bus_id);
    if(!hcd){
        ret = -ENOMEM;
        goto err;
    }
    hcd->rsrc_start = 0;
    hcd->rsrc_len   = 0;

    hcd->regs = ar9130_otg->reg_base;
    if(hcd->regs == NULL){
        dev_dbg(dev,"error mapping memory \n");
        ret = -EFAULT;
        goto err1;
    }

    /* EHCI Register offset 0x100 - Info from ChipIdea */
    ehci = hcd_to_ehci(hcd);
    ehci->caps = hcd->regs + 0x100;     /* Device/Host Capa Reg*/
    ehci->regs = hcd->regs + 0x140;     /* Device/Host Oper Reg*/

    ar9130_otg->ehci = ehci; /* Temp To Test HNP */

    printk("hcd->regs %p, %p \n", hcd, hcd->regs);
    printk("Host Capability Reg %p \n",ehci->caps);
    printk("Host Operational Reg %p \n",ehci->regs);

    ehci->transceiver = &ar9130_otg->otg;

    printk ("usb_add_hcd\n");
    ret = usb_add_hcd(hcd, 0, 0);
    if(ret != 0){
        goto err1;
    }
    dev_set_drvdata(dev, hcd);

    ehci_hc_ar9130_driver.irq = ehci_irq;
    ret = ar9130_start_hc(ehci, dev);
    if (ret != 0) {
        goto err1;
    }

    return ret;

err1:
    usb_put_hcd(hcd);
err:
    dev_err(dev,"init %s fail, %d \n", dev->bus_id, ret);
    return ret;
}
#endif
