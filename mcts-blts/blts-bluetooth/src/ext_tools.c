/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* ext_tools.c -- External tools wrapper

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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stropts.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <poll.h>

#include <blts_log.h>

#include "ext_tools.h"

static int       stop_thread = 0;
static pthread_t thread_id   = 0;

typedef struct {
	char *log_file;
	int   log_fd;

	char *hci_dev;
	int   hci_dev_id;
	int   hci_dev_fd;

	int   raw_sock_id;
} blts_hci_thread;

struct blts_hci_header {
	uint16_t length;    /* Packet length */
	uint8_t  in;        /* Packet direction */
	uint32_t time_sec;  /* Time stamp */
	uint32_t time_usec;
} __attribute__ ((packed));

static blts_hci_thread *the_data = NULL;

static pthread_mutex_t the_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Get state if thread should stop */
static int
get_stop_thread (void)
{
	int status;

	pthread_mutex_trylock (&the_mutex);

	status = stop_thread;

	pthread_mutex_unlock (&the_mutex);

	return status;
}

/* Write stop state for thread */
static void
set_stop_thread (int status)
{
	pthread_mutex_trylock (&the_mutex);

	stop_thread = status;

	pthread_mutex_unlock (&the_mutex);
}

/* Create thread data, based on info given in */
static blts_hci_thread *
create_thread_data (const char *log_file, const char *hci_device)
{
	blts_hci_thread *data = NULL;

	FUNC_ENTER();

	/* Create thread data or fail */
	data = malloc (sizeof (blts_hci_thread));

	if (!data) {
		BLTS_ERROR ("Out of memory!\n");
		return NULL;
	}

	memset (data, 0, sizeof (blts_hci_thread));

	if (log_file) {
		data->log_file = strdup (log_file);
	}

	if (hci_device) {
		data->hci_dev = strdup (hci_device);
	}

	BLTS_DEBUG ("HCI dev: %s, log file: %s\n", data->hci_dev,
		    data->log_file);

	if (data->hci_dev) {
		data->hci_dev_id = atoi (data->hci_dev + 3);
	} else {
		data->hci_dev_id = hci_get_route (NULL);
	}

	BLTS_DEBUG ("Using device %d\n", data->hci_dev_id);

	FUNC_LEAVE();

	return data;
}

/* Free thread data */
static void
free_thread_data (blts_hci_thread **ptr)
{
	if (ptr == NULL)
		return;
	if (*ptr == NULL)
		return;

	if ((*ptr)->log_file) {
		free ((*ptr)->log_file);
		(*ptr)->log_file = NULL;
	}

	if ((*ptr)->log_fd > 0) {
		close ((*ptr)->log_fd);
		(*ptr)->log_fd = 0;
	}

	if ((*ptr)->hci_dev) {
		free ((*ptr)->hci_dev);
		(*ptr)->hci_dev = NULL;
	}
	(*ptr)->hci_dev_id = 0;

	free (*ptr);
	*ptr = NULL;
}

