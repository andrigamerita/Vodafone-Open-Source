/*
 * UHCI HCD (Host Controller Driver) PCI Bus Glue.
 *
 * Extracted from uhci-hcd.c:
 * Maintainer: Alan Stern <stern@rowland.harvard.edu>
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999-2002 Johannes Erdfelt, johannes@erdfelt.com
 * (C) Copyright 1999 Randy Dunlap
 * (C) Copyright 1999 Georg Acher, acher@in.tum.de
 * (C) Copyright 1999 Deti Fliegl, deti@fliegl.de
 * (C) Copyright 1999 Thomas Sailer, sailer@ife.ee.ethz.ch
 * (C) Copyright 1999 Roman Weissgaerber, weissg@vienna.at
 * (C) Copyright 2000 Yggdrasil Computing, Inc. (port of new PCI interface
 *               support from usb-ohci.c by Adam Richter, adam@yggdrasil.com).
 * (C) Copyright 1999 Gregory P. Smith (from usb-ohci.c)
 * (C) Copyright 2004-2007 Alan Stern, stern@rowland.harvard.edu
 */

#include "pci-quirks.h"

/*
 * Make sure the controller is completely inactive, unable to
 * generate interrupts or do DMA.
 */
static void uhci_pci_reset_hc(struct uhci_hcd *uhci)
{
	uhci_reset_hc(to_pci_dev(uhci_dev(uhci)), uhci->io_addr);
}

/*
 * Initialize a controller that was newly discovered or has just been
 * resumed.  In either case we can't be sure of its previous state.
 *
 * Returns: 1 if the controller was reset, 0 otherwise.
 */
static int uhci_pci_check_and_reset_hc(struct uhci_hcd *uhci)
{
	return uhci_check_and_reset_hc(to_pci_dev(uhci_dev(uhci)),
				uhci->io_addr);
}

static void uhci_pci_configure_hc(struct uhci_hcd *uhci)
{
	struct pci_dev *pdev = to_pci_dev(uhci_dev(uhci));

	/* Enable PIRQ */
	pci_write_config_word(pdev, USBLEGSUP, USBLEGSUP_DEFAULT);
}

static int uhci_pci_resume_detect_interrupts_are_broken(struct uhci_hcd *uhci)
{
	int port;

	switch (to_pci_dev(uhci_dev(uhci))->vendor) {
	    default:
		break;

	    case PCI_VENDOR_ID_GENESYS:
		/* Genesys Logic's GL880S controllers don't generate
		 * resume-detect interrupts.
		 */
		return 1;

	    case PCI_VENDOR_ID_INTEL:
		/* Some of Intel's USB controllers have a bug that causes
		 * resume-detect interrupts if any port has an over-current
		 * condition.  To make matters worse, some motherboards
		 * hardwire unused USB ports' over-current inputs active!
		 * To prevent problems, we will not enable resume-detect
		 * interrupts if any ports are OC.
		 */
		for (port = 0; port < uhci->rh_numports; ++port) {
			if (uhci_readw(uhci, uhci->io_addr + USBPORTSC1 + port * 2) &
					USBPORTSC_OC)
				return 1;
		}
		break;
	}
	return 0;
}

static int uhci_pci_remote_wakeup_is_broken(struct uhci_hcd *uhci)
{
	int port;
	char *sys_info;
	static char bad_Asus_board[] = "A7V8X";

	/* One of Asus's motherboards has a bug which causes it to
	 * wake up immediately from suspend-to-RAM if any of the ports
	 * are connected.  In such cases we will not set EGSM.
	 */
	sys_info = dmi_get_system_info(DMI_BOARD_NAME);
	if (sys_info && !strcmp(sys_info, bad_Asus_board)) {
		for (port = 0; port < uhci->rh_numports; ++port) {
			if (uhci_readw(uhci, uhci->io_addr + USBPORTSC1 + port * 2) &
					USBPORTSC_CCS)
				return 1;
		}
	}

	return 0;
}

static int uhci_pci_init(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	uhci->io_addr = (unsigned long) hcd->rsrc_start;

	if (to_pci_dev(uhci_dev(uhci))->vendor == PCI_VENDOR_ID_VIA)
		if (to_pci_dev(uhci_dev(uhci))->device == PCI_DEVICE_ID_VIA_82C586_2) {
#ifdef CONFIG_USB_UHCI_BIG_ENDIAN_MMIO
			uhci->big_endian_mmio = 1;
#else
			dev_warn(uhci_dev(uhci),
				  "unsupported big endian VIA Vectro VT6212L quirk\n");
#endif
		}

	uhci->rh_numports = uhci_count_ports(hcd);

	/* Intel controllers report the OverCurrent bit active on.
	 * VIA controllers report it active off, so we'll adjust the
	 * bit value.  (It's not standardized in the UHCI spec.)
	 */
	if (to_pci_dev(uhci_dev(uhci))->vendor == PCI_VENDOR_ID_VIA)
		uhci->oc_low = 1;

	/* HP's server management chip requires a longer port reset delay. */
	if (to_pci_dev(uhci_dev(uhci))->vendor == PCI_VENDOR_ID_HP)
		uhci->wait_for_hp = 1;

	/* Set up pointers to PCI-specific functions */
	uhci->reset_hc = uhci_pci_reset_hc;
	uhci->check_and_reset_hc = uhci_pci_check_and_reset_hc;
	uhci->configure_hc = uhci_pci_configure_hc;
	uhci->resume_detect_interrupts_are_broken =
		uhci_pci_resume_detect_interrupts_are_broken;
	uhci->remote_wakeup_is_broken=
		uhci_pci_remote_wakeup_is_broken;


	/* Kick BIOS off this hardware and reset if the controller
	 * isn't already safely quiescent.
	 */
	check_and_reset_hc(uhci);
	return 0;
}

