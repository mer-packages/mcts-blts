/* blts_watchdog_util.h -- Various helper functions

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

#ifndef BLTS_WATCHDOG_UTIL_H
#define BLTS_WATCHDOG_UTIL_H

#include <blts_log.h>
#include <blts_timing.h>

int wdt_dep_check();
int wdt_open_close();
int wdt_send_keepalive(double execution_time);

#endif // BLTS_WATCHDOG_UTIL_H

