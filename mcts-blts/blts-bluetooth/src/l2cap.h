/* l2cap.h -- L2CAP specific functionality

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

#ifndef L2CAP_H
#define L2CAP_H

int l2cap_echo_server_oneshot(struct bt_ctx *ctx);
int l2cap_echo_test_client(struct bt_ctx *ctx, int want_transfer_test);

#endif /* L2CAP_H */
