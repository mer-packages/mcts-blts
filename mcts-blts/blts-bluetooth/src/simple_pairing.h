/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* simple_pairing.c -- Bluetooth simple pairing tests

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

#ifndef SIMPLE_PAIRING_H_
#define SIMPLE_PAIRING_H_

#include "bt_fute.h"

/* Run simple pairing test, "in band", as in l2cap socket */
int
blts_simple_pairing_run (bt_data *user_data, int master);

/* OOB initiated simple pairing */
int
blts_simple_pairing_oob_run (bt_data *user_data, int master);

/* L2CAP "secure" server for simple pairing cases */
int
blts_simple_pairing_l2cap_server (bt_data *user_data);

#endif /* SIMPLE_PAIRING_H_ */
