/*
 * hostdrv.c -- BLTS USB host-side passthrough driver
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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/usb.h>

#include "../driver-common/sysfs_log.h"
#include "../common/hostdrv_common.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB host-side test passthrough driver");

/* If 1, log successful transfers, too. Very noisy. */
static unsigned log_transfer_success;

module_param(log_transfer_success, uint, 0);
MODULE_PARM_DESC(log_transfer_success, "0 = log only errors, 1 = log all URBs");
/* Per-URB transfer buffer. This will be passed to usb_alloc_coherent, and
   needs to be a multiple of max transfer size. */
#define USB_IOBUF_DMA_SIZE      (4096 << 2)

/* Smallest packet size we'll use for urbs. This influences memory allocated
   for the urbs in isoc case and is a lower limit on user-adjustable buffers. */
#define MIN_TRANSFER_BUFFER     32

/* g_zero is supported */
#define USB_ZERO_VENDOR_ID	0x0525
#define USB_ZERO_PRODUCT_ID	0xa4a0

/* table of devices that work with this driver */
static struct usb_device_id hstdrv_id_table[] = {
	{ USB_DEVICE(USB_BLTS_VENDOR_ID, USB_BLTS_PRODUCT_ID) },
	{ USB_DEVICE(USB_ZERO_VENDOR_ID, USB_ZERO_PRODUCT_ID) },
	{ }
};

MODULE_DEVICE_TABLE(usb, hstdrv_id_table);

