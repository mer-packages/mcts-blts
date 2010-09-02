/* info.c -- HCI information exchange related functions

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
#include <sys/ioctl.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <blts_log.h>
#include <pthread.h>
#include "info.h"

/* L2CAP channels: odd-numbered ports 4097-32767 */
#define TEST_CHANNEL1 0x1001 //4097
#define TEST_CHANNEL2 0x37ff //14335
#define TEST_CHANNEL3 0x7FFF //32767
#define TEST_CHANNEL_COUNT 3

const unsigned short test_channels[TEST_CHANNEL_COUNT] =
{
	TEST_CHANNEL1,
	TEST_CHANNEL2,
	TEST_CHANNEL3
};

typedef struct _thread_data
{
	int idx;
    struct bt_ctx* ctx;
	int result;
} thread_data;

pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t connect_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Prints out content of the device_info_t structure, parameter remote
 * is used to indicate which prefix is used: (local) 0 = L (remote) 1 = R
 */
void dump_ctrl_information_data(device_info_t* devptr, int remote)
{
	if(!devptr)
	{
		log_print("%s: Invalid pointer to device_info_t struct passed!\n",
					__FUNCTION__);
		return;
	}

	int i = 0;
	char dir = remote?'R':'L';

	if(devptr->got_features)
		log_print("%c features: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
                                "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n", dir,
                		devptr->features[0], devptr->features[1], devptr->features[2],
                		devptr->features[3], devptr->features[4], devptr->features[5],
                		devptr->features[6], devptr->features[7]);

	if(devptr->got_version)
		log_print("%c version: %d-%d-%d-%d-%d\n", dir,
				devptr->version.manufacturer,
				devptr->version.hci_ver,
				devptr->version.hci_rev,
				devptr->version.lmp_ver,
				devptr->version.lmp_subver);

	if(devptr->got_ext_features)
	{
		log_print("%c ext_features: \n", dir);
		for(i=0; i<MAX_EXT_PAGES_TO_VERIFY && i<devptr->max_page; i++)
		{
			log_print("%c page %d: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x "
                                "0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n", dir, i+1,
                		devptr->ext_features[i][0], devptr->ext_features[i][1], devptr->ext_features[i][2],
                		devptr->ext_features[i][3], devptr->ext_features[i][4], devptr->ext_features[i][5],
                		devptr->ext_features[i][6], devptr->ext_features[i][7]);
			log_print("\n");
		}
	}
}

/**
 * Prints out content of the link_info_t structure, parameter remote
 * is used to indicate which prefix is used: (local) 0 = L (remote) 1 = R
 */
void dump_link_information_data(link_info_t* linkptr, int remote)
{
	if(!linkptr)
	{
		log_print("%s: Invalid pointer to link_info_t struct passed!\n",
					__FUNCTION__);
		return;
	}

	int i = 0;
	char dir = remote?'R':'L';

	if(linkptr->got_rssi)
		log_print("%c: RSSI return value: %d\n", dir, linkptr->rssi);

	if(linkptr->got_lq)
		log_print("%c: link quality: %d\n", dir, linkptr->lq);

	if(linkptr->got_level)
		log_print("%c: current transmit power level: %d\n", dir, linkptr->level);

	if(linkptr->got_map)
	{
		if(linkptr->got_map == (uint8_t)-1)
		{
			log_print("%c: AFH disabled", dir);
		}
		else
		{
			log_print("%c: AFH map 0x: ", dir);
			for (i = 0; i < 10; i++)
				log_print("%02x", linkptr->map[i]);
			log_print("\n");
		}
	}
	if(linkptr->got_clock)
		log_print("%c: clock: 0x%4.4x\n", dir, btohl(linkptr->clock));

	if(linkptr->got_offset)
		log_print("%c: clock offset: 0x%4.4x\n", dir, btohs(linkptr->offset));
}

/**
 * Accepts a connection from remote device and fills remote MAC to a given
 * bt_ctx structure. On success, descriptor for the accepted socket is returned.
 * On error, -1 is returned
 */
