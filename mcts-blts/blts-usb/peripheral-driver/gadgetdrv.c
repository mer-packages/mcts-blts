/*
 * gadgetdrv.c -- BLTS USB peripheral-side passthrough driver
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/usb.h>
#include <linux/utsname.h>
#include <linux/device.h>

#include "ep_config.h"
#include "gadget_chips.h"
#include "gadgetdrv.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB peripheral-side test passthrough driver");

#define DRIVER_VERSION "0.1"

static const char longname[] = "BLTS Gadget";

static struct usb_device_descriptor device_desc = {
	.bLength = sizeof device_desc,
	.bDescriptorType = USB_DT_DEVICE,

	.bcdUSB = __constant_cpu_to_le16(0x0200),
	.bDeviceClass = USB_CLASS_VENDOR_SPEC,

	.idVendor = __constant_cpu_to_le16(USB_BLTS_VENDOR_ID),
	.idProduct = __constant_cpu_to_le16(USB_BLTS_PRODUCT_ID),
	.bNumConfigurations = 2,
};

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

static char manufacturer[50];

/* default serial number takes at least two packets */
static char serial[] = "0123456789.0123456789.0123456789";

static struct usb_string strings_dev[] = {
	[STRING_MANUFACTURER_IDX].s = manufacturer,
	[STRING_PRODUCT_IDX].s = longname,
	[STRING_SERIAL_IDX].s = serial,
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409, /* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static int gadget_bind(struct usb_composite_dev *cdev);
static void gadgetdrv_suspend(struct usb_composite_dev *cdev);
static void gadgetdrv_resume(struct usb_composite_dev *cdev);

static struct usb_composite_driver gadget_driver = {
	.name		= "blts_gadget",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= gadget_bind,
	.speed	= USB_SPEED_UNKNOWN,
	.suspend = gadgetdrv_suspend,
	.resume	= gadgetdrv_resume
};

static struct gadgetdrv_data *glob_priv_data;

/* We'll lock the whole driver only when we do housekeeping. */
DEFINE_MUTEX(gadgetdrv_lock);

/*
 * USB transfer handling
 */

static struct usb_request *alloc_ep_req(struct usb_ep *ep, unsigned len)
{
	struct usb_request *req;

	GDTDEBUG("allocate usb request, len %d\n", len);

	if (!ep || !len)
		return NULL;

