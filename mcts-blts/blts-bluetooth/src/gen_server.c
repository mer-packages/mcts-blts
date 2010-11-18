/* gen_server.c -- Connection-independent server functionality

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
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <blts_log.h>
#include <string.h>

#include "gen_server.h"


/**
 * Serve one echo client connection (incoming socket in sock). Return 0 or -errno.
 */
int generic_server_handle_echo(int sock)
{
	int retval = 0;
	ssize_t bytes_in = 0;
	ssize_t bytes_left = 0;
	ssize_t bytes_out = 0;

	size_t buf_len = 65536;  /* max BT mtu */
	char *buf = malloc(buf_len);

	while((bytes_in = recv(sock, buf, buf_len, 0)) > 0)
	{
		bytes_left = bytes_in;

		while(bytes_left>0)
		{
			if((bytes_out = send(sock, buf + bytes_in - bytes_left, bytes_left, 0)) < 0)
			{
				BLTS_LOGGED_PERROR("send() failure");
				retval = -errno;
				goto cleanup;
			}
			bytes_left -= bytes_out;
		}
	}

	/* we expect the client to quit at any time (except when actively rx'ing) */
	if((bytes_in < 0) && (errno!=ECONNRESET))
	{
		BLTS_LOGGED_PERROR("recv() failure");
		retval = -errno;
	}

cleanup:
	free(buf);

	return retval;
}