int do_accept(struct bt_ctx *ctx, int fd)
{
	if(!ctx)
	{
		log_print("%s: Invalid pointer to bt_ctx struct passed!\n",
				__FUNCTION__);
		return -1;
    }

    struct sockaddr_l2 addr;
    socklen_t addrlen;
    int client = -1;
    char buf[19] = "";
    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if ((client = accept(fd, (struct sockaddr *) &addr, &addrlen)) < 0)
    {
       	log_print("%s: Cannot accept connection %s(%d)",
               		__FUNCTION__, strerror(errno), errno);
        return -1;
    }

    ba2str( &addr.l2_bdaddr, buf );
	log_print("Accepted a connection from %s\n", buf);

	ctx->remote_mac = addr.l2_bdaddr;

	return client;
}

/**
 * Creates a L2CAP socket and starts listening data packets from other DUT.
 * The port parameter is optional and is omitted if -1 is passed to this
 * function (ctx->local_mac is then used instead). On success, a file descriptor
 * for the new socket is returned. On error, -1 is returned
 */
int do_listen(struct bt_ctx *ctx, int port)
{
	if(!ctx)
	{
		log_print("%s: Invalid pointer to bt_ctx struct passed!\n",
				__FUNCTION__);
		return -1;
	}

    struct sockaddr_l2 addr;
    int sock = -1;

    /* creates a socket */
    sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    if (sock < 0)
	{
    	log_print("%s: Cannot create socket %s(%d)",
              		__FUNCTION__, strerror(errno), errno);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
	addr.l2_bdaddr = ctx->local_mac;
	if(port < 0)
		addr.l2_psm = htobs(ctx->local_port);
	else
		addr.l2_psm = htobs(port);

	/* binds a socket to a local socket address */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
      	log_print("%s: Cannot bind socket %s(%d)",
           		__FUNCTION__, strerror(errno), errno);
        return -1;
    }

    /* start listening mode */
    log_print("Start listening data packets from other DUT...\n");
    if(listen(sock,1) < 0)
    {
       	log_print("%s: Cannot listen socket %s(%d)",
       			__FUNCTION__, strerror(errno), errno);
       	close(sock);
        return -1;
    }

    return sock;
}

/**
 * Compares two device_info_t structures and returns 0 when contents are equal
 * otherwise -1 is returned
 */
int compare_device_info_data(device_info_t* dev1, device_info_t* dev2)
{
	int i = 0;
	int j = 0;

	if(!dev1 || !dev2) return -1;

	if(	dev1->got_version != dev2->got_version 		||
		dev1->got_features != dev2->got_features	||
		dev1->got_ext_features != dev2->got_ext_features)
		{
			log_print("Cannot compare these structures - not equally filled\n");
			return -1;
		}

	log_print("verify features...\n");
	if(dev1->got_features && dev2->got_features)
	  for (i = 0; i < 8; i++)
		  if (dev1->features[i] != dev2->features[i])
			  return -1;


	log_print("verify version...\n");
	if(dev1->got_version && dev2->got_version)
	{
			if( dev1->version.manufacturer != dev2->version.manufacturer ||
		//		dev1->version.hci_ver != dev2->version.hci_ver ||
		//		dev1->version.hci_rev != dev2->version.hci_rev ||
				dev1->version.lmp_ver != dev2->version.lmp_ver ||
				dev1->version.lmp_subver != dev2->version.lmp_subver )
				return -1;
	}

	log_print("verify ext_features...\n");
	if(dev1->got_ext_features && dev2->got_ext_features)
	{
		for(j=0; j<MAX_EXT_PAGES_TO_VERIFY && j<dev1->max_page; j++)
		{
			for (i = 0; i < 8; i++)
			  if (dev1->ext_features[j][i] != dev2->ext_features[j][i])
				  return -1;
		}
	}

	return 0;
}

/**
 * Compares two link_info_t structures and returns 0 when contents are equal
 * otherwise -1 is returned
 */
