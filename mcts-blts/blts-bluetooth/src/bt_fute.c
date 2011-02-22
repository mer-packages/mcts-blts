/* bt_fute.c -- Bluez functional tests

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

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <blts_log.h>
#include <blts_dep_check.h>

#include "bt_fute.h"
#include "hci.h"
#include "l2cap.h"
#include "rfcomm.h"
#include "info.h"
#include "pairing.h"

/** rules file for depcheck() */
#define DEPCHECK_RULES "/usr/lib/tests/blts-bluetooth-tests/blts-bluetooth-req_files.cfg"

/* -------------------------------------------------------------------------- */
/* Test cases -> */
/* -------------------------------------------------------------------------- */


/* Drivers etc check */
int fute_bt_drivers_depcheck()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = depcheck(DEPCHECK_RULES,1);
	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

/* Simple BT scan */
int fute_bt_scan()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		retval = do_scan(ctx,0,0,0);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}

/* Echo server */
int fute_bt_l2cap_echo_server()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		ctx->local_port = 0x1234;
		retval = l2cap_echo_server_oneshot(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}

/* Echo test client */
int fute_bt_l2cap_echo_client(char *server_mac, int want_transfer_test)
{
	if(!server_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		str2ba(server_mac, &ctx->remote_mac);
		ctx->remote_port = 0x1234;
		retval = l2cap_echo_test_client(ctx, want_transfer_test);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}


/* Echo server (rfcomm) */
int fute_bt_rfcomm_echo_server()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		ctx->local_port = 0x11;
		retval = rfcomm_echo_server_oneshot(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}

/* Echo test client (rfcomm) */
int fute_bt_rfcomm_echo_client(char *server_mac, int want_transfer_test)
{
	if(!server_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		str2ba(server_mac, &ctx->remote_mac);
		ctx->remote_port = 0x11;
		retval = rfcomm_echo_test_client(ctx, want_transfer_test);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}

/* Use HCI to connect & disconnect only the link */
int fute_bt_hci_connect_disconnect(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = hci_connect_remote(ctx);
		sleep(WAIT_TIME_CONNECT_DISCONNECT);
		retval |= hci_disconnect_remote(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

/* Audit incoming HCI connection sequence */
int fute_bt_hci_audit_incoming_connect()
{
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		ctx->test_timeout.tv_sec = 10;
		retval = hci_audit_incoming_connect(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

/* Transfer ACL test data to Receive ACL test data test case*/
int fute_bt_hci_transfer_acl_data(char *remote_mac)
{

	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = hci_transfer_acl_data(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;

}

/* Receive ACL test data when Transfer ACL data test case sends it*/
int fute_bt_hci_receive_acl_data()
{
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		retval = hci_receive_acl_data(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

int fute_bt_hci_change_name()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		retval = do_change_name(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}
int fute_bt_hci_verify_name(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = hci_verify_name_change(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}
int fute_bt_hci_change_class()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		retval = do_change_class(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}
int fute_bt_hci_verify_class(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = hci_verify_class_change(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;

}


/* Use HCI to connect, reset connection & connect again */
int fute_bt_hci_reset_connection(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		if((retval = hci_connect_remote(ctx)))
		{
			BLTS_DEBUG("*** Connection failed - Test FAILED\n");
			bt_ctx_free(ctx);
			return retval;
		}

		if((retval = do_reset(ctx)))
		{
			BLTS_DEBUG("*** Reset failed - Test FAILED\n");
			bt_ctx_free(ctx);
			hci_disconnect_remote(ctx);
			return retval;
		}

		sleep(WAIT_TIME_AFTER_RESET);

		retval = hci_connect_remote(ctx);

		sleep(WAIT_TIME_CONNECT_DISCONNECT);

		retval |= hci_disconnect_remote(ctx);

		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}


int fute_bt_hci_ctrl_info_local()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		retval = read_and_verify_loc_ctrl_info(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}
int fute_bt_hci_ctrl_info_remote(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = collect_and_send_rem_ctrl_info(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

int fute_bt_hci_link_info_local()
{
	BLTS_DEBUG("*** Test case start\n");

	int retval = -1;

	struct bt_ctx *ctx;
	if( (ctx = bt_ctx_new()))
	{
		ctx->local_mac = *BDADDR_ANY;
		retval = read_and_verify_link_info(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");

	return retval;
}
int fute_bt_hci_link_info_remote(char *remote_mac)
{
	if(!remote_mac) return -EINVAL;
	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
//		retval = collect_and_send_link_info(ctx);
		retval = collect_and_send_link_info_one_by_one(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;
}

int fute_bt_hci_ll_pairing(char* remote_mac, int master)
{
	if (master && !remote_mac)
		return -EINVAL;

	BLTS_DEBUG("*** Test case start\n");
	int retval = -1;

	struct bt_ctx *ctx;

	if((ctx = bt_ctx_new()))
	{
		if (master)
			str2ba(remote_mac, &ctx->remote_mac);
		ctx->dev_id = 0;
		ctx->test_timeout.tv_sec = 10;
		ctx->test_timeout.tv_nsec = 0;
		retval = pairing_no_btd(ctx, master);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n",retval?"FAILED":"PASSED");
	return retval;

}

int fute_bt_le_scan()
{
	int retval = -1;
	struct bt_ctx *ctx;

	BLTS_DEBUG("*** Test case start\n");

	if ((ctx = bt_ctx_new())) {
		ctx->local_mac = *BDADDR_ANY;
		retval = do_le_scan(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n", retval ? "FAILED" : "PASSED");

	return retval;
}

int fute_bt_le_advertise()
{
	int retval = -1;
	struct bt_ctx *ctx;

	BLTS_DEBUG("*** Test case start\n");

	if ((ctx = bt_ctx_new())) {
		ctx->local_mac = *BDADDR_ANY;
		retval = le_set_advertise_mode(ctx, 1);
		sleep(WAIT_TIME_CONNECT_DISCONNECT);
		retval |= le_set_advertise_mode(ctx, 0);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n", retval ? "FAILED" : "PASSED");

	return retval;
}

int fute_bt_le_connect_disconnect(char *remote_mac)
{
	int retval = -1;
	struct bt_ctx *ctx;

	if (!remote_mac)
		return -EINVAL;

	BLTS_DEBUG("*** Test case start\n");

	if ((ctx = bt_ctx_new()))
	{
		str2ba(remote_mac, &ctx->remote_mac);
		retval = le_connect_remote(ctx);
		sleep(WAIT_TIME_CONNECT_DISCONNECT);
		retval |= le_disconnect_remote(ctx);
		bt_ctx_free(ctx);
	}

	BLTS_DEBUG("*** Test %s\n", retval ? "FAILED" : "PASSED");

	return retval;
}

