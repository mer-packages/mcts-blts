/*
 * gadgetdrv.h -- BLTS USB peripheral-side passthrough driver definitions
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

#ifndef GADGETDRV_H
#define GADGETDRV_H

#include <linux/cdev.h>
#include "../driver-common/sysfs_log.h"
#include "../common/gadgetdrv_common.h"
#include "composite.h"

/* Driver buffer, 2 used by each endpoint (R/W separately) */
struct gadgetdrv_buffer {
	wait_queue_head_t userq; /* userspace IO wait queue */
	wait_queue_head_t usbq;  /* USB wait queue */
	struct usb_request *req;
	unsigned usb_busy:1;
	unsigned busy:1;       /* Writes from userspace block on this */
};

/* Private data for driver */
struct gadgetdrv_data {
	/* These are for the char device */
	dev_t devno;
	struct cdev cdev;

	struct usb_function function;
	struct usb_composite_dev *composite_dev;
	struct usb_ep *ep[N_GADGETDRV_DEVICES];
	struct gadgetdrv_buffer buf[N_GADGETDRV_DEVICES * 2];
	struct gadgetdrv_current_config config_state;
	unsigned started:1;
};

#define GDTTRACE(fmt, args...)

#define GDTDEBUG(fmt, args...) \
	do {\
		printk(KERN_DEBUG "bltsgadget: "fmt, ##args);\
		sysfs_log_write(KERN_DEBUG "bltsgadget: "fmt, ##args);\
	} while(0)

#define GDTERR(fmt, args...) \
	do {\
		printk(KERN_ERR "bltsgadget: "fmt, ##args);\
		sysfs_log_write(KERN_ERR "bltsgadget: "fmt, ##args);\
	} while(0)

/* TODO: Remove later */
struct usb_ep *gadgetdrv_usb_ep_autoconfig(struct usb_gadget *gadget,
	struct usb_endpoint_descriptor	*desc);
void gadgetdrv_usb_ep_autoconfig_reset(struct usb_gadget *gadget);

#endif /* GADGETDRV_H */