	req = usb_ep_alloc_request(ep, GFP_ATOMIC);
	if (req) {
		req->length = len;
		req->buf = kmalloc(len, GFP_ATOMIC);
		if (!req->buf) {
			usb_ep_free_request(ep, req);
			req = NULL;
		}
	}
	return req;
}

void free_ep_req(struct usb_ep *ep, struct usb_request *req)
{
	if (!req)
		return;

	kfree(req->buf);
	if (ep)
		usb_ep_free_request(ep, req);
}

static void ep_req_complete(struct usb_ep *ep, struct usb_request *req)
{
	int status = req->status;
	struct gadgetdrv_buffer *buf = req->context;

	switch (status) {
	case 0:
		/* request completed ok */
		break;
	case -ECONNABORTED:
		GDTDEBUG("%s: hardware forced ep reset\n", ep->name);
		break;
	case -ECONNRESET:
		GDTDEBUG("%s: request dequeued\n", ep->name);
		break;
	case -ESHUTDOWN:
		GDTDEBUG("%s: disconnected\n", ep->name);
		break;
	case -EOVERFLOW:
		GDTDEBUG("%s: buffer overrun\n", ep->name);
		break;
	case -EREMOTEIO:
		GDTDEBUG("%s: short read\n", ep->name);
		break;
	default:
		GDTDEBUG("%s: unknown (%d)\n", ep->name, status);
		break;
	}

	if (buf) {
		buf->usb_busy = 0;
		wake_up_interruptible(&buf->usbq);
	} else {
		GDTERR("No buffer associated with request!\n");
	}
}

static enum usb_device_speed gadgetdrv_speed_to_usb(
	enum usbdrv_device_speed speed)
{
	switch (speed) {
	case USBDRV_SPEED_LOW:
		return USB_SPEED_LOW;
	case USBDRV_SPEED_FULL:
		return USB_SPEED_FULL;
	case USBDRV_SPEED_HIGH:
		return USB_SPEED_HIGH;
	case USBDRV_SPEED_UNKNOWN:
		return USB_SPEED_UNKNOWN;
	}
	return USB_SPEED_UNKNOWN;
}

static int usb_ep_queue_and_wait(unsigned ep_num, struct gadgetdrv_buffer *buf)
{
	int retval;

	buf->busy = 1;
	buf->usb_busy = 1;
	buf->req->status = 0;

	retval = usb_ep_queue(glob_priv_data->ep[ep_num], buf->req, GFP_ATOMIC);
	if (retval) {
		GDTERR("usb_ep_queue failed %d\n", retval);
		goto done;
	}

	while (buf->usb_busy) {
		GDTTRACE("waiting for completion...\n");
		mutex_unlock(&gadgetdrv_lock);
		if (wait_event_interruptible(buf->usbq,
			!buf->usb_busy)) {
			usb_ep_dequeue(glob_priv_data->ep[ep_num], buf->req);
			retval = -EFAULT;
			goto done;
		}

		if (mutex_lock_interruptible(&gadgetdrv_lock)) {
			retval = -ERESTARTSYS;
			goto done;
		}
	}

	if (buf->req) {
		if (buf->req->status) {
			/* reset ep request in case there was some error */
			retval = buf->req->status;
			free_ep_req(glob_priv_data->ep[ep_num], buf->req);
			buf->req = NULL;
		}
	} else
		retval = -ECONNRESET;

done:
	return retval;
}

/*
 * Userspace interface
 */
loff_t gadgetdrv_llseek(struct file *filp, loff_t off, int whence)
{
	/* This is technically unnecessary for now, but we may use the position
	 * for something later */
	return -EINVAL;
}

ssize_t gadgetdrv_read(struct file *filp, char __user *usr_buf,
	size_t count, loff_t *offp)
{
	struct gadgetdrv_buffer *buf;
	ssize_t retval = -EIO;
	unsigned cdev_minor;
	int ep_num, max_packet;

	cdev_minor = iminor(filp->f_dentry->d_inode);
	GDTTRACE("start read for dev %d\n", cdev_minor);

	/* Each minor number has 2 buffers: minor * 2 == data read from
	 * endpoint, minor * 2 + 1 == data to write to endpoint. */
	ep_num = cdev_minor;
	buf = &glob_priv_data->buf[cdev_minor << 1];

	if (!glob_priv_data->ep[ep_num]) {
		GDTDEBUG("endpoint %d not configured yet!\n", ep_num);
		return -ENOTCONN;
	}

	if (mutex_lock_interruptible(&gadgetdrv_lock))
		return -ERESTARTSYS;

	while (buf->busy) {
		GDTDEBUG("Driver busy, waiting...\n");
		mutex_unlock(&gadgetdrv_lock);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(buf->userq, !buf->busy))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&gadgetdrv_lock))
			return -ERESTARTSYS;
	}

	max_packet = glob_priv_data->config_state.
		endpoints[ep_num].transfer_size;

	if (count > max_packet)
		count = max_packet;

	if (!buf->req) {
		buf->req = alloc_ep_req(glob_priv_data->ep[ep_num], max_packet);
		if (!buf->req) {
			retval = -ENOMEM;
			goto done;
		}
		buf->req->complete = ep_req_complete;
		buf->req->context = buf;
	}

	buf->req->length = count;
	retval = usb_ep_queue_and_wait(ep_num, buf);
	if (retval < 0)
		goto done;

	if (buf->req->actual != count)
		GDTDEBUG("Read %d bytes, expected %d\n", buf->req->actual,
			count);

	if (copy_to_user(usr_buf, buf->req->buf, buf->req->actual))
		retval = -EFAULT;
	else
		retval = buf->req->actual;

done:
	if (mutex_is_locked(&gadgetdrv_lock))
		mutex_unlock(&gadgetdrv_lock);
	GDTTRACE("R: %d\n", retval);
	buf->busy = 0;
	wake_up_interruptible(&buf->userq);
	return retval;
}

ssize_t gadgetdrv_write(struct file *filp, const char __user *usr_buf,
	size_t count, loff_t *offp)
{
	struct gadgetdrv_buffer *buf;
	ssize_t retval = -EIO;
	unsigned cdev_minor;
	int ep_num, max_packet;

	cdev_minor = iminor(filp->f_dentry->d_inode);
	GDTTRACE("start write for dev %d\n", cdev_minor);

	ep_num = cdev_minor;
	buf = &glob_priv_data->buf[1 + (cdev_minor << 1)];

	if (!glob_priv_data->ep[ep_num]) {
		GDTDEBUG("endpoint %d not configured yet!\n", ep_num);
		return -ENOTCONN;
	}

	if (mutex_lock_interruptible(&gadgetdrv_lock))
		return -ERESTARTSYS;

	while (buf->busy) {
		mutex_unlock(&gadgetdrv_lock);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(buf->userq, !buf->busy))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&gadgetdrv_lock))
			return -ERESTARTSYS;
	}

	max_packet = glob_priv_data->config_state.
		endpoints[ep_num].transfer_size;

	if (count > max_packet)
		count = max_packet;

	if (!buf->req) {
		buf->req = alloc_ep_req(glob_priv_data->ep[ep_num], max_packet);
		if (!buf->req) {
			retval = -ENOMEM;
			goto done;
		}
		buf->req->complete = ep_req_complete;
		buf->req->context = buf;
	}

	if (copy_from_user(buf->req->buf, usr_buf, count)) {
		retval = -EFAULT;
		goto done;
	}

	buf->req->length = count;
	retval = usb_ep_queue_and_wait(ep_num, buf);
	if (retval < 0)
		goto done;

	if (buf->req->actual != count)
		GDTDEBUG("Wrote %d bytes, expected %d\n", buf->req->actual,
			count);

	retval = buf->req->actual;