/* Split logs: use common sysfs test logging as well as normal log. */
/* BHTRACE is only enabled with the log_transfer_success module parameter. */
#define BHTRACE(fmt, args...)						\
	do {								\
		if (log_transfer_success) {				\
			printk(KERN_DEBUG "blts_usb_host: "fmt, ##args); \
			sysfs_log_write(KERN_DEBUG "blts_usb_host: "fmt, \
					##args);			\
		}							\
	} while (0)

#define BHDEBUG(fmt, args...)						\
	do {								\
		printk(KERN_DEBUG "blts_usb_host: "fmt, ##args);	\
		sysfs_log_write(KERN_DEBUG "blts_usb_host: "fmt, ##args); \
	} while (0)

#define BHERR(fmt, args...) \
	do {\
		printk(KERN_ERR "blts_usb_host: "fmt, ##args);\
		sysfs_log_write(KERN_ERR "blts_usb_host: "fmt, ##args);\
	} while (0)

/* Driver buffer and associated data, 2 used by each endpoint
   (R/W separately) */
struct hostdrv_buffer {
	wait_queue_head_t userq; /* userspace IO wait queue */

	unsigned long dflags;    /* Atomic flags for USB part (see below) */

	/* DMA-ready buffers for USB */
	unsigned char *iobuf;
	dma_addr_t iobuf_dma;
	struct usb_ctrlrequest *cr;
	dma_addr_t cr_dma;

	/* USB transaction data */
	struct urb *current_urb;

	int usb_interval;  /* int/isoc transfer only */

	struct usb_endpoint_descriptor usb_ep;
	int usbpipe;
};

/* Dynamic flags for buffer; use with test_bit etc. These are indices. */
#define BUF_FL_URB_ACTIVE    0  /* URB in flight */
#define BUF_FL_DISCONNECTING 1  /* Processing disconnect */
#define BUF_FL_USER_BUSY     2  /* Userspace alive */

/* Private data for driver */
struct hostdrv_data {
	/* Char device bookkeeping */
	dev_t devno;
	struct cdev cdev;

	/* Track how many users we have, so we can decide if it's safe to
	   do the USB probe */
	int open_count;

	/* Driver-wide flag: the USB connection has died on us, or
	   the endpoint configuration has changed. Either way, userspace
	   needs to close all devices and re-open + re-query the config. */
	unsigned dirty:1;

	/* see above */
	struct hostdrv_buffer buf[N_HOSTDRV_DEVICES * 2];

	/* Current interface configuration, exportable via ioctl */
	struct hostdrv_current_config config_state;

	struct usb_device *udev;    /* the usb device for this device */
	struct usb_interface *uintf;  /* Currently connected usb interface */
};

static struct hostdrv_data *private;

/* Per-endpoint buffers: i * 2 == read, i * 2 + 1 == write */
#define get_read_buffer_of_ep(ep) (&private->buf[(ep) << 1])
#define get_write_buffer_of_ep(ep) (&private->buf[((ep) << 1) + 1])


/* We'll lock the whole driver only when we do housekeeping. */
DEFINE_MUTEX(hostdrv_lock);

/*
 * USB transfer handling
 */
#if 0
static void debug_dump_urb(struct urb *urb)
{
	BHDEBUG("-- urb:\n");
	if (!urb) {
		BHDEBUG("\t(null)\n");
		return;
	}
	BHDEBUG("\tpipe=0x%x transfer_flags=0x%x transfer_buffer=%p\n",
		urb->pipe, urb->transfer_flags, urb->transfer_buffer);
	BHDEBUG("\ttransfer_buffer_length=%u actual_length=%u status=%d\n",
		urb->transfer_buffer_length, urb->actual_length, urb->status);
}
#endif
/* Called from USB core when our URB finishes */
static void usb_generic_completion(struct urb *urb)
{
	complete(urb->context);
}

/* Called from USB core when our Isochronous URB finishes. This is separated
   to handle resubmitting. */
static void usb_isoc_completion(struct urb *urb)
{
	complete(urb->context);
}

/* Common USB communication for single-urb transfers. Usage: prepare data in
   buf->iobuf, size == count; set buf->pipe properly to read/write,
   and copy the data off buf->iobuf afterwards if reading. If this seems
   familiar, see transport.c for host mass storage driver. */
static int usb_perform_transfer(struct hostdrv_buffer *buf, int timeout)
{
	struct completion urb_done;
	int status;
	long timeleft;

	init_completion(&urb_done);

	buf->current_urb->context = &urb_done;
	buf->current_urb->actual_length = 0;
	buf->current_urb->error_count = 0;
	buf->current_urb->status = 0;

	/* see drivers/usb/storage/transport.c */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	buf->current_urb->transfer_flags = 0;
	buf->current_urb->setup_dma = 0;
#else
	buf->current_urb->transfer_flags = URB_NO_SETUP_DMA_MAP;
	buf->current_urb->setup_dma = buf->cr_dma;
#endif
	if (buf->current_urb->transfer_buffer == buf->iobuf)
		buf->current_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	buf->current_urb->transfer_dma = buf->iobuf_dma;

	status = usb_submit_urb(buf->current_urb, GFP_ATOMIC);
	if (status)
		return status;

	set_bit(BUF_FL_URB_ACTIVE, &buf->dflags);

	/* We may be racing a connection failure */
	if (test_bit(BUF_FL_DISCONNECTING, &buf->dflags)
		&& test_and_clear_bit(BUF_FL_URB_ACTIVE, &buf->dflags)) {
		mutex_lock(&hostdrv_lock);
		if (buf->current_urb) {
			BHDEBUG("Cancelling URB due to connection loss\n");
			usb_unlink_urb(buf->current_urb);
		}
		mutex_unlock(&hostdrv_lock);
	}

	/* This returns immediately with time left if the URB got cancelled */
	timeleft = wait_for_completion_interruptible_timeout(&urb_done,
			timeout ? : MAX_SCHEDULE_TIMEOUT);

	mutex_lock(&hostdrv_lock);
	clear_bit(BUF_FL_URB_ACTIVE, &buf->dflags);
	if (timeleft <= 0) {
		BHDEBUG("%s while waiting for URB, cancelling.\n",
			timeleft == 0 ? "Timeout" : "Signal");
		usb_kill_urb(buf->current_urb);
	}
	mutex_unlock(&hostdrv_lock);

	return buf->current_urb->status;
}

/* Mostly adapted from mass storage driver */
static int usb_check_transfer_result(struct hostdrv_buffer *buf,
	unsigned length, int result, unsigned partial)
{
	BHTRACE("URB status %d, %u/%u B done\n", result, partial, length);
	switch (result) {
	case 0:
		if (partial != length) {
			BHTRACE("\t->Short transfer\n");
			return 0;
		}
		BHTRACE("\t->Transfer complete\n");
		return 0;
	case -EPIPE:
		/* Control -> command fail; otherwise clear stall */
		BHDEBUG("Clearing halt for ep 0x%x\n",
			buf->usb_ep.bEndpointAddress);
		if (usb_clear_halt(private->udev, buf->usbpipe) < 0)
			return -EIO;

		return 0;

	/* babble - the device tried to send more than we wanted to read */
	case -EOVERFLOW:
		BHDEBUG("\t->Babble (device tried to send too much data)\n");
		return -EIO;

	/* the transfer was cancelled by abort, disconnect, or timeout */
	case -ECONNRESET:
		BHDEBUG("\t->Transfer cancelled\n");
		return -ECONNRESET;

	/* short scatter-gather read transfer */
	case -EREMOTEIO:
		BHDEBUG("\t->Short read transfer\n");
		return 0;

	/* abort or disconnect in progress */
	case -EIO:
		BHDEBUG("\t->Abort or disconnect in progress\n");
		return -EIO;

	/* the catch-all error case */
	default:
		BHDEBUG("\t->Unknown error (%d)\n", result);
		return -EIO;
	}
}

static void hostdrv_usb_fill_isoc_urb(struct urb *urb, struct usb_device *dev,
	unsigned pipe, void *transfer_buffer, int buffer_length,
	usb_complete_t complete_fn, void *context, int interval,
	struct usb_endpoint_descriptor *desc)
{
	unsigned maxpacket, i, packets;

	/* USB 2.0: bits 0-10 -> max size; 11-12 -> number of extra
	   transmissions in high speed mode for isoc ep */

	maxpacket = 0x7ff & le16_to_cpu(desc->wMaxPacketSize);
	maxpacket *= 1 + (0x3 & (le16_to_cpu(desc->wMaxPacketSize) >> 11));
	packets = DIV_ROUND_UP(buffer_length, maxpacket);

	urb->dev = dev;
	urb->pipe = pipe;
	urb->number_of_packets = packets;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
	urb->interval = interval;
	urb->transfer_flags = URB_ISO_ASAP;

	/* The frame descriptors need to be explicit for iso transfers */
	for (i = 0; i < packets; ++i) {
		urb->iso_frame_desc[i].length =
			min((unsigned) buffer_length, maxpacket);
		buffer_length -= urb->iso_frame_desc[i].length;
		urb->iso_frame_desc[i].offset = maxpacket * i;
	}
}

static int usb_do_transfer(struct hostdrv_buffer *buf, unsigned length,
	unsigned *actual_length)
{
	int ret;

	switch (usb_endpoint_type(&buf->usb_ep)) {
	case USB_ENDPOINT_XFER_CONTROL:
		BHERR("Control transfers not implemented\n");
		return -EIO;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		hostdrv_usb_fill_isoc_urb(buf->current_urb, private->udev,
				buf->usbpipe, buf->iobuf, length,
				usb_isoc_completion, NULL, buf->usb_interval,
				&buf->usb_ep);
		break;
	case USB_ENDPOINT_XFER_BULK:
		usb_fill_bulk_urb(buf->current_urb, private->udev, buf->usbpipe,
				buf->iobuf, length, usb_generic_completion,
				NULL);
		break;
	case USB_ENDPOINT_XFER_INT:
		usb_fill_int_urb(buf->current_urb, private->udev, buf->usbpipe,
				buf->iobuf, length, usb_generic_completion,
				NULL, buf->usb_interval);
		break;
	}

	ret = usb_perform_transfer(buf, 0);

	if (actual_length)
		*actual_length = buf->current_urb->actual_length;
	return usb_check_transfer_result(buf, length, ret,
		buf->current_urb->actual_length);
}

/*
 * Userspace interface
 */
loff_t hostdrv_llseek(struct file *filp, loff_t off, int whence)
{
	/* This is technically unnecessary for now, but we may use the position
	   for something later */
	return -EINVAL;
}

ssize_t hostdrv_read(struct file *filp, char __user *usr_buf,
	size_t count, loff_t *offp)
{
	struct hostdrv_buffer *buf;
	int retval = -EIO;
	unsigned cdev_minor, actual_count = 0;

	if (private->dirty)
		return -EBADF;

	cdev_minor = iminor(filp->f_dentry->d_inode);
	BHTRACE("start read for dev %d\n", cdev_minor);

	buf = get_read_buffer_of_ep(cdev_minor);
	if (!buf->usbpipe)
		return private->uintf ? -ENOENT : -ENODEV;

	if (mutex_lock_interruptible(&hostdrv_lock))
		return -ERESTARTSYS;

	while (test_bit(BUF_FL_USER_BUSY, &buf->dflags)) {
		BHTRACE("Driver busy, waiting...\n");
		mutex_unlock(&hostdrv_lock);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(buf->userq,
			!test_bit(BUF_FL_USER_BUSY, &buf->dflags)))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&hostdrv_lock))
			return -ERESTARTSYS;
	}
	set_bit(BUF_FL_USER_BUSY, &buf->dflags);

	if (count > private->config_state.transfer_size[cdev_minor])
		count = private->config_state.transfer_size[cdev_minor];

	mutex_unlock(&hostdrv_lock);

	retval = usb_do_transfer(buf, count, &actual_count);

	if (mutex_lock_interruptible(&hostdrv_lock)) {
		clear_bit(BUF_FL_USER_BUSY, &buf->dflags);
		return -ERESTARTSYS;
	}

	if (!retval) {
		retval = actual_count;
		if (copy_to_user(usr_buf, buf->iobuf, actual_count))
			retval = -EFAULT;
	}

	clear_bit(BUF_FL_USER_BUSY, &buf->dflags);
	mutex_unlock(&hostdrv_lock);

	return retval;
}

ssize_t hostdrv_write(struct file *filp, const char __user *usr_buf,
	size_t count, loff_t *offp)
{
	struct hostdrv_buffer *buf;
	int retval = -EIO;
	unsigned cdev_minor, actual_count = 0;

	if (private->dirty)
		return -EBADF;

	cdev_minor = iminor(filp->f_dentry->d_inode);
	BHTRACE("start write for dev %d\n", cdev_minor);

	buf = get_write_buffer_of_ep(cdev_minor);
	if (!buf->usbpipe)
		return private->uintf ? -ENOENT : -ENODEV;

	if (mutex_lock_interruptible(&hostdrv_lock))
		return -ERESTARTSYS;

	while (test_bit(BUF_FL_USER_BUSY, &buf->dflags)) {
		BHTRACE("Driver busy, waiting...\n");
		mutex_unlock(&hostdrv_lock);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(buf->userq,
			!test_bit(BUF_FL_USER_BUSY, &buf->dflags)))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&hostdrv_lock))
			return -ERESTARTSYS;
	}
	set_bit(BUF_FL_USER_BUSY, &buf->dflags);

	if (count > private->config_state.transfer_size[cdev_minor])
		count = private->config_state.transfer_size[cdev_minor];

	if (copy_from_user(buf->iobuf, usr_buf, count)) {
		retval = -EFAULT;
		goto done;
	}

	mutex_unlock(&hostdrv_lock);

	retval = usb_do_transfer(buf, count, &actual_count);

	if (mutex_lock_interruptible(&hostdrv_lock)) {
		clear_bit(BUF_FL_USER_BUSY, &buf->dflags);
		return -ERESTARTSYS;
	}

	if (!retval)
		retval = actual_count;

done:
	clear_bit(BUF_FL_USER_BUSY, &buf->dflags);
	mutex_unlock(&hostdrv_lock);

	return retval;
}