int compare_link_info_data(link_info_t* dev1, link_info_t* dev2)
{
	int i = 0;

	if(!dev1 || !dev2) return -1;

	if(	dev1->got_rssi != dev2->got_rssi 	||
		dev1->got_lq != dev2->got_lq 		||
		dev1->got_level != dev2->got_level	||
		dev1->got_map != dev2->got_map		||
		dev1->got_clock != dev2->got_clock	||
		dev1->got_offset != dev2->got_offset)
		{
			log_print("Cannot compare these structures - not equally filled");
			return -1;
		}

	//TODO improve compare? - rssi / level / clock

	log_print("verify link quality...\n");
	if(dev1->got_lq && dev2->got_lq)
		if (dev1->lq != dev2->lq)
			log_print("  Note: Link quality mismatch (manufacturer specific); %d != %d\n", dev1->lq, dev2->lq);

	log_print("verify clock offset...\n");
	if(dev1->got_offset && dev2->got_offset)
		if (dev1->offset != dev2->offset) {
			if (abs(dev1->offset - dev2->offset) > 1)
				return -1;
			log_print("  Note: Clock drift +- 1 during test (still passing test)\n");
		}

	log_print("verify AFH map...\n");
	if(dev1->got_map && dev2->got_map)
		for (i = 0; i < 10; i++)
			 if (dev1->map[i] != dev2->map[i])
				return -1;
	return 0;
}

/**
 * Read local HCI controller information, receive information data
 * from other DUT and verify that locally collected information
 * matches with received data
 */
