/* wlan-core-profiler.h -- Functionality to profile wlan ioctl calls

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


#ifndef WLAN_PROFILER_H
#define WLAN_PROFILER_H

void ioctl_start_profiling();
void ioctl_stop_profiling();
void ioctl_print_profiling_data();
void ioctl_profile(const char* name);

#endif /* WLAN_PROFILER_H */


