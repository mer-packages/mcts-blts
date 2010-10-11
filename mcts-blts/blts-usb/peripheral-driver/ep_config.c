/*
 * ep_config.c -- BLTS USB peripheral-side passthrough driver endpoint
 * configurator
 *
 * Copyright (C) 2010 by Nokia Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/usb.h>
#include "gadgetdrv.h"
#include "ep_config.h"

static struct gadgetdrv_data *glob_priv_data;

static struct usb_string strings_enumloopback[] = {
	[0].s = "passthrough driver",
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_loop = {
	.language	= 0x0409, /* en-us */
	.strings	= strings_enumloopback,
};

static struct usb_gadget_strings *enumloopback_strings[] = {
	&stringtab_loop,
	NULL,
};

#ifdef CONFIG_USB_OTG
static struct usb_otg_descriptor otg_descriptor = {
	.bLength = sizeof otg_descriptor,
	.bDescriptorType = USB_DT_OTG,
	.bmAttributes = USB_OTG_SRP | USB_OTG_HNP,
};

const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};
#endif

static struct usb_interface_descriptor enumloopback_fs_intf = {
	.bLength = sizeof enumloopback_fs_intf,
	.bDescriptorType = USB_DT_INTERFACE,
	.bNumEndpoints = 0, /* set when binding */
	.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
};

static struct usb_interface_descriptor enumloopback_hs_intf = {
	.bLength = sizeof enumloopback_hs_intf,
	.bDescriptorType = USB_DT_INTERFACE,
	.bNumEndpoints = 0, /* set when binding */
	.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
};

static struct usb_descriptor_header *fs_descs[N_GADGETDRV_DEVICES + 1];
static struct usb_descriptor_header *hs_descs[N_GADGETDRV_DEVICES + 1];

static void disable_ep(struct usb_ep *ep)
{
	int retval;

	if (!ep) {
		GDTERR("no ep to disable!\n");
		return;
	}

	if (ep->driver_data) {
		retval = usb_ep_disable(ep);
		if (retval < 0)
			GDTDEBUG("disable ep failed %s; %d\n",
				ep->name, retval);
		ep->driver_data = NULL;
	}
}

static int config_type_to_xfer(enum usbdrv_endpoint_type type)
{
	switch (type) {
	case USBDRV_EP_TYPE_NONE:
		break;
	case USBDRV_EP_TYPE_CONTROL:
		return USB_ENDPOINT_XFER_CONTROL;
	case USBDRV_EP_TYPE_BULK:
		return USB_ENDPOINT_XFER_BULK;
	case USBDRV_EP_TYPE_ISOC:
		return USB_ENDPOINT_XFER_ISOC;
	case USBDRV_EP_TYPE_INT:
		return USB_ENDPOINT_XFER_INT;
	}

	return -EINVAL;
}

static unsigned config_direction_to_usb(
	enum usbdrv_endpoint_direction_flags flags)
{
	if (flags & USBDRV_EP_IN)
		return USB_DIR_IN;
	if (flags & USBDRV_EP_OUT)
		return USB_DIR_OUT;

	return 0;
}

static void log_ep_desc(struct usb_endpoint_descriptor *epd)
{
	if (!epd) {
		GDTERR("log_ep_desc: NULL endpoint descriptor!\n");
		return;
	}
	GDTDEBUG("Endpoint contents:\n");
	GDTDEBUG(" bLength %d\n", epd->bLength);
	GDTDEBUG(" bDescriptorType %d\n", epd->bDescriptorType);
	GDTDEBUG(" bEndpointAddress %d\n", epd->bEndpointAddress);
	GDTDEBUG(" bmAttributes %d\n", epd->bmAttributes);
	GDTDEBUG(" bInterval %d\n", epd->bInterval);
	GDTDEBUG(" wMaxPacketSize %d\n",
		__constant_le16_to_cpu(epd->wMaxPacketSize));
}

static struct usb_endpoint_descriptor *create_epdesc(
	struct usb_composite_dev *cdev,
	struct gadgetdrv_endpoint_config *conf)
{
	struct usb_endpoint_descriptor *epd;

	epd = kzalloc(sizeof(*epd), GFP_ATOMIC);
	if (!epd) {
		GDTERR("oom\n");
		return NULL;
	}

