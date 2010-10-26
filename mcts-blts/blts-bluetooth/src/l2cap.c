/* l2cap.c -- L2CAP specific functionality

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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <blts_log.h>

#include "hci.h"
#include "l2cap.h"
#include "bt_ctx.h"
#include "gen_server.h"
#include "gen_client.h"

/** Configure socket opts for the test */
static int l2cap_set_test_sockopt(int sock)
{
	struct l2cap_options s_opt;
	size_t len = sizeof(s_opt);

	if(getsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &s_opt, &len) < 0)
	{
		BLTS_LOGGED_PERROR("getsockopt failure");
		return -errno;
	}

	s_opt.omtu = 16*1024;
	s_opt.imtu = 16*1024;

	if(setsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &s_opt, len) < 0)
	{
		BLTS_LOGGED_PERROR("setsockopt failure");
		return -errno;
	}

	return 0;
}

/** BT client connection config for test */
static int l2cap_configure_bt_conn_for_test(bdaddr_t *remote_ba)
{
	if(!remote_ba) return -EINVAL;

	int retval = 0;

	/* get device connected to address */
	int bt_dev;

	if((bt_dev = hci_open_dev(hci_get_route(remote_ba))) < 0)
	{
		BLTS_LOGGED_PERROR("Failure getting HCI connection");
		retval=-errno;
		goto cleanup;
	}

	/* retrieve connection info, esp. handle */

	struct hci_conn_info_req *cr = malloc(sizeof (*cr) + sizeof(struct hci_conn_info));

	bacpy(&cr->bdaddr, remote_ba);
	cr->type = ACL_LINK;

	if(ioctl(bt_dev, HCIGETCONNINFO, cr) < 0)
	{
		BLTS_LOGGED_PERROR("HCIGETCONNINFO ioctl failure");
		retval = -errno;
		goto cleanup;
	}

	/* make sure autoflush timeout for this link is infinite */
	uint16_t timeout = 0;

	if(hci_read_automatic_flush_timeout(bt_dev, cr->conn_info->handle, &timeout, 0) < 0)
	{
		BLTS_LOGGED_PERROR("hci_read_automatic_flush_timeout failure");
		retval = -errno;
		goto cleanup;
	}

	if(timeout)
	{
		BLTS_DEBUG("Adjusting autoflush timeout (was %lf ms)\n",(double)timeout * 0.625);
		if(hci_write_automatic_flush_timeout(bt_dev, cr->conn_info->handle, 0, 0) < 0)
		{
			BLTS_LOGGED_PERROR("hci_write_automatic_flush_timeout failure");
			retval = -errno;
		}
	}
cleanup:
	if(cr) free(cr);
	if(bt_dev >= 0) close(bt_dev);
	return retval;
}


/**
 * Handle an incoming connection on given socket. Timeout in ctx.
 */
