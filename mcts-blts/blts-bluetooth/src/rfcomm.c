/* rfcomm.c -- RFCOMM specific functionality

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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <blts_log.h>

#include "hci.h"
#include "rfcomm.h"
#include "bt_ctx.h"
#include "gen_server.h"
#include "gen_client.h"


/**
 * Handle an incoming connection on given socket. Timeout in ctx.
 */
static int rfcomm_echo_server_wait_handle_one(struct bt_ctx *ctx, int sock)
{
	int ready, err, result;
	fd_set read_fds;
	long sk_flags = 0;

	if((!ctx) || (sock < 0)) return -EINVAL;

	/* san check */
	if(ctx->test_timeout.tv_sec > 3600)
	{
		BLTS_DEBUG("Timeout too long\n");
		return -EINVAL;
	}

	/* Set socket non-blocking (otherwise pselect..accept delay creates a race) */
	sk_flags = fcntl(sock, F_GETFL);
	if(sk_flags < 0) {
		BLTS_LOGGED_PERROR("fcntl(F_GETFL) on listen socket failed\n");
		return -errno;
	}
	sk_flags |= O_NONBLOCK;
	err = fcntl(sock, F_SETFL, sk_flags);
	if(err) {
		BLTS_LOGGED_PERROR("fcntl(F_SETFL) on listen socket failed\n");
		return -errno;
	}

	/* Wait for incoming connection */

	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	BLTS_DEBUG("Waiting...");
	ready = pselect(sock+1, &read_fds, (void*) 0, (void*) 0, &(ctx->test_timeout), (void*) 0);

	if(ready < 0 && errno != EINTR)
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
	struct sockaddr_rc sa_in;
	socklen_t sa_in_len = sizeof(sa_in);
	memset(&sa_in, 0, sa_in_len);

	BLTS_DEBUG("Accepting connection...");
	if((sock_in = accept(sock, (void*)&sa_in, &sa_in_len)) < 0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			BLTS_DEBUG("already gone, stopping.\n");
			/* Note: we can't generally tell if this was
			 * intended by the client. */
			return 0;
		}
		BLTS_LOGGED_PERROR("accept() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	/* Restore blocking mode */
	sk_flags = fcntl(sock, F_GETFL);
	if(sk_flags < 0) {
		BLTS_LOGGED_PERROR("fcntl(F_GETFL) on listen socket failed\n");
		result = -errno;
		goto done;
	}
	sk_flags &= ~O_NONBLOCK;
	err = fcntl(sock, F_SETFL, sk_flags);
	if(err) {
		BLTS_LOGGED_PERROR("fcntl(F_SETFL) on listen socket failed\n");
		result = -errno;
		goto done;
	}

	result = generic_server_handle_echo(sock_in);
done:
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
 * RFComm-specific portions of an echo server. Runs until one connection has
 * been served (= remote end has disconnected). Return 0 on success or -errno
 * otherwise. Uses local_(mac|port) (BDADDR_ANY -> default ifce), test_timeout in
 * ctx.
 */
int rfcomm_echo_server_oneshot(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	BLTS_DEBUG("Server socket...");
	int sock;

	if((sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
	{
		BLTS_LOGGED_PERROR("Server socket() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	struct sockaddr_rc local_sa;
	memset(&local_sa, 0, sizeof(local_sa));
	local_sa.rc_family = AF_BLUETOOTH;
	local_sa.rc_bdaddr = ctx->local_mac;
	local_sa.rc_channel = htobs(ctx->local_port);

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
	int result = rfcomm_echo_server_wait_handle_one(ctx, sock);

	errno=0;
	if(close(sock)<0)
	{
		BLTS_LOGGED_PERROR("close() failure for server socket");
	}
	BLTS_DEBUG("Server stopped.\n");

	return result?result:-errno;
}

/**
 * RFComm-specific portions of the echo client test. Connects to server
 * (remote* in ctx), tests connection with short packet if want_connection_test
 * is nonzero. Return 0 or -errno.
 */
int rfcomm_echo_test_client(struct bt_ctx *ctx, int want_transfer_test)
{
	if(!ctx) return -EINVAL;

	BLTS_DEBUG("Client socket...");
	int sock;

	if((sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0)
	{
		BLTS_LOGGED_PERROR("socket() failure");
		return -errno;
	}
	BLTS_DEBUG("ok.\n");

	struct sockaddr_rc sa;
	memset(&sa, 0, sizeof(sa));
	sa.rc_family = AF_BLUETOOTH;
	sa.rc_channel = htobs(ctx->remote_port);
	sa.rc_bdaddr = ctx->remote_mac;

	BLTS_DEBUG("Connect...");
	if(connect(sock, (void*)&sa, sizeof(sa)) < 0)
	{
		BLTS_LOGGED_PERROR("connect() failure");
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


