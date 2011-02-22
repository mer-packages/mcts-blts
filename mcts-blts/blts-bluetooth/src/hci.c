/* hci.c -- HCI specific functionality

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
#include <blts_log.h>

#include "bt_ctx.h"
#include "hci.h"
#include "ext_tools.h"

char *acl_test_data = "BLTS-BLUETOOTH-TESTCASE";
char test_device_name[] = "BLTS-TEST-DEVICE-NAME";

#define ACL_DATA_HDR_SIZE 5 /* header size of an HCI ACL data packet */
#define BLTS_TEST_DEV_CLASS_0	0x00
#define BLTS_TEST_DEV_CLASS_1	0x01
#define BLTS_TEST_DEV_CLASS_2	0x02

#define READ_HCI_SOCKET_TIMEOUT 30
/* -------------------------------------------------------------------------- */

int audit_incoming_connection_trace(struct hcidump_trace *trace_event, int *audit_passed);

/* -------------------------------------------------------------------------- */

/**
 * Reserves system Bluetooth resources and enables page and inquiry scans
 * Return 0 on success, -errno on fail.
 */
int hci_init_controller(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int fd = -1;
	struct hci_dev_req dr;

	if((ctx->dev_id = hci_get_route(&ctx->local_mac)) < 0)
	{
		errno = ENODEV;
		BLTS_LOGGED_PERROR("No route to remote device");
		return -errno;
	}

	if((fd = hci_open_dev(ctx->dev_id)) < 0)
	{
		BLTS_LOGGED_PERROR("hci_open_dev failed");
		return -errno;
	}

	dr.dev_id  = ctx->dev_id;
	dr.dev_opt = SCAN_PAGE | SCAN_INQUIRY;

	if (ioctl(fd, HCISETSCAN, (unsigned long) &dr) < 0)
	{
		BLTS_LOGGED_PERROR("Can't set scan mode");
		hci_close_dev(fd);
		return -errno;
	}

	ctx->hci_fd = fd;

	return 0;
}

/**
 * Changes local name of device temporarily and waits until other DUT is connected
 * Return 0 on success, -errno on fail.
 */