int hostdrv_open(struct inode *inode, struct file *filp)
{
	int ret = -ENOMEM;
	unsigned cminor, epstate;

	if (mutex_lock_interruptible(&hostdrv_lock))
		return -EBUSY;

	if (private->dirty) {
		ret = -EBADF;
		goto done;
	}

	cminor = iminor(filp->f_dentry->d_inode);
	epstate = private->config_state.endpoint_state[cminor];

	BHDEBUG("device %u opened (mode=%u) (epstate=%u)\n",
		cminor, filp->f_mode, epstate);
	private->open_count++;

	ret = 0;
done:
	mutex_unlock(&hostdrv_lock);
	return ret;
}

int hostdrv_release(struct inode *inode, struct file *filp)
{
	BUG_ON(mutex_lock_interruptible(&hostdrv_lock));

	if (--private->open_count == 0)
		private->dirty = 0;
	BHDEBUG("device released\n");
	mutex_unlock(&hostdrv_lock);
	return 0;
}

static void adjust_ep_max_transfer(unsigned endpoint, unsigned new_max_transfer)
{
	unsigned size_min, size_max;
	struct hostdrv_buffer *buf_in, *buf_out;

	if (!private->uintf)
		return;

	buf_in = get_read_buffer_of_ep(endpoint);
	buf_out = get_write_buffer_of_ep(endpoint);

	size_min = MIN_TRANSFER_BUFFER;
	size_max = USB_IOBUF_DMA_SIZE;

	/* Input pipe -> need to be prepared for anything from other end,
	   so check usb-provided max, too */
	if (usb_endpoint_dir_in(&buf_in->usb_ep))
		size_min = max((unsigned) buf_in->usb_ep.wMaxPacketSize,
			size_min);

	/* Only adjust if actually connected */
	if (usb_endpoint_dir_in(&buf_in->usb_ep)
		|| usb_endpoint_dir_out(&buf_out->usb_ep))
		private->config_state.transfer_size[endpoint] =
			clamp(new_max_transfer, size_min, size_max);
}