	epd->bLength = USB_DT_ENDPOINT_SIZE;
	epd->bDescriptorType = USB_DT_ENDPOINT,
	epd->bEndpointAddress =	config_direction_to_usb(conf->direction);
	epd->bmAttributes = config_type_to_xfer(conf->type);
	epd->bInterval = conf->interval;

	return epd;
}

static int enumloopback_bind(struct usb_configuration *c,
	struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_endpoint_descriptor *fs_epd, *hs_epd;
	enum usbdrv_device_speed speed;
	int	id, t, i = 1;

	GDTDEBUG("enumloopback bind\n");

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;

	enumloopback_fs_intf.bInterfaceNumber = id;
	enumloopback_hs_intf.bInterfaceNumber = id;

	speed = glob_priv_data->config_state.used_speed;

	glob_priv_data->config_state.endpoints[0].type =
		USBDRV_EP_TYPE_CONTROL;
	glob_priv_data->config_state.endpoints[0].direction =
		USBDRV_EP_IN | USBDRV_EP_OUT;
	glob_priv_data->config_state.endpoints[0].max_packet_size =
		cdev->gadget->ep0->maxpacket;
	glob_priv_data->config_state.endpoints[0].interval = 0;
	/* FIXME: writing/reading ep0 not supported yet */
	/* glob_priv_data->ep[0] = cdev->gadget->ep0; */

	/* create descriptors and configure endpoints */

	for (t = 1; t < N_GADGETDRV_DEVICES; t++) {
		if (glob_priv_data->config_state.endpoints[t].type ==
			USBDRV_EP_TYPE_NONE)
			continue;

		fs_epd = create_epdesc(cdev,
			&glob_priv_data->config_state.endpoints[t]);
		if (!fs_epd) {
			GDTERR("failed to create ep desc for ep %d\n", t);
			goto autoconf_fail;
		}

		glob_priv_data->ep[t] = gadgetdrv_usb_ep_autoconfig(
			cdev->gadget, fs_epd);
		if (!glob_priv_data->ep[t])
			goto autoconf_fail;

		glob_priv_data->ep[t]->driver_data = cdev;

		GDTDEBUG("-----------------------\n");
		GDTDEBUG("ep %s autoconfigured\n", glob_priv_data->ep[t]->name);

		GDTDEBUG("new full speed ep descriptor\n");
		log_ep_desc(fs_epd);
		fs_descs[i] = (struct usb_descriptor_header *)fs_epd;

		/* support high speed hardware */
		if (gadget_is_dualspeed(cdev->gadget)) {
			hs_epd = create_epdesc(cdev,
				&glob_priv_data->config_state.endpoints[t]);
			if (!hs_epd) {
				GDTERR("failed to create ep desc for ep %d\n",
					t);
				goto autoconf_fail;
			}

			hs_epd->wMaxPacketSize = __constant_cpu_to_le16(
				glob_priv_data->config_state.
				endpoints[t].max_packet_size);
			hs_epd->bEndpointAddress = fs_epd->bEndpointAddress;

			GDTDEBUG("new high speed ep descriptor\n");
			log_ep_desc(hs_epd);
			hs_descs[i] = (struct usb_descriptor_header *)hs_epd;
		}
		GDTDEBUG("-----------------------\n");

		i++;
	}

	if (speed == USBDRV_SPEED_FULL ||
		speed == USBDRV_SPEED_LOW ||
		speed == USBDRV_SPEED_UNKNOWN) {
		fs_descs[i] = NULL;
		enumloopback_fs_intf.bNumEndpoints = i - 1;
		fs_descs[0] = (struct usb_descriptor_header *)
			&enumloopback_fs_intf;
		f->descriptors = fs_descs;
		c->fullspeed = true;
	} else {
		memset(fs_descs, 0, sizeof(fs_descs));
		f->descriptors = NULL;
		c->fullspeed = false;
	}

	if (gadget_is_dualspeed(cdev->gadget) &&
		(speed == USBDRV_SPEED_HIGH ||
		speed == USBDRV_SPEED_UNKNOWN)) {
		hs_descs[i] = NULL;
		enumloopback_hs_intf.bNumEndpoints = i - 1;
		hs_descs[0] = (struct usb_descriptor_header *)
			&enumloopback_hs_intf;
		f->hs_descriptors = hs_descs;
		c->highspeed = true;
	} else {
		memset(hs_descs, 0, sizeof(hs_descs));
		f->hs_descriptors = NULL;
		c->highspeed = false;
	}

	GDTDEBUG("enumloopback bind ok, got %d eps\n", i - 1);

	return 0;

autoconf_fail:
	GDTERR("%s: can't autoconfigure on %s\n", f->name, cdev->gadget->name);
	return -ENODEV;
}

