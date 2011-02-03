/* radio-cases.h -- Radio mode settings

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
#ifndef RADIO_MODE_CASES_H
#define RADIO_MODE_CASES_H

#include <blts_params.h>

int my_ofono_chage_radio_technology(void*, int);

void *radio_variant_set_arg_processor(struct boxed_value *, void *);


#endif /* RADIO_MODE_CASES_H */