int read_and_verify_loc_ctrl_info(struct bt_ctx *ctx)
{
	int sock = -1;
	int client = -1;
	int retval = -1;
	int bytes_read = -1;
	device_info_t remote_info;
	device_info_t* local_info = NULL;

	if(hci_init_controller(ctx) < 0)
	{
		log_print("HCI Initialization failed!\n");
		return -1;
	}

	local_info = hci_get_info_local(ctx);

	if(!local_info)
	{
		log_print("Cannot read controller information!\n");
		retval=-1;
		goto cleanup;
	}

	/* create socket and start listening */
	sock = do_listen(ctx, -1);

	if(sock < 0)
	{
		log_print("Socket listening failed!\n");
		retval=-1;
		goto cleanup;
	}

	/* accept one connection */
	client = do_accept(ctx, sock);

	if(client < 0)
	{
		log_print("Accepting client connection failed!\n");
		retval=-1;
		goto cleanup;
	}

	/* read information packet from remote socket */
	bytes_read = recv(client, &remote_info, sizeof(remote_info), 0);
    if( bytes_read > 0 )
	{
    	log_print("Compare remote controller information data with local one...\n");
    	dump_ctrl_information_data(local_info, 0);
    	dump_ctrl_information_data(&remote_info, 1);

		if(!compare_device_info_data(&remote_info, local_info))
		{
			log_print("Information comparison success\n");
			retval=0;
		}
		else
		{
			log_print("Information comparison failed\n");
			retval=-1;
		}
    }
	else
	{
		log_print("Cannot read information from remote socket\n");
		retval=-1;
	}

cleanup:

	if(local_info) free(local_info);

	/* close connections */
	if(client != -1) close(client);
	if(sock != -1) close(sock);

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		logged_perror("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

/**
 * Read remote HCI controller information, connect to other DUT
 * and send collected information data
 */
int collect_and_send_rem_ctrl_info(struct bt_ctx *ctx)
{
	struct sockaddr_l2 rem_addr;
    int sock = -1;
	int status = -1;
	int retval = -1;
	char addr[19];

	device_info_t* remote_info = NULL;

	memset(&rem_addr, 0, sizeof(rem_addr));
    /* allocate a socket */
    sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

	/* set the connection parameters */
    rem_addr.l2_family = AF_BLUETOOTH;
	rem_addr.l2_bdaddr = ctx->remote_mac;
	rem_addr.l2_psm = htobs(0x1001);

	if(hci_init_controller(ctx) < 0)
	{
		log_print("HCI Initialization failed!\n");
		return -1;
	}

	remote_info = hci_get_info_remote(ctx);

	if(!remote_info)
	{
		log_print("Cannot read controller information!\n");
		retval=-1;
		goto cleanup;
	}

	dump_ctrl_information_data(remote_info, 1);

	ba2str( &rem_addr.l2_bdaddr, addr );
	log_print("Connection to %s\n", addr);

	/* connect to other DUT */
	status = connect(sock, (struct sockaddr *)&rem_addr, sizeof(rem_addr));

	/* send information packet */
	if( status == 0 )
	{
		int n = -1;

		log_print("Write controller information data to socket...\n");
		n = write(sock, remote_info, sizeof(*remote_info));
		if (n < 0)
		{
			log_print("Error writing to socket\n");
			retval=-1;
		}
		else
		{
			log_print("%d bytes written\n", n);
			retval=0;
		}
	}
	else
	{
		log_print("Error connecting to remote socket:%d\n", status);
		retval=-1;
	}

cleanup:
	if(remote_info)	free(remote_info);

	if(sock != -1) close(sock);

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		logged_perror("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

/**
 * Create threads to read and verify link information about multiple L2CAP test channels
 */
int read_and_verify_link_info(struct bt_ctx *ctx)
{
	int i = 0;
	int j = 0;
	int retval = 0;
	uint32_t rc;
	pthread_t ids[TEST_CHANNEL_COUNT];

	if(hci_init_controller(ctx) < 0)
	{
		log_print("HCI Initialization failed!\n");
		return -1;
	}

	thread_data* array =  (thread_data*) malloc(sizeof(thread_data) * TEST_CHANNEL_COUNT);

	if(!array)
	{
		log_print("Memory allocation failed!\n");
		retval = -1;
		goto cleanup;
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		array[i].idx = i;
		array[i].ctx = ctx;

		if ((rc = pthread_create (&ids[i], NULL, read_link_info_thread, (void *) &(array[i]))) != 0)
		{
			log_print("Cannot create thread:%d \n", i);
				for (j = 0; j < i; j++)
					pthread_cancel (ids[j]);
				retval = -1;
				goto cleanup;
		}
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		pthread_join(ids[i], NULL);
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		if(array[i].result != 0)
		{
			retval = -1;
			break;
		}
	}

cleanup:
	if(array) free(array);

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		logged_perror("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

/**
 * Create threads to collect and send link information about multiple L2CAP test channels
 */
int collect_and_send_link_info(struct bt_ctx *ctx)
{
	int i = 0;
	int j = 0;
	int retval = 0;
	uint32_t rc;
	pthread_t ids[TEST_CHANNEL_COUNT];

	if(hci_init_controller(ctx) < 0)
	{
		log_print("HCI Initialization failed!\n");
		return -1;
	}

	thread_data* array =  (thread_data*) malloc(sizeof(thread_data) * TEST_CHANNEL_COUNT);

	if(!array)
	{
		log_print("Memory allocation failed!\n");
		retval = -1;
		goto cleanup;
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		array[i].idx = i;
		array[i].ctx = ctx;

		if ((rc = pthread_create (&ids[i], NULL, send_link_info_thread, (void *) &(array[i]))) != 0)
		{
			log_print("Cannot create thread:%d \n", i);
			for (j = 0; j < i; j++)
				pthread_cancel (ids[j]);
			retval = -1;
			goto cleanup;
		}
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		pthread_join(ids[i], NULL);
	}

	for (i = 0; i < TEST_CHANNEL_COUNT; i++)
	{
		if(array[i].result != 0)
		{
			retval = -1;
			break;
		}
	}

cleanup:
	if(array) free(array);

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		logged_perror("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

/**
 * Thread function to listen one test channel and to receive link information
 * data package from other DUT. The received data is compared with locally
 * collected data and finally the end of communication mark is sent to other DUT.
 * The comparison result (0 = success / -1 = failure ) is passed to caller in
 * result field of the thread_data structure
 */
void *read_link_info_thread(void *dataptr)
{
	int sock = -1;
	int client = -1;
	int bytes_read = 0;
	link_info_t remote_info;
	thread_data* data = NULL;

	if(!dataptr)
	{
		log_print("Thread data pointer is empty!\n");
		pthread_exit((void *)-1);
	}

	data = (thread_data*)dataptr;

	if(!data->ctx || data->idx >= TEST_CHANNEL_COUNT ||  data->idx < 0)
	{
		log_print("Thread data pointer contains invalid data!\n");
		pthread_exit((void *)-1);
	}

	log_print("Start listening test channel 0x%X...\n", test_channels[data->idx]);
	sock = do_listen(data->ctx, test_channels[data->idx]);

	if(sock < 0)
	{
		log_print("Test channel listening failed!\n");
		goto error;
	}

	client = do_accept(data->ctx, sock);
	if(client < 0)
	{
		log_print("Accepting client connection failed!\n");
		goto error;
	}

	/* receive one link information data package at time */
	pthread_mutex_lock( &accept_mutex );

	bytes_read = recv(client, &remote_info, sizeof(remote_info), 0);
	if( bytes_read > 0 )
	{
		char end_mark[] = "END";

		log_print("Remote link information data received\n");
		link_info_t* local_info = hci_get_link_info(data->ctx, 0);

		if(!local_info)
		{
			log_print("Cannot read link information data locally!\n");
			data->result=-1;
		}

		log_print("Compare remote link information data with local one...\n");
		dump_link_information_data(local_info, 0);
		dump_link_information_data(&remote_info, 1);

		if(!compare_link_info_data(&remote_info, local_info))
		{
			log_print("Information comparison success\n");
			data->result = 0;
		}
		else
		{
			log_print("Information comparison failed\n");
			data->result = -1;
		}
		/* send end of communication mark to other DUT */
		send(client, &end_mark, sizeof(end_mark), 0);
	}
	else
	{
		log_print("Cannot read remote link information data\n");
		data->result = -1;
	}
	pthread_mutex_unlock( &accept_mutex );

	close(client);
	close(sock);

	pthread_exit(NULL);
	return 0;

error:
	data->result=-1;

	if(client != -1) close(client);
	if(sock != -1) close(sock);

	pthread_exit((void *)-1);

}

/**
 * Thread function that connects to other DUT using one test channel.
 * After the connection is established, the link information data package
 * is sent and the thread waits end of communication mark from other DUT.
 * The result (0 = success / -1 = failure ) is passed to caller in
 * result field of the thread_data structure
 */
void *send_link_info_thread(void *dataptr)
{
	int sock = -1;
	struct sockaddr_l2 addr;
	thread_data* data = NULL;

	if(!dataptr)
	{
		log_print("Thread data pointer is empty!\n");
		pthread_exit((void *)-1);
	}

	data = (thread_data*)dataptr;

	if(!data->ctx || data->idx >= TEST_CHANNEL_COUNT ||  data->idx < 0)
	{
		log_print("Thread data pointer contains invalid data!\n");
		pthread_exit((void *)-1);
	}

	memset(&addr, 0, sizeof(addr));
	sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

	if(sock < 0)
	{
		log_print("Cannot create socket\n");
		goto error;
	}

	log_print("Socket:%d created\n", sock);

	addr.l2_family = AF_BLUETOOTH;
	addr.l2_psm =  htobs(test_channels[data->idx]);
	addr.l2_bdaddr = data->ctx->remote_mac;

	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		log_print("Connection to remote address failed!\n");
		goto error;
	}

	/* send one link information data package at time */
	pthread_mutex_lock( &connect_mutex );

	link_info_t *info = hci_get_link_info(data->ctx, 1);
	if(!info)
	{
		log_print("Cannot read link information locally!\n");
		data->result=-1;
	}
	else
	{
		int written = 0;

		dump_link_information_data(info, 0);

		log_print("Write link information data to socket...\n");
		written = write(sock, info, sizeof(*info));

		if(written > 0)
		{
			char end_mark[4] = {0}; /* "END" */

			log_print("%d bytes written\n", written);

			/* wait for end of communication mark */
			recv(sock, end_mark, 3, 0);
			log_print("%s\n\n", &end_mark);
			data->result = 0;
		}
		else
		{
			log_print("Writing to socket failed!\n");
			data->result = -1;
		}
	}

	pthread_mutex_unlock( &connect_mutex );

	close(sock);

	pthread_exit(NULL);
	return 0;

error:
	data->result=-1;

	if(sock != -1) close(sock);

	pthread_exit((void *)-1);
}
