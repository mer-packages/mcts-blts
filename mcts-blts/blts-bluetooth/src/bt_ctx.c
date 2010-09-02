/* bt_ctx.c -- Test information data structure handling

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#include "bt_ctx.h"

/** Return empty bt_ctx struct. Free with bt_ctx_free */
struct bt_ctx* bt_ctx_new()
{
	struct bt_ctx* ctx = malloc(sizeof *ctx);
	if(!ctx) return 0;

	/* set safe defaults */
	ctx->local_mac = *BDADDR_ANY;
	ctx->remote_mac = *BDADDR_ANY;
	ctx->dev_id = -1;
	ctx->local_port = 0;
	ctx->remote_port = 0;
	ctx->hci_fd = -1;
	ctx->conn_handle = 0xffff;

	ctx->test_timeout.tv_sec = 30;
	ctx->test_timeout.tv_nsec = 0;

	return ctx;
}

/** Free bt_ctx struct. */
void bt_ctx_free(struct bt_ctx* ctx)
{
	if(!ctx) return;

	free(ctx);
}

