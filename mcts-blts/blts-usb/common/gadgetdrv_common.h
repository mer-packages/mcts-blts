/*
 * gadgetdrv_common.h -- BLTS USB peripheral-side passthrough driver shared
 * definitions
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

/* This is intended to be shared by the userspace test controller. */

#ifndef GADGETDRV_COMMON_H
#define GADGETDRV_COMMON_H

#include <linux/ioctl.h>
#include "usbdrv_common.h"

#define N_GADGETDRV_DEVICES USBDRV_MAX_ENDPOINTS

/* TODO: support for configuration->interface->endpoint  */
struct gadgetdrv_endpoint_config {
	enum usbdrv_endpoint_type type;
	enum usbdrv_endpoint_direction_flags direction;
	unsigned max_packet_size;
	unsigned interval;
	unsigned transfer_size;
};

/** Result from get current config ioctl, and argument to set config ioctl.
 * @max_power: Maximum power consumption of the USB device from the bus
 *  in mA.
 * @speed: Highest speed the driver/device handles.
 * @used_speed: Used speed;
 *  USBDRV_SPEED_UNKNOWN: Support any speed.
 *  USBDRV_SPEED_FULL: Full speed only.
 *  USBDRV_SPEED_HIGH: High speed only.
 * @endpoints: Array of endpoint descriptors.
 */
struct gadgetdrv_current_config {
	int max_power;
	enum usbdrv_device_speed speed;
	enum usbdrv_device_speed used_speed;
	struct gadgetdrv_endpoint_config endpoints[N_GADGETDRV_DEVICES];
};

/* ioctls */
#define GADGETDRV_IOC_MAGIC 0xB2

#define GADGETDRV_IOCGCONF _IOR(GADGETDRV_IOC_MAGIC, 0, struct gadgetdrv_current_config)
#define GADGETDRV_IOCSCONF _IOW(GADGETDRV_IOC_MAGIC, 1, struct gadgetdrv_current_config)
#define GADGETDRV_IOCSTART _IO(GADGETDRV_IOC_MAGIC, 2)
#define GADGETDRV_IOCSTOP  _IO(GADGETDRV_IOC_MAGIC, 3)

#define GADGETDRV_IOC_MAX 3

#endif /* GADGETDRV_COMMON_H */

