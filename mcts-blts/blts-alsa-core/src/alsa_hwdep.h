/* alsa_hwdep.h -- ALSA hwdep IF functionality

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


#ifndef ALSA_HWDEP_H
#define ALSA_HWDEP_H

#include "alsa_util.h"

hwdep_device* hwdep_open(int card, int device);
int hwdep_close(hwdep_device* hw);
int hwdep_print_info(hwdep_device* hw);

#endif /* ALSA_HWDEP_H */