static int l2cap_echo_server_wait_handle_one(struct bt_ctx *ctx, int sock)
{
	if((!ctx) || (sock < 0)) return -EINVAL;

	/* san check */
	if(ctx->test_timeout.tv_sec > 3600)
	{
		BLTS_DEBUG("Timeout too long\n");
		return -EINVAL;
	}

	/* Wait for incoming connection */

	fd_set read_fds;
	int ready;

	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	BLTS_DEBUG("Waiting...");
	ready = pselect(sock+1, &read_fds, (void*) 0, (void*) 0, &(ctx->test_timeout), (void*) 0);

	if(ready < 0)
	{
		BLTS_LOGGED_PERROR("pselect() failure");
		return -errno;
	}
	else if(!ready)
	{
		BLTS_DEBUG("No connections, timing out.\n");
		return -ETIMEDOUT;
	}

	BLTS_DEBUG("ok.\n");

	/* Ok, something is happening */

	int sock_in;
	struct sockaddr_l2 sa_in;
	socklen_t sa_in_len = sizeof(sa_in);
	memset(&sa_in, 0, sa_in_len);

	BLTS_DEBUG("Accepting connection...");
	if((sock_in = accept(sock, (void*)&sa_in, &sa_in_len)) < 0)
	{
		BLTS_LOGGED_PERROR("accept() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	int result = generic_server_handle_echo(sock_in);

	errno=0;
	if(close(sock_in) < 0)
	{
		BLTS_LOGGED_PERROR("Incoming socket close() failure");
	}
	return result?result:-errno;
}



/* -------------------------------------------------------------------------- */
/* Individual test code -> */

/**
 * L2CAP-specific portions of an echo server. Runs until one connection has
 * been served (= remote end has disconnected). Return 0 on success or -errno
 * otherwise. Uses local_(mac|port) (BDADDR_ANY -> default ifce), test_timeout in
 * ctx.
 */
int l2cap_echo_server_oneshot(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	BLTS_DEBUG("Server socket...");
	int sock;

	if((sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0)
	{
		BLTS_LOGGED_PERROR("Server socket() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	BLTS_DEBUG("Socket options...");
	if(l2cap_set_test_sockopt(sock) < 0)
	{
		/* error in fn */
		close(sock);
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	struct sockaddr_l2 local_sa;
	memset(&local_sa, 0, sizeof(local_sa));
	local_sa.l2_family = AF_BLUETOOTH;
	local_sa.l2_bdaddr = ctx->local_mac;
	local_sa.l2_psm = htobs(ctx->local_port);

	BLTS_DEBUG("Bind socket...");
	if(bind(sock, (void*)&local_sa, sizeof(local_sa)) < 0)
	{
		BLTS_LOGGED_PERROR("Server bind() failure");
		close(sock);
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	BLTS_DEBUG("Listen...");
	if(listen(sock, 1) < 0)
	{
		BLTS_LOGGED_PERROR("Server listen() failure");
		close(sock);
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	BLTS_DEBUG("Now waiting for connections.\n");
	int result = l2cap_echo_server_wait_handle_one(ctx, sock);

	errno=0;
	if(close(sock)<0)
	{
		BLTS_LOGGED_PERROR("close() failure for server socket");
	}
	BLTS_DEBUG("Server stopped.\n");

	return result?result:-errno;
}

/**
 * L2CAP-specific portions of the echo client test. Connects to server
 * (remote* in ctx), tests connection with short packet if want_connection_test
 * is nonzero. Return 0 or -errno.
 */
int l2cap_echo_test_client(struct bt_ctx *ctx, int want_transfer_test)
{
	if(!ctx) return -EINVAL;

	BLTS_DEBUG("Client socket...");
	int sock;

	if((sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0)
	{
		BLTS_LOGGED_PERROR("socket() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	BLTS_DEBUG("Socket options...");
	if(l2cap_set_test_sockopt(sock) < 0)
	{
		/* error in fn */
		close(sock);
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	struct sockaddr_l2 sa;
	memset(&sa, 0, sizeof(sa));
	sa.l2_family = AF_BLUETOOTH;
	sa.l2_psm = htobs(ctx->remote_port);
	sa.l2_bdaddr = ctx->remote_mac;

	BLTS_DEBUG("Connect...");
	if(connect(sock, (void*)&sa, sizeof(sa)) < 0)
	{
		BLTS_LOGGED_PERROR("connect() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	BLTS_DEBUG("Check BT connection settings...");
	if(l2cap_configure_bt_conn_for_test(&(ctx->remote_mac)) < 0)
	{
		/* error in fn */
		close(sock);
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	int result = 0;
	if(want_transfer_test)
	{
		BLTS_DEBUG("Connection test start.\n");
		result = generic_client_test_echo(sock);
		BLTS_DEBUG("Connection test finished.\n");
	}
	errno=0;
	if(close(sock) < 0)
	{
		BLTS_LOGGED_PERROR("Socket close() failure");
	}
	return result?result:-errno;
}


