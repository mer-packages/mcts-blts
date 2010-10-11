/*
 * hostdrv_common.h -- BLTS USB host-side passthrough driver shared definitions
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

#ifndef HOSTDRV_COMMON_H
#define HOSTDRV_COMMON_H

#include <linux/ioctl.h>
#include "usbdrv_common.h"

#define N_HOSTDRV_DEVICES 16

/* Result from get current config ioctl, and argument to set config ioctl. */
struct hostdrv_current_config {
	enum usbdrv_endpoint_type endpoint_type[N_HOSTDRV_DEVICES];

	/* endpoint state, bitwise or of flags */
	unsigned endpoint_state[N_HOSTDRV_DEVICES];

	/* sizes of usb transfer buffers (configurable in test) */
	unsigned transfer_size[N_HOSTDRV_DEVICES];
};

/* ioctls */
#define HOSTDRV_IOC_MAGIC 0xB2

#define HOSTDRV_IOCGCONF _IOR(HOSTDRV_IOC_MAGIC, 0, struct hostdrv_current_config)
#define HOSTDRV_IOCSCONF _IOW(HOSTDRV_IOC_MAGIC, 1, struct hostdrv_current_config)

#define HOSTDRV_IOC_MAX 1

#endif
