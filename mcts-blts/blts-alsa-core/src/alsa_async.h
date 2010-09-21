/* alsa_async.h -- Asynchronous IO handling

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

#ifndef BLTS_ALSA_ASYNC_H
#define BLTS_ALSA_ASYNC_H

#include "alsa_util.h"

typedef void (*async_callback)(pcm_device *hw, void *data);

int async_handler_add(async_callback callback, pcm_device *hw, int trigger_fd, void *data);
int async_handler_del(async_callback callback, pcm_device *hw, int trigger_fd, void *data);
int async_handler_clear();

int async_io_enable(int fd);
int async_io_disable(int fd);

#endif /* BLTS_ALSA_ASYNC_H */
