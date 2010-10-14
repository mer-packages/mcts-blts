/* info.h -- HCI information exchange related functions

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

#ifndef INFO_H
#define INFO_H

#include "hci.h"
#include "bt_ctx.h"

/* -------------------------------------------------------------------------- */
/* Helper functions */

void dump_ctrl_information_data(device_info_t* devptr, int remote);
void dump_link_information_data(link_info_t* linkptr, int remote);

int compare_device_info_data(device_info_t* dev1, device_info_t* dev2);
int compare_link_info_data(link_info_t* dev1, link_info_t* dev2);

int do_accept(struct bt_ctx *ctx, int fd);
int do_listen(struct bt_ctx *ctx, int port);

void *read_link_info_thread(void *dataptr);
void *send_link_info_thread(void *dataptr);

int send_link_info(void *dataptr);
/* -------------------------------------------------------------------------- */
/* Test code */
int read_and_verify_loc_ctrl_info(struct bt_ctx *ctx);
int collect_and_send_rem_ctrl_info(struct bt_ctx *ctx);

int read_and_verify_link_info(struct bt_ctx *ctx);
int collect_and_send_link_info(struct bt_ctx *ctx);
int collect_and_send_link_info_one_by_one(struct bt_ctx *ctx); /* non-thread version from above */

#endif /* INFO_H */
