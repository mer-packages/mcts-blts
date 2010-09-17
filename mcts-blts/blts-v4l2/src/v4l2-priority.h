/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* v4l2-priority.h -- IF for v4l2 core priority ioctl tests

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

#ifndef V4L2_PRIORITY_H
#define V4L2_PRIORITY_H

#include "blts-v4l2-definitions.h"

/* Base priority case */
int
v4l2_case_priority (v4l2_data *user_data, int test_num);

#endif /* V4L2_PRIORITY_H */