done:
	if (mutex_is_locked(&gadgetdrv_lock))
		mutex_unlock(&gadgetdrv_lock);
	GDTTRACE("W: %d\n", retval);
	buf->busy = 0;
	wake_up_interruptible(&buf->userq);
	return retval;
}

int gadgetdrv_open(struct inode *inode, struct file *filp)
{
	int ret = -ENOMEM;
	unsigned cminor, epstate;

	if (mutex_lock_interruptible(&gadgetdrv_lock))
		return -EBUSY;

	cminor = iminor(filp->f_dentry->d_inode);
	epstate = glob_priv_data->config_state.endpoints[cminor].type;

	GDTDEBUG("device %u opened (mode=%u) (epstate=%u)\n",
		cminor, filp->f_mode, epstate);

	ret = 0;
	mutex_unlock(&gadgetdrv_lock);

	return ret;
}

int gadgetdrv_release(struct inode *inode, struct file *filp)
{
	int cdev_minor;
	struct gadgetdrv_buffer *buf;

	BUG_ON(mutex_lock_interruptible(&gadgetdrv_lock));

	cdev_minor = iminor(filp->f_dentry->d_inode);
	buf = &glob_priv_data->buf[cdev_minor << 1];
	if (buf->req) {
		if (buf->usb_busy)
			usb_ep_dequeue(glob_priv_data->ep[cdev_minor],
				buf->req);
		free_ep_req(glob_priv_data->ep[cdev_minor], buf->req);
		buf->req = NULL;
	}

	buf = &glob_priv_data->buf[1 + (cdev_minor << 1)];
	if (buf->req) {
		if (buf->usb_busy)
			usb_ep_dequeue(glob_priv_data->ep[cdev_minor],
				buf->req);
		free_ep_req(glob_priv_data->ep[cdev_minor], buf->req);
		buf->req = NULL;
	}

	GDTDEBUG("device released\n");
	mutex_unlock(&gadgetdrv_lock);
	return 0;
}

int gadgetdrv_ioctl(struct inode *inode, struct file *filp,
	unsigned cmd, unsigned long arg)
{
	int err = 0;
	int i;
	struct gadgetdrv_endpoint_config *ep_conf;

	if (_IOC_TYPE(cmd) != GADGETDRV_IOC_MAGIC) {
		err = -ENOTTY;
		goto done_not_locked;
	}

	if (_IOC_NR(cmd) > GADGETDRV_IOC_MAX) {
		err = -ENOTTY;
		goto done_not_locked;
	}

	if (mutex_lock_interruptible(&gadgetdrv_lock)) {
		err = -ERESTARTSYS;
		goto done_not_locked;
	}

	switch (cmd) {
	case GADGETDRV_IOCGCONF:
		if (copy_to_user((void __user *) arg,
			&glob_priv_data->config_state, _IOC_SIZE(cmd)))
			err = -EFAULT;
		break;
	case GADGETDRV_IOCSCONF:
		if (glob_priv_data->started) {
			GDTERR("SCONF: gadget running! cannot change "\
				"configuration\n");
			err = -EPERM;
			break;
		}

		if (copy_from_user(&glob_priv_data->config_state,
			(void __user *) arg,
			_IOC_SIZE(cmd))) {
			err = -EFAULT;
			break;
		}
		ep_conf = glob_priv_data->config_state.endpoints;
		for (i = 0; i < N_GADGETDRV_DEVICES; i++)
			if (!ep_conf[i].transfer_size)
				ep_conf[i].transfer_size =
					ep_conf[i].max_packet_size;
		break;
	case GADGETDRV_IOCSTART:
		if (glob_priv_data->started) {
			GDTERR("START: gadget already started!\n");
			err = -EPERM;
			break;
		}
		GDTDEBUG("START: start called, registering gadget\n");
		gadget_driver.speed = gadgetdrv_speed_to_usb(
			glob_priv_data->config_state.speed);
		err = usb_composite_register(&gadget_driver);
		if (err) {
			GDTERR("START: failed to register gadget driver (%d)\n",
				err);
			break;
		}
		glob_priv_data->started = 1;
		break;
	case GADGETDRV_IOCSTOP:
		/* TODO: Cleanup all ep reqs etc. */
		if (!glob_priv_data->started) {
			GDTERR("STOP: gadget not started yet\n");
			err = -ENODEV;
			break;
		}
		usb_composite_unregister(&gadget_driver);
		glob_priv_data->started = 0;
		break;
	}

	mutex_unlock(&gadgetdrv_lock);
done_not_locked:
	return err;
}