static long hostdrv_unlocked_ioctl(struct file *filp, unsigned cmd,
				   unsigned long arg)
{
	struct hostdrv_current_config tmp;
	int err = 0, i;

	if (_IOC_TYPE(cmd) != HOSTDRV_IOC_MAGIC) {
		err = -ENOTTY;
		goto done_not_locked;
	}

	if (_IOC_NR(cmd) > HOSTDRV_IOC_MAX) {
		err = -ENOTTY;
		goto done_not_locked;
	}

	if (err) {
		err = -EFAULT;
		goto done_not_locked;
	}

	if (mutex_lock_interruptible(&hostdrv_lock)) {
		err = -ERESTARTSYS;
		goto done_not_locked;
	}
	if (private->dirty) {
		err = -EBADF;
		goto done;
	}

	switch (cmd) {
	case HOSTDRV_IOCGCONF:
		if (copy_to_user((void __user *) arg,
			&private->config_state, _IOC_SIZE(cmd)))
			err = -EFAULT;
		break;
	case HOSTDRV_IOCSCONF:
		if (copy_from_user(&tmp, (void __user *) arg,
			_IOC_SIZE(cmd))) {
			err = -EFAULT;
			break;
		}

		for (i = 0; i < N_HOSTDRV_DEVICES; ++i)
			adjust_ep_max_transfer(i, tmp.transfer_size[i]);

		break;
	}
done:
	mutex_unlock(&hostdrv_lock);
done_not_locked:
	return err;
}

