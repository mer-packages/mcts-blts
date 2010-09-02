/*
 * ep_config.h -- BLTS USB peripheral-side passthrough driver endpoint
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


#ifndef EP_CONFIG_H
#define EP_CONFIG_H

#include "gadgetdrv.h"

int register_eps(struct usb_composite_dev *cdev,
	struct gadgetdrv_data *data);

#endif /* EP_CONFIG_H */

