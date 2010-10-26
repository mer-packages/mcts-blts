/* gen_client.c -- Connection-independent client functionality

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
#include <string.h>
#include <blts_log.h>

#include "gen_client.h"


int generic_client_test_echo(int sock)
{
	char outbuf[5] = "ping";
	char inbuf[5] = {0};
	ssize_t bytes_out=0,bytes_left=0,bytes_in=0;

	bytes_left = 4;
	while(bytes_left>0)
	{
		if((bytes_out = send(sock, outbuf, bytes_left, 0)) < 0)
		{
			BLTS_LOGGED_PERROR("send() failure");
			return -errno;
		}
		bytes_left -= bytes_out;
	}

	bytes_left = 4;
	while(bytes_left>0)
	{
		if((bytes_in = recv(sock, inbuf, bytes_left, 0)) < 0)
		{
			BLTS_LOGGED_PERROR("recv() failure");
			return -errno;
		}
		bytes_left -= bytes_in;
	}

	if(strncmp(inbuf,outbuf,4) != 0)
	{
		BLTS_DEBUG("Received data not equal to sent.");
		return -EIO;
	}
	return 0;
}