static int gadget_bind(struct usb_composite_dev *cdev)
{
	int gcnum;
	struct usb_gadget *gadget = cdev->gadget;
	int id;
	int err;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;

	gcnum = usb_gadget_controller_number(gadget);

	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		GDTDEBUG("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	GDTDEBUG("%s, version: " DRIVER_VERSION "\n", longname);

	snprintf(manufacturer, sizeof manufacturer, "%s %s with %s",
		init_utsname()->sysname, init_utsname()->release,
		gadget->name);

	glob_priv_data->composite_dev = cdev;

	err = register_eps(cdev, glob_priv_data);
	if (err < 0)
		return err;

	return 0;
}

static void gadgetdrv_suspend(struct usb_composite_dev *cdev)
{
	GDTDEBUG("gadgetdrv_suspend\n");
}

static void gadgetdrv_resume(struct usb_composite_dev *cdev)
{
	GDTDEBUG("gadgetdrv_resume\n");
}

/*
 * Initialisation and cleanup
 */
const struct file_operations gadgetdrv_fops = {
	.owner = THIS_MODULE,
	.read = gadgetdrv_read,
	.write = gadgetdrv_write,
	.open = gadgetdrv_open,
	.release = gadgetdrv_release,
	.ioctl = gadgetdrv_ioctl,
	.llseek = gadgetdrv_llseek,
};

static int __init gadgetdrv_chardev_init(struct gadgetdrv_data *data)
{
	int err;
	err = alloc_chrdev_region(&data->devno, 0,
		N_GADGETDRV_DEVICES, "blts_gadget");

	if (err) {
		GDTDEBUG("cannot allocate major number\n");
		return -1;
	}

	cdev_init(&data->cdev, &gadgetdrv_fops);
	err = cdev_add(&data->cdev, data->devno, N_GADGETDRV_DEVICES);

	if (err) {
		GDTDEBUG("cannot add device\n");
		unregister_chrdev_region(data->devno, N_GADGETDRV_DEVICES);
	}

	return err;
}

static int __init gadgetdrv_buf_init(struct gadgetdrv_data *data)
{
	int i;

	for (i = 0; i < N_GADGETDRV_DEVICES * 2; ++i) {
		init_waitqueue_head(&data->buf[i].userq);
		init_waitqueue_head(&data->buf[i].usbq);
	}
	return 0;
}

static void gadgetdrv_chardev_exit(struct gadgetdrv_data *data)
{
	if (!data)
		return;

	mutex_lock(&gadgetdrv_lock);
	if (data->devno) {
		cdev_del(&data->cdev);
		unregister_chrdev_region(data->devno, N_GADGETDRV_DEVICES);
		data->devno = 0;
	}
	mutex_unlock(&gadgetdrv_lock);
}

static int __init gadgetdrv_init(void)
{
	int err;
	struct gadgetdrv_data *data;

	err = sysfs_log_add();
	if (err)
		return err;

	GDTDEBUG("init\n");

	data = kzalloc(sizeof *data, GFP_KERNEL);
	if (!data) {
		GDTERR("oom\n");
		err = -ENOMEM;
		goto error;
	}

	glob_priv_data = data;

	mutex_init(&gadgetdrv_lock);

	err = gadgetdrv_buf_init(data);
	if (err)
		goto error;

	err = gadgetdrv_chardev_init(data);
	if (err)
		goto error;

	GDTDEBUG("Registered major %u, minors %u-%u\n",
		MAJOR(data->devno), MINOR(data->devno),
		MINOR(data->devno) + N_GADGETDRV_DEVICES - 1);

	return 0;

error:
	if (data) {
		gadgetdrv_chardev_exit(data);
		kfree(data);
	}
	sysfs_log_remove();
	return err;
}

static void __exit gadgetdrv_exit(void)
{
	struct gadgetdrv_data *data = glob_priv_data;

	if (data) {
		if (data->started)
			usb_composite_unregister(&gadget_driver);
		gadgetdrv_chardev_exit(data);
		kfree(data);
	}
	GDTDEBUG("exit\n");
	sysfs_log_remove();
}

module_init(gadgetdrv_init);
module_exit(gadgetdrv_exit);