int do_change_name(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int retval = 0;
	char name[248];

	if(hci_init_controller(ctx) < 0)
	{
		BLTS_DEBUG("HCI Initialization failed!\n");
		return -1;
	}

	/* Read and store initial local name */
	if (hci_read_local_name(ctx->hci_fd, sizeof(name), name, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't read local name");
		retval=-errno;
		goto cleanup;
	}
	BLTS_DEBUG("Initial local name: %s\n", &name);

	/* Replace local name with test value */
	if (hci_write_local_name(ctx->hci_fd, (char*)&test_device_name, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't change local name to test value");
		retval=-errno;
		goto cleanup;
	}
	BLTS_DEBUG("Local name replaced with test value\n");

	/* Wait here until other DUT is verified changed value */
	retval = hci_receive_conn_request(ctx);

	BLTS_DEBUG("Set back original local name...\n");
	if (hci_write_local_name(ctx->hci_fd, (char*)&name, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't change local name back to original");
		retval=-errno;
	}

cleanup:

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

/**
 * Changes class of device temporarily and waits until other DUT is connected
 * Return 0 on success, -errno on fail.
 */
int do_change_class(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int retval = 0;
	uint8_t cls[3];
	uint32_t changed_cod = 0;
	uint32_t original_cod = 0;

	if(hci_init_controller(ctx) < 0)
	{
		BLTS_DEBUG("HCI Initialization failed!\n");
		return -1;
	}

	/* Read and store initial class of device */
	if (hci_read_class_of_dev(ctx->hci_fd, cls, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't read class of device");
		retval=-errno;
		goto cleanup;
	}


	changed_cod = 	(BLTS_TEST_DEV_CLASS_2  << 16) |
					(BLTS_TEST_DEV_CLASS_1  <<  8) |
					(BLTS_TEST_DEV_CLASS_0         ) ;

	/* Replace class of device with test value */
	if (hci_write_class_of_dev(ctx->hci_fd,	changed_cod, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't change class informations");
		retval = errno?-errno:-1;
		goto cleanup;
	}

	/* Wait here until other DUT is verified changed value */
	retval = hci_receive_conn_request(ctx);


	original_cod = 	(cls[2]  << 16)  |
					(cls[1]   <<  8) |
					(cls[0]          ) ;

	BLTS_DEBUG("set back original class of device...\n");
	if (hci_write_class_of_dev(ctx->hci_fd, original_cod, 1000) < 0)
	{
		BLTS_LOGGED_PERROR("Can't change class informations ");
		retval = errno?-errno:-1;
	}

cleanup:

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}


/**
 * Run BT scan according to ctx data. Return 0 on success, -errno on fail.
 * If all other pointers given, return scan info there.
 */
int do_scan(struct bt_ctx *ctx, int *n_responses, inquiry_info *responses, char **names)
{
	if(!ctx) return -EINVAL;

	int want_results = 0;
	if(n_responses && responses && names) want_results = 1;

	int retval = 0;
	int sock = -1;

	int req_flags, req_max_time;

	inquiry_info *resps = 0;
	int n_resps = -1, max_resps = 255;

	char *scan_addr=0, *scan_name=0, **resolved=0;

	/* use first available adapter */
	BLTS_DEBUG("Trying to get device... ");
	if((ctx->dev_id = hci_get_route(&ctx->local_mac)) < 0)
	{
		BLTS_LOGGED_PERROR("hci_get_route failed");
		retval = errno? -errno : -1 ;
		goto cleanup;
	}

	BLTS_DEBUG("got #%d.\nOpening socket...",ctx->dev_id);

	if((sock = hci_open_dev(ctx->dev_id)) < 0)
	{
		BLTS_LOGGED_PERROR("hci_open_dev failed");
		retval = -1;
		goto cleanup;
	}
	BLTS_DEBUG("ok.\nStarting scan...");
	/* Don't return devices not currently in range */
	req_flags = IREQ_CACHE_FLUSH;

	/* Use max 8*1.28 = 10.24 seconds (this is supposed to be standard) */
	req_max_time = 8;

	resps = malloc(max_resps * sizeof(resps));

	/* now, do the scan */
	if((n_resps = hci_inquiry(ctx->dev_id, req_max_time, max_resps,
				  (void*) 0, &resps, req_flags)) < 0)
	{
		BLTS_LOGGED_PERROR("hci_inquiry failed");
		retval = errno?-errno:-1;
		goto cleanup;
	}

	BLTS_DEBUG("got %d responses.\nTrying to read names...\n",n_resps);

	/* now, resolve the scanned names */
	int i;
	scan_addr = malloc(19); /* 'xx:xx:xx:xx:xx:xx' + 2 */
	size_t scan_name_sz = 256;
	scan_name = malloc(scan_name_sz);

	if(want_results)
	{
		resolved = malloc(n_resps);
	}

	int n_unknowns = 0;

	for(i = 0; i < n_resps; ++i)
	{
		ba2str(&(resps+i)->bdaddr, scan_addr);
		memset(scan_name, 0, scan_name_sz);
		errno = 0;
		if(hci_read_remote_name(sock, &(resps+i)->bdaddr,
					scan_name_sz, scan_name, 0) < 0)
		{
			if(errno)
			{
				if(errno != EIO)
				{
					BLTS_LOGGED_PERROR("hci_read_remote_name failure (real error)");
					retval=-errno;
				}
				else
				{
					BLTS_LOGGED_PERROR("hci_read_remote_name failure (no name sent by remote end?)");
				}
				/* continue anyway */
			}
			n_unknowns++;
		}
		BLTS_DEBUG("  %s - %s\n",scan_addr, scan_name?scan_name:"[unknown]");
		if(want_results)
			resolved[i] = scan_name ? strndup(scan_name,scan_name_sz) : (void*)0;
	}
	free(scan_addr); free(scan_name);

	if(want_results)
	{
		*n_responses = n_resps;
		responses = resps;
		names = resolved;
	}

	BLTS_DEBUG("Scan done.\n");
	/* heuristic: assume most devices report a name when asked */
	if(n_resps > 3 && n_unknowns > (n_resps / 2))
	{
		BLTS_DEBUG("Warning: many addresses could not be resolved\n");
	}


cleanup:

	if((sock>=0) && (close(sock) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	if(resps) free(resps);

	return retval;
}

/**
 * Reset BT device according to ctx data. Return 0 on success, -errno on fail.
 */
int do_reset(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int sk;

	BLTS_DEBUG("Trying to reset device...\n");

	if ((sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0)
	{
	BLTS_DEBUG("%s: %s", __FUNCTION__, strerror(errno));
		goto error;
	}

	BLTS_DEBUG("HCIDEVRESET...\n");
	if(ioctl(sk, HCIDEVRESET, ctx->dev_id) < 0 )
	{
	BLTS_DEBUG("%s: unable to reset device %d down: %s",
		__FUNCTION__, ctx->dev_id, strerror(errno));
		goto error;
	}

	BLTS_DEBUG("HCIDEVDOWN...\n");
	if(ioctl(sk, HCIDEVDOWN, ctx->dev_id) < 0 )
	{
		if (errno != EALREADY)
		{
			BLTS_DEBUG("%s: unable to bring device %d down: %s",
				__FUNCTION__, ctx->dev_id, strerror(errno));
			goto error;
		}
	}

	BLTS_DEBUG("HCIDEVUP...\n");
	if(ioctl(sk, HCIDEVUP, ctx->dev_id) < 0)
	{
		if (errno != EALREADY)
		{
			BLTS_DEBUG("%s(): unable to bring device %d up: %s",
				__FUNCTION__, ctx->dev_id, strerror(errno));
			goto error;
		}
	}

	close(sk);

	return 0;

error:
	close(sk);

	return -errno;
}


/**
 * Use HCI commands to establish connection with a remote device
 */
int hci_connect_remote(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	if((ctx->dev_id = hci_get_route(&ctx->remote_mac)) < 0)
	{
		errno = ENODEV;
		BLTS_LOGGED_PERROR("No route to remote device");
		return -errno;
	}

	int fd;
	if((fd = hci_open_dev(ctx->dev_id)) < 0)
	{
		BLTS_LOGGED_PERROR("HCI device open failed");
		return -errno;
	}

	uint16_t handle = -1;

	/* allow role switch */
	uint8_t rswitch = 0x01;

	uint16_t clkoffset = 0x0;
	/* any packet type */
	uint16_t ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;

	int timeout = 10000; /* ms */

	if(hci_create_connection(fd, &ctx->remote_mac, htobs(ptype),
				 htobs(clkoffset), rswitch, &handle, timeout) < 0)
	{
		BLTS_LOGGED_PERROR("HCI create connection failed");
		return -errno;
	}

	ctx->hci_fd = fd;
	ctx->conn_handle = handle;

	/* Leave the device open here, for additional commands.
	   Use hci_disconnect_remote to close. */

	return 0;
}

/*
 * Wait for incoming HCI connect, audit command/event sequence
 */
int hci_audit_incoming_connect(struct bt_ctx *ctx)
{
	int ret, audit_passed = 0;
	char *audit_dump = "/var/log/tests/audit_incoming_connect.hcidump";
	if (!ctx)
		return -EINVAL;

	BLTS_DEBUG("Starting packet capture...");

	int timeout = ctx->test_timeout.tv_sec +
		(int) (0.5 + 1E-9*(double)(ctx->test_timeout.tv_nsec));

	if (start_hcidump(audit_dump, 0) < 0) {
		BLTS_DEBUG("Cannot capture packets, not running test.\n");
		return -1;
	}
	sleep(timeout);
	if (stop_hcidump() < 0) {
		BLTS_DEBUG("Error stopping hcidump, can't continue.\n");
		return -1;
	}

	BLTS_DEBUG("Auditing trace:\n");
	ret = parse_hcidump(audit_dump,
		(hcidump_trace_handler) audit_incoming_connection_trace, &audit_passed);
	BLTS_DEBUG("Audit done.\n");
	return ret?ret : !audit_passed;
}

/**
 * Use HCI commands to disconnect an existing connection with a remote device
 */
int hci_disconnect_remote(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;
	if(ctx->hci_fd < 0) return -EINVAL;
	if(ctx->conn_handle >= 0x0eff) return -EINVAL; /* <- see HCI spec */

	uint8_t reason = HCI_OE_USER_ENDED_CONNECTION;
	int timeout = 10000; /* ms */

	int retval = 0;
	if(hci_disconnect(ctx->hci_fd, ctx->conn_handle, reason, timeout) < 0)
	{
		retval = -errno;
		BLTS_LOGGED_PERROR("Cannot disconnect link");
	}

	ctx->conn_handle = 0xffff;
	hci_close_dev(ctx->hci_fd);
	ctx->hci_fd = -1;

	return retval;
}

/**
 * Verifies that class of the other DUT is really changed and makes a
 * quick connection to other DUT to wake it up
 * Return 0 on success, -errno on fail.
 */
int hci_verify_class_change(struct bt_ctx *ctx)
{
	int i = 0;
	int retval = 0;
	int n_resps = -1;
	inquiry_info *resps = NULL;

	if(hci_init_controller(ctx) < 0)
	{
		BLTS_DEBUG("HCI Initialization failed!\n");
		return -1;
	}

	resps = malloc(255 * sizeof(resps));

	if((n_resps = hci_inquiry(ctx->dev_id, 8, 255,
				  (void*) 0, &resps, IREQ_CACHE_FLUSH)) <= 0)
	{
		BLTS_LOGGED_PERROR("hci_inquiry failed");
		retval = errno?-errno:-1;
		goto cleanup;
	}

	for(i = 0; i < n_resps; i++)
	{
		errno = ENOENT; retval=-1;
		if (((resps+i)->dev_class[0] == BLTS_TEST_DEV_CLASS_0) &&
			((resps+i)->dev_class[1] == BLTS_TEST_DEV_CLASS_1) &&
			((resps+i)->dev_class[2] == BLTS_TEST_DEV_CLASS_2))
		{
			if(memcmp(&(resps+i)->bdaddr, &ctx->remote_mac, sizeof(bdaddr_t)) == 0)
			{
				/* Wake up other DUT ...*/
				uint16_t handle = -1;
				uint8_t rswitch = 0x01; /* allow role switch */
				uint16_t clkoffset = 0x0;
				int timeout = 25000; /* ms */
				uint16_t ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5; /* any packet type */

				if( (retval = hci_create_connection(ctx->hci_fd, &(resps+i)->bdaddr, htobs(ptype),
					htobs(clkoffset), rswitch, &handle, timeout)) < 0)
				{
					BLTS_LOGGED_PERROR("HCI create connection failed");
					break;
				}
				sleep(WAIT_TIME_CONNECT_DISCONNECT);
				retval = hci_disconnect(ctx->hci_fd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
				break;
			}
		}
	}

cleanup:

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	if(resps) free(resps);

	return retval;
}

/**
 * Verifies that local name of the other DUT is really changed and makes a
 * quick connection to other DUT to wake it up
 * Return 0 on success, -errno on fail.
 */
int hci_verify_name_change(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	char name[248];
	int retval = 0;

	char addr[19] = { 0 };

	if(hci_init_controller(ctx) < 0)
	{
		BLTS_DEBUG("HCI Initialization failed!\n");
		return -1;
	}

	ba2str(&ctx->remote_mac, addr);
	BLTS_DEBUG("Read remote name from: %s\n", &addr);

	if (hci_read_remote_name(ctx->hci_fd, &ctx->remote_mac, sizeof(name), name, 50000) < 0)
	{
		BLTS_LOGGED_PERROR("Cannot read remote name");
		retval = errno?-errno:-1;
		goto cleanup;
	}

	BLTS_DEBUG("Expected Name: %s <-> Device Name: %s\n", &test_device_name, &name);
	if (strcmp((char*)&test_device_name, (char*)&name) != 0)
	{
		errno = ENOENT;
		BLTS_LOGGED_PERROR("Device name is not expected");
		retval = errno?-errno:-1;
	}
	else
	{
		/* Wake up other DUT ...*/
		uint16_t handle = -1;
		uint8_t rswitch = 0x01; /* allow role switch */
		uint16_t clkoffset = 0x0;
		int timeout = 25000; /* ms */
		uint16_t ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5; /* any packet type */

		if( (retval = hci_create_connection(ctx->hci_fd, &ctx->remote_mac, htobs(ptype),
					htobs(clkoffset), rswitch, &handle, timeout)) < 0)
		{
			BLTS_LOGGED_PERROR("HCI create connection failed");
			goto cleanup;
		}

		sleep(WAIT_TIME_CONNECT_DISCONNECT);
		retval = hci_disconnect(ctx->hci_fd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
	}

cleanup:

	if((ctx->hci_fd>=0) && (hci_close_dev(ctx->hci_fd) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}

struct acl_test_packet {
	uint8_t type;
	hci_acl_hdr hdr;
	uint8_t data[HCI_MAX_ACL_SIZE];
} __attribute__((packed));
/**
 * Sends ACL data packet down to the hci controller
 */
int hci_transfer_acl_data(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int retval = -1;
	int written = 0;
	size_t dlen = strlen(acl_test_data);

	if (!hci_connect_remote(ctx))
	{
		struct acl_test_packet p;
		memset(&p, 0, sizeof(struct acl_test_packet));

		p.type = HCI_ACLDATA_PKT;
		p.hdr.handle = htobs(acl_handle_pack(ctx->conn_handle, ACL_START));
		p.hdr.dlen = htobs(dlen);
		memcpy(p.data, acl_test_data, dlen);

		written = write(ctx->hci_fd, &p, 1 + HCI_ACL_HDR_SIZE + dlen);

		sleep(WAIT_TIME_CONNECT_DISCONNECT);

		retval = hci_disconnect_remote(ctx);

		if(written <= 0)
		{
			BLTS_LOGGED_PERROR("Transfer acl data failed");
			retval = -1;
		}
 	}

	return retval;
}

/**
 * Open hci socket and waits until ACL data packet arrives
 */
int hci_receive_acl_data(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int sock = -1;
	int retval = 0;
	struct sockaddr_hci addr;

	/* NULL to hci_get_route will retrieve the resource number of the
	   first available Bluetooth adapter */
	if((ctx->dev_id = hci_get_route(BDADDR_ANY)) < 0)
	{
		errno = ENODEV;
		BLTS_LOGGED_PERROR("No route to remote device");
		return -errno;
	}

	if((sock = open_hci_socket(ctx->dev_id)) < 0)
	{
		BLTS_LOGGED_PERROR("open_hci_socket failed");
		retval = -1;
		goto cleanup;
	}

	if(filter_acl_data(sock) < 0)
	{
		BLTS_LOGGED_PERROR("filter_acl_data failed");
		retval = -1;
		goto cleanup;
	}

	/* Bind socket to the HCI device */
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = ctx->dev_id;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		BLTS_LOGGED_PERROR("open_hci_socket failed");
		retval = -1;
		goto cleanup;
	}

	retval = read_hci_socket(sock, HCI_MAX_FRAME_SIZE, 1, (read_hci_data_callback) verify_acl_test_data);

cleanup:

	if((sock>=0) && (close(sock) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}




/* -------------------------------------------------------------------------- */
/* --> Helper code that really should be in libbluetooth but isn't */


int hci_read_automatic_flush_timeout(int dd, uint16_t handle, uint16_t *timeout, int to)
{
	read_automatic_flush_timeout_cp cp;
	read_automatic_flush_timeout_rp rp;
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp.handle = handle;

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_HOST_CTL;
	rq.ocf = OCF_READ_AUTOMATIC_FLUSH_TIMEOUT;
	rq.cparam = &cp;
	rq.clen = READ_AUTOMATIC_FLUSH_TIMEOUT_CP_SIZE;
	rq.rparam = &rp;
	rq.rlen = READ_AUTOMATIC_FLUSH_TIMEOUT_RP_SIZE;
	rq.event = EVT_CMD_COMPLETE;

	if(hci_send_req(dd, &rq, to) < 0)
		return -1;

	if(rp.status)
	{
		errno = bt_error(rp.status);
		return -1;
	}

	*timeout = rp.timeout;
	return 0;
}

int hci_write_automatic_flush_timeout(int dd, uint16_t handle, uint16_t timeout, int to)
{
	write_automatic_flush_timeout_cp cp;
	write_automatic_flush_timeout_rp rp;
	struct hci_request rq;

	memset(&cp, 0, sizeof(cp));
	cp.handle = handle;
	cp.timeout = timeout;

	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_HOST_CTL;
	rq.ocf = OCF_WRITE_AUTOMATIC_FLUSH_TIMEOUT;
	rq.cparam = &cp;
	rq.clen = WRITE_AUTOMATIC_FLUSH_TIMEOUT_CP_SIZE;
	rq.rparam = &rp;
	rq.rlen = WRITE_AUTOMATIC_FLUSH_TIMEOUT_RP_SIZE;
	rq.event = EVT_CMD_COMPLETE;

	if(hci_send_req(dd, &rq, to) < 0)
		return -1;

	if(rp.status)
	{
		errno = bt_error(rp.status);
		return -1;
	}

	return 0;
}


/**
 * Receive messages from hci socket and call handler function
 */
int read_hci_socket(int sock, int buf_size, int count, read_hci_data_callback call_function)
{
	struct cmsghdr *cmsg;
	struct msghdr msg;
	struct iovec  iv;
	struct frame frm;
	char *buf = NULL;
	char *ctrl = NULL;
	int retval = -1;

	if (!(buf = malloc(buf_size)))
	{
		BLTS_LOGGED_PERROR("Can't allocate data buffer");
		retval = -1;
		goto cleanup;
	}

	frm.data = buf;

	if (!(ctrl = malloc(100)))
	{
		BLTS_LOGGED_PERROR("Can't allocate control buffer");
		retval = -1;
		goto cleanup;
	}

	memset(&msg, 0, sizeof(msg));

	while (count--)
	{
		int round=0;
		int ret=-1;

		iv.iov_base = frm.data;
		iv.iov_len  = buf_size;

		/* Set msg struct */
		msg.msg_iov = &iv;
		msg.msg_iovlen = 1;
		msg.msg_control = ctrl;
		msg.msg_controllen = 100;

		BLTS_DEBUG("Wait messages from hci socket (~ %d sec)...\n", READ_HCI_SOCKET_TIMEOUT);

		do {
			ret = recvmsg(sock, &msg, MSG_PEEK | MSG_DONTWAIT);
			sleep(1); /* wait 1 sec */
			round++;
			BLTS_DEBUG("%d-", round);
		}while(ret < 0 && round < READ_HCI_SOCKET_TIMEOUT);

		if(ret<0)
		{
			BLTS_DEBUG("Read HCI socket timeout.\n");
			retval = -1;
			goto cleanup;
		}

		if ((frm.data_len = recvmsg(sock, &msg, 0)) < 0)
		{
			BLTS_LOGGED_PERROR("Receive failed");
			retval = -1;
			goto cleanup;
		}

		/* Process control message */
		frm.in = 0;
		cmsg = CMSG_FIRSTHDR(&msg);

		while (cmsg)
		{
			switch (cmsg->cmsg_type)
			{
				case HCI_CMSG_DIR:
					frm.in = *((int *)CMSG_DATA(cmsg));
					break;
				case HCI_CMSG_TSTAMP:
					frm.ts = *((struct timeval *)CMSG_DATA(cmsg));
					break;
			}
			cmsg = CMSG_NXTHDR(&msg, cmsg);
		}

		frm.ptr = frm.data;
		frm.len = frm.data_len;

		if(call_function)
			retval = call_function(&frm);
		else
			retval = 0;

		BLTS_DEBUG("retval: %d \n", retval);

		if(retval < 0)
			break;
	}

cleanup:
	if(buf) free(buf);
	if(ctrl) free(ctrl);

	return retval;
}

/**
 * Open hci socket and set filter that accepts all ptypes and events
 */
int open_hci_socket(int dev)
{
	struct hci_filter flt;
	struct hci_dev_info di;
	int sk, dd, opt;

	dd = hci_open_dev(dev);
	if (dd < 0)
	{
		BLTS_LOGGED_PERROR("Can't open device");
		return -1;
	}

	if (hci_devinfo(dev, &di) < 0)
	{
		BLTS_LOGGED_PERROR("Can't get device info");
		return -1;
	}

	opt = hci_test_bit(HCI_RAW, &di.flags);

	if (ioctl(dd, HCISETRAW, opt) < 0)
	{
		if (errno == EACCES)
		{
			BLTS_LOGGED_PERROR("Can't access device");
			return -1;
		}
	}

	hci_close_dev(dd);

	/* Create HCI socket */
	sk = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (sk < 0)
	{
		BLTS_LOGGED_PERROR("Can't create raw socket");
		return -1;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_DATA_DIR, &opt, sizeof(opt)) < 0)
	{
		BLTS_LOGGED_PERROR("Can't enable data direction info");
		return -1;
	}

	opt = 1;
	if (setsockopt(sk, SOL_HCI, HCI_TIME_STAMP, &opt, sizeof(opt)) < 0)
	{
		BLTS_LOGGED_PERROR("Can't enable time stamp");
		return -1;
	}

	/* Setup filter */
	hci_filter_clear(&flt);
	hci_filter_all_ptypes(&flt);
	hci_filter_all_events(&flt);
	if (setsockopt(sk, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
	{
		BLTS_LOGGED_PERROR("Can't set filter");
		return -1;
	}
	return sk;
}

/**
 * Verify that ACL data packet contains hard coded acl_test_data string
 */
int verify_acl_test_data(struct frame *frm)
{
	char *buf = NULL;
	int len = 0;
	int result = -1;

	if(!frm)
	{
		BLTS_DEBUG("NULL data packet received...\n");
		return -1;
	}

	frm->ptr += HCI_ACL_HDR_SIZE;
	frm->len -= HCI_ACL_HDR_SIZE;

	buf = frm->ptr;
	len = frm->len;

	if(!len || !buf)
		return -1;

	BLTS_DEBUG("len %d - buf:%s\n", len , buf+1);

	result = strncmp(buf+1, acl_test_data, len-1);

	return result;
}

/**
 * Filter out all packets except ACL data packets
 */
int filter_acl_data(int sock)
{
	struct hci_filter flt;

	BLTS_DEBUG("Set filter to accept only ACL data packets\n");

	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_ACLDATA_PKT, &flt);

	if(setsockopt(sock, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
	{
		BLTS_LOGGED_PERROR("setsockopt failed");
		return -1;
	}

	return 0;
}

/*
 * This gets called once for each packet in the trace
 */
int audit_incoming_connection_trace(struct hcidump_trace *trace_event, int *audit_passed)
{
	struct {
		uint8_t low;
		uint8_t high;
	} sequence[] = {
		{ 0x04, 0x04 }, /* connection request event */
		{ 0x01, 0x09 }, /* accept connection request command */
		{ 0x04, 0x0f }, /* command status event */
		{ 0x04, 0x03 }, /* connect complete event */
	};

	static unsigned next_expected = 0;
	unsigned steps = 4;
	static double start_time;

	if (next_expected >= steps)
		return 0;

/* 	BLTS_DEBUG("%.2x %.2x, ",trace_event->data[0],trace_event->data[1]); */
	if (trace_event->data[0] == sequence[next_expected].low &&
		trace_event->data[1] == sequence[next_expected].high) {
/* 		BLTS_DEBUG("step %d ok.\n",next_expected+1); */
		if (next_expected == 0) {
			start_time = trace_event->tv_sec
				+ 1E-6*(double)(trace_event->tv_usec);
			BLTS_DEBUG("Found connection sequence start at t = %lf\n", start_time);
		}
		next_expected++;

		if (next_expected == steps) {
			double end_time = trace_event->tv_sec + 1E-6*(double)(trace_event->tv_usec);
			BLTS_DEBUG("Sequence complete after %d steps at t = %lf\n", steps, end_time);
			BLTS_DEBUG("Latency = %lf s\n", end_time-start_time);
			*audit_passed = 1;
		}
	}

	return 0;
}

/**
 * Filter out all packets except defined event packets
 */
int filter_events(int sock, int *events)
{
	int i;
	struct hci_filter flt;

	BLTS_DEBUG("Set filter to accept only predefined events\n");

	hci_filter_clear(&flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);

	for (i = 0; events[i] != 0; i++)
	{
		hci_filter_set_event(events[i], &flt);
	}

	if (setsockopt(sock, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
	{
		BLTS_LOGGED_PERROR("setsockopt failed");
		return -1;
	}

	return 0;
}

/**
 * Set filter to accept only connection requests and read hci socket
 * until connection request is received
 */
int hci_receive_conn_request(struct bt_ctx *ctx)
{
	if(!ctx) return -EINVAL;

	int sock = -1;
	int retval = 0;
	struct sockaddr_hci addr;

	if((sock = open_hci_socket(ctx->dev_id)) < 0)
	{
		BLTS_LOGGED_PERROR("open_hci_socket failed");
		retval = -1;
		goto cleanup;
	}

	int events[] = { EVT_CONN_REQUEST, 0 };

	if(filter_events(sock, events) < 0)
	{
		BLTS_LOGGED_PERROR("filter_events failed");
		retval = -1;
		goto cleanup;
	}

	/* Bind socket to the HCI device */
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = ctx->dev_id;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		BLTS_LOGGED_PERROR("open_hci_socket failed");
		retval = -1;
		goto cleanup;
	}

	retval = read_hci_socket(sock, HCI_MAX_FRAME_SIZE, 1, NULL);

cleanup:

	if((sock>=0) && (close(sock) < 0))
	{
		BLTS_LOGGED_PERROR("socket close failed");
		retval = errno?-errno:-1;
	}

	return retval;
}


/**
 * Collect version and feature information about remote device.
 * Return allocated device_info_t information structure or NULL
 * if data cannot be collected
 */
device_info_t* hci_get_info_remote(struct bt_ctx *ctx)
{
	if(!ctx) return NULL;

	uint16_t handle = -1;
	struct hci_dev_info di;
	int connected = 0;

	/* do memory allocations here*/
	device_info_t* infoptr = malloc(sizeof(device_info_t));

	if (NULL == infoptr)
		return NULL;

	memset(infoptr, 0, sizeof(device_info_t));
	struct hci_conn_info_req *cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));

	if (!infoptr || !cr )
	{
		BLTS_DEBUG("%s: Cannot allocate memory for connection info", __FUNCTION__);
		goto error;
	}

	BLTS_DEBUG("Requesting remote information ...\n");

	if(ctx->hci_fd < 0)
	{
		ctx->hci_fd = hci_open_dev(ctx->dev_id);
		if (ctx->hci_fd < 0)
		{
			BLTS_DEBUG("%s: HCI device open failed", __FUNCTION__);
			goto error;
		}
	}

	di.dev_id = ctx->dev_id;

	if (ioctl(ctx->hci_fd, HCIGETDEVINFO, (void *) &di))
	{
		BLTS_DEBUG("%s: Unable to get device info %d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
		goto error;
	}

	bacpy(&cr->bdaddr, &ctx->remote_mac);
	cr->type = ACL_LINK;

	if (hci_create_connection(ctx->hci_fd, &ctx->remote_mac,
		htobs(di.pkt_type & ACL_PTYPE_MASK),
		0, 0x01, &handle, 25000) < 0)
	{
		BLTS_DEBUG("%s: Cannot create connection\n", __FUNCTION__);
		goto error;
	}
	connected = 1;

	if (hci_read_remote_version(ctx->hci_fd, handle, &(infoptr->version), 20000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read version info for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	infoptr->got_version=1;

	if (hci_read_remote_features(ctx->hci_fd, handle, infoptr->features, 20000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read features for hci%d: %s",
		__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	infoptr->got_features=1;

	if ((di.features[7] & LMP_EXT_FEAT) && (infoptr->features[7] & LMP_EXT_FEAT))
	{
		int i = 0;
		uint8_t features[8];

		if (hci_read_remote_ext_features(ctx->hci_fd, handle, 0,
				&(infoptr->max_page), features, 20000) < 0)
		{
			BLTS_DEBUG("%s: Cannot read ext features for hci%d: %s",
				__FUNCTION__, ctx->dev_id, strerror(errno));
		}
		else
		{
			infoptr->got_ext_features = 1;
			for (i = 1; i <= infoptr->max_page && i <= MAX_EXT_PAGES_TO_VERIFY ; i++)
			{
				uint8_t max = 0;
				if (hci_read_remote_ext_features(ctx->hci_fd, handle, i, &max, infoptr->ext_features[i-1], 1000) < 0 )
				{
					BLTS_DEBUG("%s: Cannot read ext features for hci%d: %s",
					__FUNCTION__, ctx->dev_id, strerror(errno));
					infoptr->got_ext_features = 0;
					break;
				}
			}
		}
	}

	hci_disconnect(ctx->hci_fd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
	if(cr) free(cr);

	return infoptr;

error:

	if(connected) hci_disconnect(ctx->hci_fd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
	if(infoptr) free(infoptr);
	if(cr) free(cr);
	return NULL;

}

/**
 * Collect version and feature information about local device.
 * Return allocated device_info_t information structure or NULL
 * if data cannot be collected
 */
device_info_t* hci_get_info_local(struct bt_ctx *ctx)
{
	if(!ctx) return NULL;

	struct hci_dev_info di;

	device_info_t* infoptr = malloc(sizeof(device_info_t));

	if (NULL == infoptr)
		return NULL;

	memset(infoptr, 0, sizeof(device_info_t));

	BLTS_DEBUG("Requesting local information ...\n");

	if(ctx->hci_fd < 0)
	{
		ctx->hci_fd = hci_open_dev(ctx->dev_id);
		if (ctx->hci_fd < 0)
		{
			BLTS_DEBUG("%s: HCI device open failed", __FUNCTION__);
			goto error;
		}
	}

	di.dev_id = ctx->dev_id;

	if (ioctl(ctx->hci_fd, HCIGETDEVINFO, (void *) &di))
	{
		BLTS_DEBUG("%s: Unable to get device info %d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
		goto error;
	}

	if (hci_read_local_version(ctx->hci_fd, &(infoptr->version), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read version info for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else infoptr->got_version=1;


	if (hci_read_local_features(ctx->hci_fd, infoptr->features, 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read features for hci%d: %s",
		__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else infoptr->got_features=1;

	if ((di.features[7] & LMP_EXT_FEAT) && (infoptr->features[7] & LMP_EXT_FEAT))
	{
		int i;
		uint8_t features[8];

		if (hci_read_local_ext_features(ctx->hci_fd, 0, &infoptr->max_page,
		features, 20000) < 0)
		{
			BLTS_DEBUG("%s: Cannot read ext features for hci%d: %s",
			__FUNCTION__, ctx->dev_id, strerror(errno));
		}
		else
		{
			infoptr->got_ext_features=1;
			for (i = 1; i <= infoptr->max_page && i <= MAX_EXT_PAGES_TO_VERIFY ; i++)
			{
				uint8_t max=0;
				if (hci_read_local_ext_features(ctx->hci_fd, i, &max, infoptr->ext_features[i-1], 1000) < 0 )
				{
					BLTS_DEBUG("%s: Cannot read ext features for hci%d: %s",
					__FUNCTION__, ctx->dev_id, strerror(errno));
					infoptr->got_ext_features = 0;
					break;
				}

			}
		}

	}
	else
	{
		infoptr->got_ext_features=0;
	}

	return infoptr;

error:

	if(infoptr) free(infoptr);
	return NULL;
}

/**
 * Collect RSSI, link quality, transmit power level, AFH map, clock and clock offset information about connected link
 * Return allocated link_info_t link information structure or NULL if data cannot be collected
 * parameter which indicates which clock is referred (0 = local, 1 = remote)
 */
link_info_t* hci_get_link_info(struct bt_ctx *ctx, int which)
{
	if(!ctx) return NULL;

	uint8_t mode=0;
	uint16_t handle = -1;
	struct hci_dev_info di;

	/* do memory allocations here*/
	link_info_t* linkptr = (link_info_t*)malloc(sizeof(link_info_t));

	if (NULL == linkptr)
		return NULL;

	memset(linkptr, 0, sizeof(link_info_t));
	struct hci_conn_info_req *cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));

	if (!linkptr || !cr )
	{
		BLTS_DEBUG("%s: Cannot allocate memory for connection link info", __FUNCTION__);
		goto error;
	}

	BLTS_DEBUG("Requesting link information ...\n");

	/* We'll use this in the error case */
	if (hci_devinfo(ctx->dev_id, &di) < 0)
	{
		BLTS_DEBUG("$s: Adapter %d failure", __FUNCTION__, ctx->dev_id);
		goto error;
	}


	if(ctx->hci_fd < 0)
	{
		ctx->hci_fd = hci_open_dev(ctx->dev_id);

		if (ctx->hci_fd < 0)
		{
			BLTS_DEBUG("%s: HCI device open failed", __FUNCTION__);
			goto error;
		}
	}

	bacpy(&cr->bdaddr, &ctx->remote_mac);
	cr->type = ACL_LINK;

	if (ioctl(ctx->hci_fd, HCIGETCONNINFO, (unsigned long) cr) < 0)
	{
		if (hci_create_connection(ctx->hci_fd, &ctx->remote_mac,
			htobs(di.pkt_type & ACL_PTYPE_MASK),
			0, 0x01, &handle, 25000) < 0)
		{
			BLTS_DEBUG("%s: Unable to get connection info %d: %s\n",
				__FUNCTION__, ctx->dev_id, strerror(errno));
			goto error;
		}
	}
	else
	{
		handle = htobs(cr->conn_info->handle);
	}

	if (hci_read_rssi(ctx->hci_fd, handle, &(linkptr->rssi), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read RSSI info for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
		linkptr->got_rssi=1;

	if (hci_read_link_quality(ctx->hci_fd, handle, &(linkptr->lq), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read link quality for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
		linkptr->got_lq=1;


	if (hci_read_transmit_power_level(ctx->hci_fd, handle, 0, &(linkptr->level), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read transmit power level for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
		linkptr->got_level = 1;


	if (hci_read_afh_map(ctx->hci_fd, handle, &mode, linkptr->map, 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read AFH map request for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
	{
		if (mode != 0x01)
			linkptr->got_map=-1; /* undefined */
		else
			linkptr->got_map=1;
	}

	if (hci_read_clock(ctx->hci_fd, handle, which, &(linkptr->clock), &(linkptr->accuracy), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read clock for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
		linkptr->got_clock = 1;

	if (hci_read_clock_offset(ctx->hci_fd, handle, &(linkptr->offset), 1000) < 0)
	{
		BLTS_DEBUG("%s: Cannot read clock offset for hci%d: %s\n",
			__FUNCTION__, ctx->dev_id, strerror(errno));
	}
	else
		linkptr->got_offset = 1;

	if(cr) free(cr);

	return linkptr;

error:
	if(linkptr) free(linkptr);
	if(cr) free(cr);
	return NULL;
}


/* -------------------- BT LE ----------------- */

static int print_advertising_devices(int fd)
{
	unsigned char buf[HCI_MAX_EVENT_SIZE];
	struct hci_filter nf, of;
	socklen_t olen = sizeof(of);
	int len, num = 10;
	evt_le_meta_event *meta;
	le_advertising_info *info;
	char addr[18];

	if (getsockopt(fd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		BLTS_DEBUG("Could not get socket options\n");
		return -1;
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
		BLTS_DEBUG("Could not set socket options\n");
		return -1;
	}

	while(num--) {
		while((len = read(fd, buf, sizeof(buf))) < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			BLTS_LOGGED_PERROR("read() failed");
			goto cleanup;
		}

		meta = (void*)(buf + (1 + HCI_EVENT_HDR_SIZE));
		len -= (1 + HCI_EVENT_HDR_SIZE);

		if (meta->subevent != 0x02)
			break;

		info = (le_advertising_info *) (meta->data + 1);
		ba2str(&info->bdaddr, addr);

		BLTS_DEBUG("address: %s (rssi: %u)\n", addr, info->rssi);
	}

cleanup:
	if (setsockopt(fd, SOL_HCI, HCI_FILTER, &of, sizeof(of))) {
		BLTS_DEBUG("Could not set socket options\n");
		return -1;
	}

	return len < 0 ? -1 : 0;
}

int do_le_scan(struct bt_ctx *ctx)
{
	int fd, err = 0;

	BLTS_DEBUG("Trying to get device... ");
	if ((ctx->dev_id = hci_get_route(&ctx->local_mac)) < 0) {
		BLTS_LOGGED_PERROR("hci_get_route failed");
		return -1;
	}

	fd = hci_open_dev(ctx->dev_id);
	if (fd < 0) {
		BLTS_LOGGED_PERROR("Could not open device");
		return -1;
	}

	err = hci_le_set_scan_parameters(fd, 0x01, htobs(0x0010),
		htobs(0x0010), 0x00, 0x00);
	if (err < 0) {
		BLTS_LOGGED_PERROR("Set scan parameters failed");
		goto cleanup;
	}

	err = hci_le_set_scan_enable(fd, 0x01, 0x00);
	if (err < 0) {
		BLTS_LOGGED_PERROR("Enable scan failed");
		goto cleanup;
	}

	BLTS_DEBUG("LE Scan ...\n");

	err = print_advertising_devices(fd);
	if (err < 0) {
		BLTS_ERROR("Could not receive advertising events\n");
		goto cleanup;
	}

	err = hci_le_set_scan_enable(fd, 0x00, 0x00);
	if (err < 0) {
		BLTS_LOGGED_PERROR("Disable scan failed");
		goto cleanup;
	}

	err = 0;

cleanup:
	hci_close_dev(fd);

	return err;
}

int le_set_advertise_mode(struct bt_ctx *ctx, int adv_on)
{
	struct hci_request req;
	le_set_advertise_enable_cp advertise_cp;
	uint8_t status;
	int ret, fd;

	memset(&advertise_cp, 0, sizeof(advertise_cp));
	memset(&req, 0, sizeof(req));

	if (!ctx)
		return -EINVAL;

	if ((ctx->dev_id = hci_get_route(&ctx->remote_mac)) < 0) {
		BLTS_LOGGED_PERROR("No route to remote device");
		return -errno;
	}

	if ((fd = hci_open_dev(ctx->dev_id)) < 0) {
		BLTS_LOGGED_PERROR("HCI device open failed");
		return -errno;
	}

	advertise_cp.enable = adv_on ? 0x01 : 0x00;
	req.ogf = OGF_LE_CTL;
	req.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
	req.cparam = &advertise_cp;
	req.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
	req.rparam = &status;
	req.rlen = 1;

	BLTS_DEBUG("Setting advertise %s\n", adv_on ? "on" : "off");

	ret = hci_send_req(fd, &req, 100);
	if (ret < 0) {
		BLTS_LOGGED_PERROR("Cannot set advertise mode");
		hci_close_dev(fd);
		return ret;
	}

	if (status)
		BLTS_ERROR("Set advertise mode on hci%d returned status %d\n",
			ctx->dev_id, status);

	hci_close_dev(fd);
	return status;
}

int le_connect_remote(struct bt_ctx *ctx)
{
	int err, fd;
	uint16_t interval, latency, max_ce_length, max_interval, min_ce_length;
	uint16_t min_interval, supervision_timeout, window, handle;
	uint8_t initiator_filter, own_bdaddr_type, peer_bdaddr_type;

	if (!ctx)
		return -EINVAL;

	if ((ctx->dev_id = hci_get_route(&ctx->remote_mac)) < 0) {
		BLTS_LOGGED_PERROR("No route to remote device");
		return -errno;
	}

	if ((fd = hci_open_dev(ctx->dev_id)) < 0) {
		BLTS_LOGGED_PERROR("HCI device open failed");
		return -errno;
	}

	peer_bdaddr_type = 0;
	interval = htobs(0x4);
	window = htobs(0x4);
	initiator_filter = 0;
	own_bdaddr_type = 0;
	min_interval = htobs(0xF);
	max_interval = htobs(0xF);
	latency = htobs(0);
	supervision_timeout = htobs(0x0C80);
	min_ce_length = htobs(0x1);
	max_ce_length = htobs(0x1);

	err = hci_le_create_conn(fd, interval, window, initiator_filter,
		peer_bdaddr_type, ctx->remote_mac, own_bdaddr_type, min_interval,
		max_interval, latency, supervision_timeout,
		min_ce_length, max_ce_length, &handle, 25000);
	if (err < 0) {
		BLTS_LOGGED_PERROR("Could not create connection");
		hci_close_dev(fd);
		return err;
	}

	BLTS_DEBUG("Connection handle %d\n", handle);

	ctx->hci_fd = fd;
	ctx->conn_handle = handle;

	return 0;
}

int le_disconnect_remote(struct bt_ctx *ctx)
{
	int err, retval = 0;
	uint8_t reason = HCI_OE_USER_ENDED_CONNECTION;

	if (!ctx || ctx->hci_fd < 0 || ctx->conn_handle >= 0x0eff)
		return -EINVAL;

	err = hci_disconnect(ctx->hci_fd, ctx->conn_handle, reason, 10000);
	if (err < 0) {
		BLTS_LOGGED_PERROR("Cannot disconnect link");
		retval = -errno;
	}

	hci_close_dev(ctx->hci_fd);
	ctx->conn_handle = 0xffff;
	ctx->hci_fd = -1;

	return retval;
}

