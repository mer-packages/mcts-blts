/* alsa_pcm.h -- ALSA PCM IF functionality

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

#ifndef ALSA_PCM_H
#define ALSA_PCM_H

#include "alsa_util.h"

pcm_device* pcm_open(int card, int device, int dir);
int pcm_close(pcm_device* hw);
int pcm_play_sine(pcm_device *hw);
int pcm_record(pcm_device *hw);
int pcm_init(pcm_device* hw);
int pcm_print_info(pcm_device* hw);

int pcm_link_device(pcm_device *hw);
int pcm_unlink_device(pcm_device *hw);

void pcm_stored_devices_init();
void pcm_device_fd_store(int card, int device, int fd);
int pcm_get_device_fd(int card, int device, int timeout);
void pcm_stored_devices_free();

void pcm_thread_init(unsigned n_pcm_threads);

#endif /* ALSA_PCM_H */