static void enumloopback_unbind(struct usb_configuration *c,
	struct usb_function *f)
{
	int i;

	GDTDEBUG("enumloopback unbind\n");

	/* first one is enumloopback_intf */
	i = 1;
	while (fs_descs[i])
		kfree(fs_descs[i++]);
	memset(fs_descs, 0, sizeof(fs_descs));

	i = 1;
	while (hs_descs[i])
		kfree(hs_descs[i++]);
	memset(hs_descs, 0, sizeof(hs_descs));

	memset(glob_priv_data->ep, 0, sizeof(glob_priv_data->ep));
}

static void enumloopback_disable(struct usb_function *f)
{
	int i;

	GDTDEBUG("disable enumloopback\n");

	for (i = 1; i < N_GADGETDRV_DEVICES; i++) {
		if (glob_priv_data->ep[i]) {
			GDTDEBUG("disabling ep %d\n", i);
			disable_ep(glob_priv_data->ep[i]);
		}
	}
}

static int enable_enumloopback(struct usb_composite_dev *cdev)
{
	int t, i = 1, result = 0;

	GDTDEBUG("enable enumloopback\n");

	for (t = 1; t < N_GADGETDRV_DEVICES; t++) {
		if (glob_priv_data->config_state.endpoints[t].type
			== USBDRV_EP_TYPE_NONE)
			continue;

		if (gadget_is_dualspeed(cdev->gadget) &&
			cdev->gadget->speed == USB_SPEED_HIGH && hs_descs[i])
			result = usb_ep_enable(glob_priv_data->ep[t],
				(struct usb_endpoint_descriptor *)hs_descs[i]);
		else if (fs_descs[i])
			result = usb_ep_enable(glob_priv_data->ep[t],
				(struct usb_endpoint_descriptor *)fs_descs[i]);

		if (result < 0) {
			GDTERR("failed to enable ep %d\n", t);
			goto fail_ep_start;
		}
		glob_priv_data->ep[t]->driver_data = cdev;
		i++;
	}

	return result;

fail_ep_start:
	GDTDEBUG("enable enumloopback failed %d\n", result);

	enumloopback_disable(NULL);
	return result;
}

static int enumloopback_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct usb_composite_dev *cdev = f->config->cdev;

	GDTDEBUG("enumloopback set alt\n");

	enumloopback_disable(f);
	return enable_enumloopback(cdev);
}

static int enumloopback_bind_config(struct usb_configuration *c)
{
	memset(&glob_priv_data->function, 0, sizeof(glob_priv_data->function));
	glob_priv_data->function.name = "enumloopback";
	glob_priv_data->function.descriptors = fs_descs;
	glob_priv_data->function.bind = enumloopback_bind;
	glob_priv_data->function.unbind = enumloopback_unbind;
	glob_priv_data->function.set_alt = enumloopback_set_alt;
	glob_priv_data->function.disable = enumloopback_disable;

	return usb_add_function(c, &glob_priv_data->function);
}

static struct usb_configuration enumloopback_driver = {
	.label = "enumloopback",
	.strings = enumloopback_strings,
	.bind = enumloopback_bind_config,
	.bConfigurationValue = 1,
	 /* TODO: Make this configurable */
	.bmAttributes = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower = 2, /* set before usb_add_config */
};

int register_eps(struct usb_composite_dev *cdev,
	struct gadgetdrv_data *data)
{
	int id;

	glob_priv_data = data;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_enumloopback[0].id = id;

	enumloopback_fs_intf.iInterface = id;
	enumloopback_hs_intf.iInterface = id;
	enumloopback_driver.iConfiguration = id;
	enumloopback_driver.bMaxPower = data->config_state.max_power / 2;

#ifdef CONFIG_USB_OTG
	if (gadget_is_otg(cdev->gadget)) {
		GDTDEBUG("using otg descriptor\n");
		enumloopback_driver.descriptors = otg_desc;
		enumloopback_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}
#endif

	GDTDEBUG("add usb config\n");
	return usb_add_config(cdev, &enumloopback_driver);
}