/* Make sure the controller is quiescent and that we're not using it
 * any more.  This is mainly for the benefit of programs which, like kexec,
 * expect the hardware to be idle: not doing DMA or generating IRQs.
 *
 * This routine may be called in a damaged or failing kernel.  Hence we
 * do not acquire the spinlock before shutting down the controller.
 */
static void uhci_shutdown(struct pci_dev *pdev)
{
	struct usb_hcd *hcd = (struct usb_hcd *) pci_get_drvdata(pdev);

	uhci_hc_died(hcd_to_uhci(hcd));
}

#ifdef CONFIG_PM
static int uhci_pci_suspend(struct usb_hcd *hcd, pm_message_t message)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	int rc = 0;

	dev_dbg(uhci_dev(uhci), "%s\n", __FUNCTION__);

	spin_lock_irq(&uhci->lock);
	if (!test_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags) || uhci->dead)
		goto done_okay;		/* Already suspended or dead */

	if (uhci->rh_state > UHCI_RH_SUSPENDED) {
		dev_warn(uhci_dev(uhci), "Root hub isn't suspended!\n");
		rc = -EBUSY;
		goto done;
	};

	/* All PCI host controllers are required to disable IRQ generation
	 * at the source, so we must turn off PIRQ.
	 */
	pci_write_config_word(to_pci_dev(uhci_dev(uhci)), USBLEGSUP, 0);
	mb();
	hcd->poll_rh = 0;

	/* FIXME: Enable non-PME# remote wakeup? */

	/* make sure snapshot being resumed re-enumerates everything */
	if (message.event == PM_EVENT_PRETHAW)
		uhci_hc_died(uhci);

done_okay:
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
done:
	spin_unlock_irq(&uhci->lock);
	return rc;
}

static int uhci_pci_resume(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	dev_dbg(uhci_dev(uhci), "%s\n", __FUNCTION__);

	/* Since we aren't in D3 any more, it's safe to set this flag
	 * even if the controller was dead.
	 */
	set_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	mb();

	spin_lock_irq(&uhci->lock);

	/* FIXME: Disable non-PME# remote wakeup? */

	/* The firmware or a boot kernel may have changed the controller
	 * settings during a system wakeup.  Check it and reconfigure
	 * to avoid problems.
	 */
	check_and_reset_hc(uhci);

	/* If the controller was dead before, it's back alive now */
	configure_hc(uhci);

	if (uhci->rh_state == UHCI_RH_RESET) {

		/* The controller had to be reset */
		usb_root_hub_lost_power(hcd->self.root_hub);
		suspend_rh(uhci, UHCI_RH_SUSPENDED);
	}

	spin_unlock_irq(&uhci->lock);

	if (!uhci->working_RD) {
		/* Suspended root hub needs to be polled */
		hcd->poll_rh = 1;
		usb_hcd_poll_rh_status(hcd);
	}
	return 0;
}
#endif

static const struct hc_driver uhci_driver = {
	.description =		hcd_name,
	.product_desc =		"UHCI Host Controller",
	.hcd_priv_size =	sizeof(struct uhci_hcd),

	/* Generic hardware linkage */
	.irq =			uhci_irq,
	.flags =		HCD_USB11,

	/* Basic lifecycle operations */
	.reset =		uhci_pci_init,
	.start =		uhci_start,
#ifdef CONFIG_PM
	.suspend =		uhci_pci_suspend,
	.resume =		uhci_pci_resume,
	.bus_suspend =		uhci_rh_suspend,
	.bus_resume =		uhci_rh_resume,
#endif
	.stop =			uhci_stop,

	.urb_enqueue =		uhci_urb_enqueue,
	.urb_dequeue =		uhci_urb_dequeue,

	.endpoint_disable =	uhci_hcd_endpoint_disable,
	.get_frame_number =	uhci_hcd_get_frame_number,

	.hub_status_data =	uhci_hub_status_data,
	.hub_control =		uhci_hub_control,
};

static const struct pci_device_id uhci_pci_ids[] = { {
	/* handle any USB UHCI controller */
	PCI_DEVICE_CLASS(PCI_CLASS_SERIAL_USB_UHCI, ~0),
	.driver_data =	(unsigned long) &uhci_driver,
	}, { /* end: all zeroes */ }
};

MODULE_DEVICE_TABLE(pci, uhci_pci_ids);

static struct pci_driver uhci_pci_driver = {
	.name =		(char *)hcd_name,
	.id_table =	uhci_pci_ids,

	.probe =	usb_hcd_pci_probe,
	.remove =	usb_hcd_pci_remove,
	.shutdown =	uhci_shutdown,

#ifdef	CONFIG_PM
	.suspend =	usb_hcd_pci_suspend,
	.resume =	usb_hcd_pci_resume,
#endif	/* PM */
};
 
