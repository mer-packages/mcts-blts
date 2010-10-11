/*
 * usbdrv_common.h -- USB related types and defines
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

#ifndef USBDRV_COMMON_H
#define USBDRV_COMMON_H

#define USBDRV_MAX_ENDPOINTS 16

enum usbdrv_endpoint_type {
	USBDRV_EP_TYPE_NONE = 0,
	USBDRV_EP_TYPE_CONTROL,
	USBDRV_EP_TYPE_BULK,
	USBDRV_EP_TYPE_ISOC,
	USBDRV_EP_TYPE_INT
};

enum usbdrv_endpoint_direction_flags {
	USBDRV_EP_IN = 1,
	USBDRV_EP_OUT = 2,
};

enum usbdrv_device_speed {
	USBDRV_SPEED_UNKNOWN = 0,
	USBDRV_SPEED_LOW,
	USBDRV_SPEED_FULL,
	USBDRV_SPEED_HIGH,
};

/* BLTS Gadget IDs */
#define USB_BLTS_VENDOR_ID	0x0421 /* Nokia */
#define USB_BLTS_PRODUCT_ID	0xbeef

#endif /* USBDRV_COMMON_H */

