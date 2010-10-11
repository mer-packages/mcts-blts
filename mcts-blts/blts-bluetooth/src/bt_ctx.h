/* bt_ctx.h -- Header for test information data structure handling

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

#ifndef BT_CTX_H
#define BT_CTX_H

#include <time.h>
#include <bluetooth/bluetooth.h>

/**
 * Everything Bt-specific we need to know about the test progress.
 */
struct bt_ctx {
	bdaddr_t local_mac;   /**< our MAC */
	bdaddr_t remote_mac;  /**< other end MAC */

	int dev_id;            /**< local bt device id */

	int local_port;        /**< port we use */
	int remote_port;       /**< port other end uses */

	int hci_fd;            /**< opened HCI device fd */
	uint16_t conn_handle;  /**< handle of established connection (HCI tests only) */

	struct timespec test_timeout;     /**< timeout for test case */
};



struct bt_ctx* bt_ctx_new();
void bt_ctx_free(struct bt_ctx* ctx);

#endif /* BT_CTX_H */
