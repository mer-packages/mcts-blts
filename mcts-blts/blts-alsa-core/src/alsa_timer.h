/* alsa_timer.h -- ALSA timer IF functionality

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


#ifndef ALSA_TIMER_H
#define ALSA_TIMER_H

#include "alsa_util.h"

timer_device* timer_open();
int timer_close(timer_device* hw);
int timer_print_info(timer_device* hw);
int timer_select_for_card(timer_device *hw, int card);
int timer_select_for_device(timer_device *hw, int card, int device);
int timer_set_period(timer_device *hw, unsigned long period_ns);
int timer_read(timer_device *hw, unsigned *resolution, unsigned *ticks);
int timer_get_resolution_ns(timer_device *hw, unsigned long *resolution);
int timer_print_status(timer_device* hw);
int timer_print_status_trace(timer_device *hw);
#endif /* ALSA_TIMER_H */