/*
 * USB interface
 */
static void usb_show_endpoint_descriptor(struct usb_endpoint_descriptor *desc)
{
	char *type[4] = { "control", "isochronous", "bulk", "interrupt" };
	BHDEBUG("-----------------------\n");
	BHDEBUG("Length             = %d\n", desc->bLength);
	BHDEBUG("DescriptorType     = 0x%x\n", desc->bDescriptorType);
	BHDEBUG("EndpointAddress    = 0x%x [%s]\n", desc->bEndpointAddress,
		(desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_CONTROL ? "i/o" :
		(desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ?
		"in" : "out");
	BHDEBUG("Attributes         = 0x%x [%s]\n", desc->bmAttributes,
		type[USB_ENDPOINT_XFERTYPE_MASK & desc->bmAttributes]);
	BHDEBUG("MaxPacketSize      = %u (bytes)\n", desc->wMaxPacketSize);
	BHDEBUG("Interval           = %u\n", desc->bInterval);
	BHDEBUG("-----------------------\n");
}

#define USB_DEFAULT_MINOR_BASE 192
const struct file_operations usb_fops = {
	.owner = THIS_MODULE,
};

static struct usb_class_driver blts_usb_class = {
	.name = "usb/blts_usb_host%d",
	.fops = &usb_fops,
	.minor_base = USB_DEFAULT_MINOR_BASE,
};

static void usb_buffer_release_dma_safe(struct hostdrv_buffer *buf)
{
	BUG_ON(!mutex_is_locked(&hostdrv_lock));
	if (!buf)
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	if (buf->iobuf && buf->iobuf_dma)
		usb_free_coherent(private->udev, USB_IOBUF_DMA_SIZE,
				buf->iobuf, buf->iobuf_dma);
	kfree(buf->cr);
#else
	if (buf->iobuf && buf->iobuf_dma)
		usb_buffer_free(private->udev, USB_IOBUF_DMA_SIZE,
				buf->iobuf, buf->iobuf_dma);
	if (buf->cr && buf->cr_dma)
		usb_buffer_free(private->udev, sizeof(*buf->cr),
				buf->cr, buf->cr_dma);
#endif
	buf->iobuf = NULL;
	buf->iobuf_dma = 0;
	buf->cr = NULL;
	buf->cr_dma = 0;
}

static int usb_buffer_setup_dma_safe(struct hostdrv_buffer *buf)
{
	BUG_ON(!mutex_is_locked(&hostdrv_lock));
	BUG_ON(!buf);

	if (buf->iobuf || buf->iobuf_dma || buf->cr || buf->cr_dma)
		usb_buffer_release_dma_safe(buf);

	/* 2.6.34 renames this and usb_buffer_free, presumably the compatibility
	   macros will eventually go away. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	buf->iobuf = usb_alloc_coherent(private->udev, USB_IOBUF_DMA_SIZE,
				GFP_ATOMIC, &buf->iobuf_dma);
#else
	buf->iobuf = usb_buffer_alloc(private->udev, USB_IOBUF_DMA_SIZE,
				GFP_ATOMIC, &buf->iobuf_dma);
#endif
	if (!buf->iobuf || !buf->iobuf_dma) {
		BHERR("IO buffer allocation failed\n");
		return -ENOMEM;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
	buf->cr = kmalloc(sizeof(*buf->cr), GFP_ATOMIC);
	buf->cr_dma = -1;
#else
	buf->cr = usb_buffer_alloc(private->udev, sizeof(*buf->cr),
				GFP_ATOMIC, &buf->cr_dma);
#endif
	if (!buf->cr || !buf->cr_dma) {
		BHERR("Control buffer allocation failed\n");
		return -ENOMEM;
	}

	BHDEBUG("Allocated %u B + %lu B USB DMA buffers\n",
		USB_IOBUF_DMA_SIZE, (unsigned long) sizeof(*buf->cr));
	return 0;
}

static int usb_buffer_setup_urb(struct hostdrv_buffer *buf)
{
	unsigned packets = 0;

	if (!buf->usbpipe)
		return -EINVAL;

	/* If this triggers, we forgot a cleanup after a disconnect. */
	BUG_ON(buf->current_urb);

	/* Isoc endpoints need a bit extra. */
	if (usb_endpoint_xfer_isoc(&buf->usb_ep))
		packets = DIV_ROUND_UP(USB_IOBUF_DMA_SIZE, MIN_TRANSFER_BUFFER);

	buf->current_urb = usb_alloc_urb(packets, GFP_ATOMIC);

	BHDEBUG("URB size = %lu B\n", (unsigned long)(sizeof(struct urb)
			+ packets * sizeof(struct usb_iso_packet_descriptor)));
	if (!buf->current_urb)
		return -ENOMEM;
	return 0;
}


static void usb_stop_all_active(void)
{
	int i, ret;
	struct hostdrv_buffer *buf;
	wait_queue_head_t user_io_stop_q;

	init_waitqueue_head(&user_io_stop_q);

	mutex_lock(&hostdrv_lock);

	for (i = 0; i < N_HOSTDRV_DEVICES * 2; ++i) {
		buf = &private->buf[i];
		if (test_and_clear_bit(BUF_FL_URB_ACTIVE, &buf->dflags)) {
			BHDEBUG("\t->Cancelling active URB\n");
			if (buf->current_urb)
				usb_kill_urb(buf->current_urb);
		}
		while (test_bit(BUF_FL_USER_BUSY, &buf->dflags)) {
			mutex_unlock(&hostdrv_lock);
			ret = wait_event_timeout(user_io_stop_q,
				!test_bit(BUF_FL_USER_BUSY, &buf->dflags),
				msecs_to_jiffies(2000));
			if (!ret) {
				BHERR("Timed out waiting for USB completion\n");
				/* TODO: possibly forcibly shut down userspace
				   IO here. This should happen only when the
				   HCD is actually broken, though. */
				mutex_lock(&hostdrv_lock);
				break;
			}
			mutex_lock(&hostdrv_lock);
		}
	}
	mutex_unlock(&hostdrv_lock);
}

/* Stop active transfers and release all related resources. Might sleep. */
static void usb_buffer_teardown(void)
{
	int i;
	struct hostdrv_buffer *buf;

	usb_stop_all_active();

	mutex_lock(&hostdrv_lock);
	BHDEBUG("Releasing USB state\n");
	for (i = 0; i < N_HOSTDRV_DEVICES * 2; ++i) {
		buf = &private->buf[i];

		BUG_ON(test_bit(BUF_FL_URB_ACTIVE, &buf->dflags));

		usb_buffer_release_dma_safe(buf);
		usb_free_urb(buf->current_urb);
		buf->current_urb = NULL;
		memset(&buf->usb_ep, 0, USB_DT_ENDPOINT_SIZE);
		buf->usbpipe = 0;
		clear_bit(BUF_FL_DISCONNECTING, &buf->dflags);
	}
	mutex_unlock(&hostdrv_lock);
}

/* Sets up our bookkeeping for an individual endpoint. Allocates buffers,
   updates config struct. Call this with hostdrv_lock held. */
/* NOTE: USB 3 has mandatory values for some of the endpoint
   settings. Check those here when support is added. */
static int usb_probe_new_ep(struct usb_device *dev,
	struct usb_endpoint_descriptor *endpoint, int index)
{
	struct hostdrv_buffer *buf_in, *buf_out, *work;
	struct hostdrv_current_config *config = &private->config_state;
	int pipe, ret;

	buf_in = get_read_buffer_of_ep(index);
	buf_out = get_write_buffer_of_ep(index);

	memcpy(&buf_in->usb_ep, endpoint, USB_DT_ENDPOINT_SIZE);
	memcpy(&buf_out->usb_ep, endpoint, USB_DT_ENDPOINT_SIZE);

	if (endpoint->bEndpointAddress & USB_DIR_IN)
		config->endpoint_state[index] |= USBDRV_EP_IN;
	else
		config->endpoint_state[index] |= USBDRV_EP_OUT;

	config->transfer_size[index] = endpoint->wMaxPacketSize;

	/* FIXME: most of this can be combined --> */
	switch (endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_CONTROL:
		config->endpoint_type[index] = USBDRV_EP_TYPE_CONTROL;
		config->endpoint_state[index] = USBDRV_EP_IN | USBDRV_EP_OUT;
		break;
	case USB_ENDPOINT_XFER_ISOC:
		config->endpoint_type[index] = USBDRV_EP_TYPE_ISOC;
		if (endpoint->bEndpointAddress & USB_DIR_IN) {
			work = buf_in;
			pipe = usb_rcvisocpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
		} else {
			pipe = usb_sndisocpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
			work = buf_out;
		}
		work->usbpipe = pipe;
		work->usb_interval = endpoint->bInterval;

		ret = usb_buffer_setup_dma_safe(work);
		if (ret) {
			BHERR("DMA setup failed\n");
			return -ENOMEM;
		}

		ret = usb_buffer_setup_urb(work);
		if (ret) {
			BHERR("URB allocation failed\n");
			return ret;
		}
		break;
	case USB_ENDPOINT_XFER_BULK:
		config->endpoint_type[index] = USBDRV_EP_TYPE_BULK;
		if (endpoint->bEndpointAddress & USB_DIR_IN) {
			work = buf_in;
			pipe = usb_rcvbulkpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
		} else {
			pipe = usb_sndbulkpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
			work = buf_out;
		}
		work->usbpipe = pipe;

		ret = usb_buffer_setup_dma_safe(work);
		if (ret) {
			BHERR("DMA setup failed\n");
			return -ENOMEM;
		}

		ret = usb_buffer_setup_urb(work);
		if (ret) {
			BHERR("URB allocation failed\n");
			return ret;
		}
		break;
	case USB_ENDPOINT_XFER_INT:
		config->endpoint_type[index] = USBDRV_EP_TYPE_INT;
		if (endpoint->bEndpointAddress & USB_DIR_IN) {
			work = buf_in;
			pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
		} else {
			pipe = usb_sndintpipe(dev, endpoint->bEndpointAddress
					& USB_ENDPOINT_NUMBER_MASK);
			work = buf_out;
		}
		work->usbpipe = pipe;
		work->usb_interval = endpoint->bInterval;

		ret = usb_buffer_setup_dma_safe(work);
		if (ret) {
			BHERR("DMA setup failed\n");
			return -ENOMEM;
		}

		ret = usb_buffer_setup_urb(work);
		if (ret) {
			BHERR("URB allocation failed\n");
			return ret;
		}
		break;
	}
	return 0;
}


static int hostdrv_probe(struct usb_interface *intf,
			const struct usb_device_id *id)
{
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	struct hostdrv_current_config *config;
	int i, retval = -ENOMEM, last_ep;

	if (mutex_lock_interruptible(&hostdrv_lock)) {
		BHERR("Driver busy\n");
		return -EIO;
	}

	private->udev = usb_get_dev(interface_to_usbdev(intf));

	config = &private->config_state;

	/* set up the endpoint information */

	/* handle endpoint 0 */
	endpoint = &private->udev->ep0.desc;
	usb_show_endpoint_descriptor(endpoint);
	retval = usb_probe_new_ep(private->udev, endpoint, 0);
	if (retval)
		goto error;

	iface_desc = intf->cur_altsetting;
	last_ep = min((unsigned)iface_desc->desc.bNumEndpoints,
		(unsigned)(N_HOSTDRV_DEVICES-1));
	for (i = 1; i < last_ep + 1; ++i) {
		endpoint = &iface_desc->endpoint[i-1].desc;
		usb_show_endpoint_descriptor(endpoint);
		retval = usb_probe_new_ep(private->udev, endpoint, i);
		if (retval)
			goto error;
	}

	usb_set_intfdata(intf, private);

	/* The device is now ready for registration */
	retval = usb_register_dev(intf, &blts_usb_class);
	if (retval) {
		BHERR("Not able to get a minor for this device.\n");
		usb_set_intfdata(intf, NULL);
		goto error;
	}

	private->uintf = intf;
	/* This is misleading, since we expect userspace to only touch the
	   character device API. */
	/* BHDEBUG("USB device now attached to minor %d\n", intf->minor); */

error:
	mutex_unlock(&hostdrv_lock);
	return retval;
}

static void hostdrv_common_start_teardown(void)
{
	int i;
	struct hostdrv_buffer *buf;

	mutex_lock(&hostdrv_lock);
	for (i = 0; i < N_HOSTDRV_DEVICES * 2; ++i) {
		buf = &private->buf[i];
		set_bit(BUF_FL_DISCONNECTING, &buf->dflags);
	}

	if (private->open_count)
		private->dirty = 1;

	for (i = 0; i < N_HOSTDRV_DEVICES; ++i) {
		private->config_state.endpoint_type[i] = 0;
		private->config_state.endpoint_state[i] = 0;
		private->config_state.transfer_size[i] = 0;
	}
	mutex_unlock(&hostdrv_lock);

	usb_buffer_teardown();
}

static void hostdrv_common_finish_teardown(void)
{
	int i;

	BUG_ON(!mutex_is_locked(&hostdrv_lock));
	for (i = 0; i < N_HOSTDRV_DEVICES * 2; ++i)
		clear_bit(BUF_FL_DISCONNECTING, &private->buf[i].dflags);
}

static void hostdrv_disconnect(struct usb_interface *intf)
{
	hostdrv_common_start_teardown();

	mutex_lock(&hostdrv_lock);
	if (intf) {
		usb_deregister_dev(intf, &blts_usb_class);
		private->udev = NULL;
		private->uintf = NULL;
	}

	hostdrv_common_finish_teardown();
	mutex_unlock(&hostdrv_lock);
}

static int hostdrv_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	BHDEBUG("USB suspend\n");

	usb_stop_all_active();

	return 0;
}

static int hostdrv_usb_resume(struct usb_interface *intf)
{
	BHDEBUG("USB resume\n");
	/* We'll end up in .disconnect if the connection broke during suspend */

	return 0;
}

/*
 * Initialisation and cleanup
 */
const struct file_operations hostdrv_fops = {
	.owner = THIS_MODULE,
	.read = hostdrv_read,
	.write = hostdrv_write,
	.open = hostdrv_open,
	.release = hostdrv_release,
	.unlocked_ioctl = hostdrv_unlocked_ioctl,
	.llseek = hostdrv_llseek,
};

static int __init hostdrv_chardev_init(struct hostdrv_data *data)
{
	int err;
	err = alloc_chrdev_region(&data->devno, 0,
		N_HOSTDRV_DEVICES, "blts_usb_host");

	if (err) {
		BHERR("cannot allocate major number\n");
		return -1;
	}

	cdev_init(&data->cdev, &hostdrv_fops);
	err = cdev_add(&data->cdev, data->devno, N_HOSTDRV_DEVICES);

	if (err) {
		BHERR("cannot add device\n");
		unregister_chrdev_region(data->devno, N_HOSTDRV_DEVICES);
	}

	return err;
}

static int __init hostdrv_buf_init(struct hostdrv_data *data)
{
	int i;

	for (i = 0; i < N_HOSTDRV_DEVICES * 2; ++i)
		init_waitqueue_head(&data->buf[i].userq);

	return 0;
}

static void hostdrv_chardev_exit(struct hostdrv_data *data)
{
	if (!data)
		return;

	mutex_lock(&hostdrv_lock);
	if (data->devno) {
		cdev_del(&data->cdev);
		unregister_chrdev_region(data->devno, N_HOSTDRV_DEVICES);
		data->devno = 0;
	}
	mutex_unlock(&hostdrv_lock);
}

static void hostdrv_buf_exit(struct hostdrv_data *data)
{
	if (!data)
		return;
}

static struct usb_driver blts_usb_driver = {
	.name =		"blts_usb_host",
	.id_table =	hstdrv_id_table,
	.probe =	hostdrv_probe,
	.disconnect =	hostdrv_disconnect,
	.suspend =	hostdrv_usb_suspend,
	.resume =	hostdrv_usb_resume,
};

static int __init hostdrv_init(void)
{
	int err;
	struct hostdrv_data *data;

	err = sysfs_log_add();
	if (err)
		return err;

	BHDEBUG("Init\n");

	data = kzalloc(sizeof *data, GFP_KERNEL);
	if (!data) {
		BHERR("OOM while initializing.\n");
		return -1;
	}

	private = data;

	mutex_init(&hostdrv_lock);

	err = hostdrv_buf_init(data);
	if (err)
		goto error;

	err = hostdrv_chardev_init(data);
	if (err)
		goto error;

	err = usb_register(&blts_usb_driver);
	if (err)
		goto error;

	BHDEBUG("Registered major %u, minors %u-%u\n",
		MAJOR(data->devno), MINOR(data->devno),
		MINOR(data->devno) + N_HOSTDRV_DEVICES - 1);

	return 0;

error:
	if (data) {
		hostdrv_buf_exit(data);
		hostdrv_chardev_exit(data);
		kfree(data);
	}

	return err;
}

static void __exit hostdrv_exit(void)
{
	struct hostdrv_data *data = private;

	if (data) {
		usb_deregister(&blts_usb_driver);
		hostdrv_buf_exit(data);
		hostdrv_chardev_exit(data);
		kfree(data);
	}
	BHDEBUG("Exit.\n");
	sysfs_log_remove();
}

module_init(hostdrv_init);
module_exit(hostdrv_exit);

