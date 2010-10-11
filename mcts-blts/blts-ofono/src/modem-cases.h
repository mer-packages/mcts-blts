/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* modem-cases.h -- Modem cases for blts-ofono

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

#ifndef MODEM_CASES_H
#define MODEM_CASES_H

/* Sets all modems online */
int
blts_ofono_case_modems_online (void *user_data, int testnum);

/* Sets all modems offline */
int
blts_ofono_case_modems_offline (void *user_data, int testnum);

#endif /* MODEM_CASES_H */
