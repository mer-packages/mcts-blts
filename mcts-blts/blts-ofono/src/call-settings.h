/* call-settings.h -- Call settings test cases

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

#ifndef CALL_SETTINGS_H
#define CALL_SETTINGS_H

#include <blts_params.h>

int blts_ofono_settings_waiting(void*, int);

void *call_settings_variant_read_arg_processor(struct boxed_value *, void *);

#endif /* CALL_SETTINGS_H */
