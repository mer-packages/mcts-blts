/* wlan-core-ioctl.h -- IF for wlan core ioctls

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef WLAN_IOCTL_H
#define WLAN_IOCTL_H

#include "wlan-core-definitions.h"

#include <linux/version.h>
#include <linux/wireless.h>

/* Backward compatibility for old kernels.*/
//#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,28)
//#warning " *** Compiling with ancient kernel headers. Expect breakage."
//#endif

int IOCTL_CALL(wlan_core_data* data, int request, struct iwreq * pwrq);

#endif  /* WLAN_IOCTL_H */