/* Opens the logfile */
static int
open_log_file (blts_hci_thread *data)
{
	if (!data)
		return -1;

	if (data->log_fd > 0)
		return -1;

	if (!data->log_file)
		return -1;

	data->log_fd = open (data->log_file, O_WRONLY | O_CREAT | O_TRUNC,
			     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (data->log_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open log file for writing.");
		BLTS_ERROR ("Error %s\n", strerror (errno));
		data->log_fd = -1;
		return -1;
	}

	return data->log_fd;
}

/* Opens a hci device node and socket to it */
static int
open_hci_socket (blts_hci_thread *data)
{
	int opt;
	struct hci_dev_info dev_info;
	struct hci_filter flt;
	struct sockaddr_hci addr;
	char btname[256] = {'\0'};

	FUNC_ENTER();

	if (!data)
		return -1;

	/* Set hci device into raw mode */
	data->hci_dev_fd = hci_open_dev (data->hci_dev_id);

	if (data->hci_dev_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open hci device");
		return -1;
	}

	if (hci_devinfo (data->hci_dev_id, &dev_info) < 0) {
		BLTS_LOGGED_PERROR ("Can't get device info");
		return -1;
	}

	BLTS_DEBUG ("Mapped to device: %s\n", dev_info.name);

	if (hci_read_local_name (data->hci_dev_fd, 256, btname, 0) < 0) {
		BLTS_LOGGED_PERROR ("Can't read local name");
		return -1;
	}

	BLTS_DEBUG ("Device local name: %s\n", btname);

	opt = hci_test_bit (HCI_RAW, &dev_info.flags);

	if (ioctl (data->hci_dev_fd, HCISETRAW, opt) < 0) {
		if (errno == EACCES) {
			BLTS_LOGGED_PERROR ("Can't access device");
			return -1;
		}
	}

	hci_close_dev (data->hci_dev_fd);
	data->hci_dev_fd = -1;

	/* Create HCI socket */
	data->raw_sock_id = socket (AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

	if (data->raw_sock_id < 0) {
		BLTS_LOGGED_PERROR ("Can't create raw socket!");
		return -1;
	}

	/* Set as non block */
	opt = fcntl (data->raw_sock_id, F_GETFL, NULL);

	opt |= O_NONBLOCK;

	if (fcntl (data->raw_sock_id, F_SETFL, opt) < 0) {
		BLTS_LOGGED_PERROR ("Failed to set non-blocking");
		close (data->raw_sock_id);
		data->raw_sock_id = -1;
		return -1;
	}

	/* Set direction info */
	opt = 1;
	if (setsockopt (data->raw_sock_id, SOL_HCI, HCI_DATA_DIR, &opt,
			sizeof (opt)) < 0) {
		BLTS_LOGGED_PERROR ("Can't enable data direction info");
		close (data->raw_sock_id);
		data->raw_sock_id = -1;
		return -1;
	}

	/* Enable time stamps */
	opt = 1;
	if (setsockopt (data->raw_sock_id, SOL_HCI, HCI_TIME_STAMP, &opt,
			sizeof (opt)) < 0) {
		BLTS_LOGGED_PERROR ("Can't enable time stamp");
		close (data->raw_sock_id);
		data->raw_sock_id = -1;
		return -1;
	}

	/* Allow all ptypes and events */
	hci_filter_clear (&flt);
	hci_filter_all_ptypes (&flt);
	hci_filter_all_events (&flt);
	if (setsockopt (data->raw_sock_id, SOL_HCI, HCI_FILTER, &flt,
			sizeof (flt)) < 0) {
		BLTS_LOGGED_PERROR ("Can't set filter");
		close (data->raw_sock_id);
		data->raw_sock_id = -1;
		return -1;
	}

	/* Bind socket to the HCI device */
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = data->hci_dev_id;

	if (bind (data->raw_sock_id, (struct sockaddr *) &addr,
		  sizeof(addr)) < 0) {
		BLTS_LOGGED_PERROR ("Binding failed");
		close (data->raw_sock_id);
		data->raw_sock_id = -1;
		return -1;
	}

	BLTS_DEBUG ("Bound to raw socket %d\n", data->raw_sock_id);

	FUNC_LEAVE();

	return data->raw_sock_id;
}

/* Writes data from raw socket into a file in hexdump format */
static int
process_socket_data (blts_hci_thread *data)
{
	struct blts_hci_header *hdr;
	struct cmsghdr         *cmessage;
	struct msghdr           message;
	struct iovec            iov;
	struct timeval         *ts;

	char *buffer = NULL;
	char *ctrl = NULL;
	int len, written;

	FUNC_ENTER();

	int header_size = sizeof (struct blts_hci_header);
	int data_size = header_size + HCI_MAX_FRAME_SIZE;

	/* Buffer and control buffer get owned by the the message struct. Do
	 * not free?!
	 *
	 * Now, the buffer is aligned in the message struct, so the
	 * header_size amount is leaked each time this function is called.
	 */
	buffer = malloc (data_size);
	if (!buffer) {
		BLTS_LOGGED_PERROR ("Can't allocate buffer");
		return -1;
	}

	ctrl = malloc (100);
	if (!ctrl) {
		BLTS_LOGGED_PERROR ("Can't allocate control buffer");
		free (buffer);
		return -1;
	}

	memset (buffer, 0, data_size);
	memset (&message, 0, sizeof (message));

	/* Buffer position and size is a bit obscure, but this is the
	 * way, how hcidump does it originally. And have to admit, it's
	 * a simple way of adding a custom header info into the dump.
	 *
	 * Note here, that these two pointers, the actual data and header,
	 * are allocated into same pointer, which is then written into the
	 * dump file. The data received from socket is stored into the
	 * buffer, that's aligned to skip the header data.
	 */
	hdr = (void *)buffer;
	iov.iov_base = buffer + header_size;
	iov.iov_len  = HCI_MAX_FRAME_SIZE;

	message.msg_iov    = &iov;
	message.msg_iovlen = 1;

	message.msg_control    = ctrl;
	message.msg_controllen = 100;

	len = recvmsg (data->raw_sock_id, &message, MSG_DONTWAIT);

	if (len < 0) {
		if (errno == EINTR || errno == EAGAIN) {
			return 0;
		}
		BLTS_LOGGED_PERROR ("Receive failed.");
		return -1;
	}

	if (!len) {
		BLTS_ERROR ("Client disconnected.\n");
		return -1;
	}

	BLTS_DEBUG ("Read %d bytes\n", len);

	hdr->length = htobs (len);

	cmessage = CMSG_FIRSTHDR (&message);
	while (cmessage) {
		if (cmessage->cmsg_type == HCI_CMSG_DIR) {
			hdr->in = *(CMSG_DATA (cmessage));
		} else if (cmessage->cmsg_type == HCI_CMSG_TSTAMP) {
			ts = (struct timeval *)CMSG_DATA (cmessage);
			hdr->time_sec = htobl (ts->tv_sec);
			hdr->time_usec = htobl (ts->tv_usec);
		}
		cmessage = CMSG_NXTHDR (&message, cmessage);
	}

	/* Add header size to length to be written */
	len += header_size;

	BLTS_DEBUG ("Writing %d bytes (added header data)\n", len);

	do {
		written = write (data->log_fd, buffer, len);

		if (written < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			BLTS_LOGGED_PERROR ("Writing to log failed.");
			return -1;
		}
		if (!written)
			break;
		len -= written;
		buffer += written;
		BLTS_DEBUG ("Wrote %d bytes, %d remaining.\n", written, len);
	} while (len > 0);

	FUNC_LEAVE();

	return 1;
}

/* The thread function */
static void *
read_hci_socket (void *user_data)
{
	blts_hci_thread *data = user_data;
	struct pollfd fds[1];
	int ret;

	FUNC_ENTER();

	if (!data)
		pthread_exit ((void *)-1);

	/* Watch the socket data */
	fds[0].fd = data->raw_sock_id;
	fds[0].events = POLLIN;
	fds[0].revents = 0;

	while (!get_stop_thread()) {
		ret = poll (fds, 1, 2000);

		if (ret <= 0) {
			BLTS_DEBUG ("No data in 2 seconds.\n");
			continue;
		}

		if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
			BLTS_DEBUG ("Socket disconnected\n");
			break;
		}

		ret = process_socket_data (data);

		if (ret < 0) {
			BLTS_ERROR ("Failed to process data\n");
			pthread_exit ((void *)-3);
		}
	}

	FUNC_LEAVE();

	pthread_exit (NULL);
}

/* Reads the dump file, well any file, based on file descriptor */
static int
read_dump (int file, char *buf, int len)
{
        int retval = 0;
	int n_read = 0;

        while (len > 0) {
		n_read = read (file, buf, len);

		if (n_read < 0) {
                        if (errno == EINTR || errno == EAGAIN)
                                continue;
			BLTS_LOGGED_PERROR ("Failed to read");
                        return -1;
                }
                if (!n_read)
                        return 0;
                len    -= n_read;
		buf    += n_read;
		retval += n_read;

		BLTS_DEBUG ("Read %d bytes, %d left\n", n_read, len);
        }
        return retval;
}

/* Starts hci dump */
int
start_hcidump (char *log_file, char *hci_device)
{
	int ret = 0;

	FUNC_ENTER();

	/* Already got thread running.. most likely anyway */
	if (the_data != NULL) {
		BLTS_ERROR ("Already dumping hci data.\n");
		return -1;
	}

	if (thread_id) {
		BLTS_ERROR ("Aready running a hci dump thread.\n");
		return -1;
	}

	the_data = create_thread_data (log_file, hci_device);

	if (!the_data) {
		BLTS_ERROR ("Failed to create context for thread.\n");
		return -1;
	}

	ret = open_log_file (the_data);

	if (ret < 0) {
		BLTS_ERROR ("Failed to open log file!\n");
		free_thread_data (&the_data);
		return -1;
	}

	ret = open_hci_socket (the_data);

	if (ret < 0) {
		BLTS_ERROR ("Failed to open HCI socket!\n");
		free_thread_data (&the_data);
		return -1;
	}

	/* Create a thread or fail */
	ret = pthread_create (&thread_id, NULL, read_hci_socket, the_data);

	if (ret) {
		BLTS_ERROR ("Read thread creation failed!\n");
		free_thread_data (&the_data);
		return -1;
	}

	FUNC_LEAVE();

	return 1;
}

/* Stops hci dump */
int
stop_hcidump ()
{
	int ret, thr_join = 0;

	if (thread_id <= 0) {
		BLTS_ERROR ("No dump thread running.\n");
		return -1;
	}

	if (!the_data) {
		BLTS_ERROR ("No thread data, assuming thread is not "
			    "running.\n");
		return -1;
	}

	set_stop_thread (1);

	ret = pthread_join (thread_id, (void**)&thr_join);

	if (ret) {
		BLTS_LOGGED_PERROR ("Failed to join thread.");
		free_thread_data (&the_data);
		return -1;
	}

	if (thr_join) {
		BLTS_ERROR ("Thread exited with %d\n", thr_join);
		free_thread_data (&the_data);
		return thr_join;
	}

	free_thread_data (&the_data);

	return 1;
}

/* Parses through hci dump and passes it to parser func */
int
parse_hcidump (char *dump_file,
	       hcidump_trace_handler trace_handler,
	       void *user_data)
{
	struct blts_hci_header hdr;
	struct hcidump_trace   trace;
	struct stat sb;
	int   retval = 0;
	int   log_fd = 0;
	char *buffer = NULL;
	int   header_size = sizeof (struct blts_hci_header);

	FUNC_ENTER();

	if (!dump_file || stat (dump_file, &sb) < 0) {
		BLTS_ERROR ("No dump to parse.");
		retval = -1;
		goto CLEANUP;
	}

	log_fd = open (dump_file, O_RDONLY,
		       S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (log_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open log file.");
		retval = -1;
		goto CLEANUP;
	}

	buffer = malloc (HCI_MAX_FRAME_SIZE);

	if (!buffer) {
		BLTS_LOGGED_PERROR ("Failed to allocate memory for buffer");
		close (log_fd);
		retval = -1;
		goto CLEANUP;
	}

	do {
		retval = read_dump (log_fd, (char *) &hdr, header_size);

		if (retval < 0) {
			BLTS_ERROR ("Failed to read packet header!\n");
			goto CLEANUP;
		}

		trace.data_len = btohl (hdr.length);
		trace.incoming = hdr.in;
		trace.tv_sec   = btohl (hdr.time_sec);
		trace.tv_usec  = btohl (hdr.time_usec);
		trace.data     = buffer;

		memset (buffer, 0, HCI_MAX_FRAME_SIZE);

		retval = read_dump (log_fd, buffer, trace.data_len);

		if (retval < 0) {
			BLTS_ERROR ("Failed to read dump data!\n");
			goto CLEANUP;
		}

		if (trace_handler (&trace, user_data)) {
			BLTS_ERROR ("Failed to handle trace!\n");
			retval = -1;
			goto CLEANUP;
		}

	} while (retval != 0);

	FUNC_LEAVE();

CLEANUP:
	if (log_fd > 0)
		close (log_fd);
	if (buffer)
		free (buffer);

	return retval;
}
