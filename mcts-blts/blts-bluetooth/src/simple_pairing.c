/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* simple_pairing.c -- Bluetooth simple pairing tests

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

#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

#include <blts_log.h>

#include "bt_ctx.h"
#include "bt_fute.h"
#include "simple_pairing.h"
#include "bt_fute_agent.h"
#include "hci.h"
#include "l2cap.h"
#include "gen_client.h"

/* The tests require custom data into the context. As in either OOB data
 * or L2CAP connection info (socket id)
 */
#define ROLE_SLAVE  0x01
#define ROLE_MASTER 0x00

typedef struct {
	struct bt_ctx *ctx;

	int l2cap_fd;
	uint8_t role; /* 0x00 for slave 0x01 for master */
} blts_simple_pairing_data;

/* Create a Simple pairing data, out of generic bluetooth context */
static blts_simple_pairing_data *
blts_simple_pairing_data_new (struct bt_ctx *ctx)
{
	blts_simple_pairing_data *data;

	FUNC_ENTER();

	data = malloc (sizeof (blts_simple_pairing_data));
	data->ctx = ctx;
	data->l2cap_fd = -1;

	FUNC_LEAVE();

	return data;
}

/* Free simple pairing data (but not bt_ctx, memory owned by calling module) */
static void
blts_simple_pairing_data_free (blts_simple_pairing_data **data)
{
	if (!data || *data == NULL) return;

	FUNC_ENTER();

	if ((*data)->l2cap_fd > 0) {
		close ((*data)->l2cap_fd);
		(*data)->l2cap_fd = -1;
	}

	free (*data);
	*data = NULL;

	FUNC_LEAVE();
}

/**** FIXME: MOVE TO UTILS */

/* stores last seen connection bdaddr */
bdaddr_t glob_connected_ba;

/* Event interpretation (from Bluez hcidump) */
static char *event_str[] = {
	"Unknown",
	"Inquiry Complete",
	"Inquiry Result",
	"Connect Complete",
	"Connect Request",
	"Disconn Complete",
	"Auth Complete",
	"Remote Name Req Complete",
	"Encrypt Change",
	"Change Connection Link Key Complete",
	"Master Link Key Complete",
	"Read Remote Supported Features",
	"Read Remote Ver Info Complete",
	"QoS Setup Complete",
	"Command Complete",
	"Command Status",
	"Hardware Error",
	"Flush Occurred",
	"Role Change",
	"Number of Completed Packets",
	"Mode Change",
	"Return Link Keys",
	"PIN Code Request",
	"Link Key Request",
	"Link Key Notification",
	"Loopback Command",
	"Data Buffer Overflow",
	"Max Slots Change",
	"Read Clock Offset Complete",
	"Connection Packet Type Changed",
	"QoS Violation",
	"Page Scan Mode Change",
	"Page Scan Repetition Mode Change",
	"Flow Specification Complete",
	"Inquiry Result with RSSI",
	"Read Remote Extended Features",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Synchronous Connect Complete",
	"Synchronous Connect Changed",
	"Sniff Subrate",
	"Extended Inquiry Result",
	"Encryption Key Refresh Complete",
	"IO Capability Request",
	"IO Capability Response",
	"User Confirmation Request",
	"User Passkey Request",
	"Remote OOB Data Request",
	"Simple Pairing Complete",
	"Unknown",
	"Link Supervision Timeout Change",
	"Enhanced Flush Complete",
	"Unknown",
	"User Passkey Notification",
	"Keypress Notification",
	"Remote Host Supported Features Notification",
};

/*
 * Wait for incoming hci traffic on dd according to filter, return message in
 * newly allocated *resp (size = resp_len). Timeout as in poll().
 * Return 0 on success or -errno.
 */
static int wait_for_events(int dd, struct hci_filter *filter,
			unsigned char **resp, int *resp_len, int timeout)
{
	unsigned char *buf;
	struct hci_filter of;
	socklen_t olen;
	int err, retries, len;

	if (!resp)
		return -EINVAL;

	buf = malloc(HCI_MAX_EVENT_SIZE);
	if (!buf)
		return -ENOMEM;

	/* We'll likely want to save the previous filter */
	olen = sizeof(of);
	if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		free(buf);
		return -1;
	}

	if (setsockopt(dd, SOL_HCI, HCI_FILTER, filter, sizeof(*filter)) < 0) {
		free(buf);
		return -1;
	}

	retries = 10;
	while (retries--) {

		if (timeout) {
			struct pollfd p;
			int n;

			p.fd = dd; p.events = POLLIN;
			while ((n = poll(&p, 1, timeout)) < 0) {
				if (errno == EAGAIN || errno == EINTR)
					continue;
				goto failed;
			}

			if (!n) {
				errno = ETIMEDOUT;
				goto failed;
			}

			timeout -= 10;
			if (timeout < 0) timeout = 0;

		}

		while ((len = read(dd, buf, HCI_MAX_EVENT_SIZE)) < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			goto failed;
		}

		goto done;
	}

	errno = ETIMEDOUT;

failed:
	free(buf);
	err = errno;
	setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));
	errno = err;
	return -1;

done:
	*resp = buf;
	*resp_len = len;
	setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));
	return 0;
}

static void set_default_event_filter(struct hci_filter *flt)
{
	hci_filter_clear(flt);
	hci_filter_set_ptype(HCI_EVENT_PKT, flt);
	hci_filter_set_event(EVT_CMD_STATUS, flt);
	hci_filter_set_event(EVT_CMD_COMPLETE, flt);
	hci_filter_set_event(EVT_PIN_CODE_REQ, flt);
	hci_filter_set_event(EVT_LINK_KEY_REQ, flt);
	hci_filter_set_event(EVT_LINK_KEY_NOTIFY, flt);
	hci_filter_set_event(EVT_RETURN_LINK_KEYS, flt);
	hci_filter_set_event(EVT_IO_CAPABILITY_REQUEST, flt);
	hci_filter_set_event(EVT_IO_CAPABILITY_RESPONSE, flt);
	hci_filter_set_event(EVT_USER_CONFIRM_REQUEST, flt);
	hci_filter_set_event(EVT_USER_PASSKEY_REQUEST, flt);
	hci_filter_set_event(EVT_REMOTE_OOB_DATA_REQUEST, flt);
	hci_filter_set_event(EVT_USER_PASSKEY_NOTIFY, flt);
	hci_filter_set_event(EVT_KEYPRESS_NOTIFY, flt);
	hci_filter_set_event(EVT_SIMPLE_PAIRING_COMPLETE, flt);
	hci_filter_set_event(EVT_AUTH_COMPLETE, flt);
	hci_filter_set_event(EVT_REMOTE_NAME_REQ_COMPLETE, flt);
	hci_filter_set_event(EVT_READ_REMOTE_VERSION_COMPLETE, flt);
	hci_filter_set_event(EVT_READ_REMOTE_FEATURES_COMPLETE, flt);
	hci_filter_set_event(EVT_REMOTE_HOST_FEATURES_NOTIFY, flt);
	hci_filter_set_event(EVT_INQUIRY_COMPLETE, flt);
	hci_filter_set_event(EVT_INQUIRY_RESULT, flt);
	hci_filter_set_event(EVT_INQUIRY_RESULT_WITH_RSSI, flt);
	hci_filter_set_event(EVT_EXTENDED_INQUIRY_RESULT, flt);
	hci_filter_set_event(EVT_CONN_REQUEST, flt);
	hci_filter_set_event(EVT_CONN_COMPLETE, flt);
	hci_filter_set_event(EVT_DISCONN_COMPLETE, flt);
}

/*
 * Wait until event occurs on dd or timeout (as in poll()) is reached.
 * If we were waiting for an incoming connection, also store the bdaddr to
 * globals.
 * Return 0 on event, -errno on fail (-ETIMEDOUT on timeout).
 */
static int wait_until_event(int dd, uint8_t event, int timeout)
{
	struct hci_filter nf;
	double t_start,t_end,t_diff;
	struct timeval tv;
	unsigned char *buf = 0;
	int buf_len, ret = 0;
	hci_event_hdr *hdr;

	BLTS_DEBUG("Waiting for event 0x%.2x (\"%s\")\n", event, event_str[event]);

	set_default_event_filter(&nf);

	gettimeofday(&tv, 0);
	t_start = tv.tv_sec + 1E-6 * tv.tv_usec;
	t_end = t_start + 1E-3 * timeout;

	while (1) {
		if (buf) {
			free(buf);
			buf = 0;
		}
		if (wait_for_events(dd, &nf, &buf, &buf_len, timeout) < 0) {
			BLTS_DEBUG("Failed waiting for event\n");
			ret = errno?-errno:-1;
			break;
		}
		gettimeofday(&tv, 0);
		t_diff = t_end - tv.tv_sec - 1E-6 * tv.tv_usec;
		if (t_diff <= 0) {
			BLTS_DEBUG("Timed out waiting for event\n");
			ret = -ETIMEDOUT;
			break;
		}

		timeout = t_diff * 1E3;

		hdr = (void *) (buf + 1);
		BLTS_DEBUG("> Event 0x%.2x (\"%s\") (%lf s)\n",hdr->evt,
			event_str[hdr->evt], t_end - t_diff - t_start);
		if (hdr->evt == event) {
			if (event == EVT_CONN_COMPLETE) {
				evt_conn_complete* params = (HCI_EVENT_HDR_SIZE+1+(void*)buf);
				bacpy(&glob_connected_ba, &(params->bdaddr));
			}
			ret = 1;
			break;
		}
	}

	if (buf)
		free(buf);

	return ret;
}

/* For some reason, this is missing from libbluetooth */
typedef	struct {
	bdaddr_t bdaddr;
} __attribute__((packed)) link_key_neg_reply_cp;
#define LINK_KEY_NEG_REPLY_CP_SIZE 6

/*** FIXME: </utils> */

/* Create a secure l2cap socket */
static int
blts_simple_pairing_create_l2cap_socket (
    blts_simple_pairing_data *data, int master)
{
	int ret = 0;
	int flags = 0;
	struct l2cap_options opts;
	socklen_t len;
	struct bt_security sec;
	struct bt_ctx *ctx;

	FUNC_ENTER();

	ctx = data->ctx;

	data->l2cap_fd = socket (AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);

	if (data->l2cap_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to create socket");
		ret = -1;
		goto DONE;
	}

	/* Generic L2CAP socket options */
	len = sizeof (struct l2cap_options);
	memset (&opts, 0, len);

	ret = getsockopt (
	    data->l2cap_fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, &len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to get l2cap options");
		goto DONE;
	}

	/* Omtu and imtu (packet size?) */
	opts.imtu = 16 * 1024;
	opts.omtu = 16 * 1024;

	ret = setsockopt (
	    data->l2cap_fd, SOL_L2CAP, L2CAP_OPTIONS, &opts, len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set l2cap options");
		goto DONE;
	}

	/* Link mode flags */
	len = sizeof (flags);
	ret = getsockopt (data->l2cap_fd, SOL_L2CAP, L2CAP_LM, &flags, &len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to get l2cap link mode flags");
		goto DONE;
	}

	if (master) {
		flags |= L2CAP_LM_MASTER;
	} else {
		flags &= ~L2CAP_LM_MASTER;
	}

	flags |= L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT | L2CAP_LM_SECURE;

	ret = setsockopt (data->l2cap_fd, SOL_L2CAP, L2CAP_LM, &flags, len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set l2cap link mode flags");
		goto DONE;
	}

	/* Security level */
	len = sizeof (struct bt_security);
	memset (&sec, 0, len);
	sec.level = BT_SECURITY_MEDIUM;

	ret = setsockopt (
	    data->l2cap_fd, SOL_BLUETOOTH, BT_SECURITY, &sec, len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set security level");
		goto DONE;
	}

	FUNC_LEAVE();

DONE:
	return ret;
}

/* Open a l2cap connection to remote device */
static int
blts_simple_pairing_l2cap_connect (blts_simple_pairing_data *data, int master)
{
	int ret = 0;
	socklen_t len;
	struct sockaddr_l2 address;
	struct bt_ctx *ctx;

	FUNC_ENTER();

	ctx = data->ctx;

	/* Create the socket */
	ret = blts_simple_pairing_create_l2cap_socket (data, master);

	if (ret < 0) {
		BLTS_ERROR ("Failed to create l2cap socket\n");
		goto DONE;
	}

        /* And finally the connection */
	len = sizeof (struct sockaddr_l2);
	memset (&address, 0, len);
	address.l2_family = AF_BLUETOOTH;
	address.l2_bdaddr = ctx->remote_mac;
	address.l2_psm    = 0;

	ret = connect (data->l2cap_fd, (struct sockaddr *) &address, len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to connect");
		goto DONE;
	}

	FUNC_LEAVE();

DONE:
	return ret;
}

/* Init device with simple pairing mode */
static int
blts_simple_pairing_init (blts_simple_pairing_data *data, int master)
{
	int ret = 0;
	uint8_t mode = 0x00;
	struct hci_dev_info dev_info;
	struct hci_dev_req  dev_request;
	struct bt_ctx *ctx;

	FUNC_ENTER();

	ctx = data->ctx;

	ctx->hci_fd = hci_open_dev (ctx->dev_id);

	if (ctx->hci_fd <= 0) {
		BLTS_LOGGED_PERROR ("Failed to open hci device");
		goto DONE;
	}

	ret = hci_send_cmd (ctx->hci_fd, OGF_HOST_CTL, OCF_RESET, 0, 0);

	if (ret < 0 && errno != ENETDOWN && errno != ENODEV) {
		BLTS_LOGGED_PERROR ("Failed to reset device");
		goto DONE;
	}

	usleep (100000);

	ret = ioctl (ctx->hci_fd, HCIDEVDOWN, ctx->dev_id);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to put device down");
		goto DONE;
	}

	usleep (100000);

	memset (&dev_request, 0, sizeof (struct hci_dev_req));

	dev_request.dev_id = ctx->dev_id;
	dev_request.dev_opt = (HCI_LM_ACCEPT | HCI_LM_AUTH |
			       HCI_LM_ENCRYPT | HCI_LM_TRUSTED |
			       HCI_LM_SECURE);

	if (master) {
		dev_request.dev_opt |= HCI_LM_MASTER;
	} else {
		dev_request.dev_opt &= ~HCI_LM_MASTER;
	}

	ret = ioctl (ctx->hci_fd, HCISETLINKMODE, &dev_request);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set link mode");
		goto DONE;
	}

	ret = ioctl (ctx->hci_fd, HCIDEVUP, ctx->dev_id);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to bring device back up");
		goto DONE;
	}

	usleep (100000);

	dev_request.dev_opt = (HCI_LP_RSWITCH | HCI_LP_SNIFF | HCI_LP_HOLD |
			       HCI_LP_PARK);
	ret = ioctl (ctx->hci_fd, HCISETLINKPOL, &dev_request);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set link policy");
		goto DONE;
	}

	ret = hci_devinfo (ctx->dev_id, &dev_info);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to get device info");
		goto DONE;
	}

	if (hci_test_bit (HCI_RAW, &dev_info.flags)) {
		BLTS_ERROR ("Device is in raw mode\n");
		ret = -1;
		goto DONE;
	}

	if (!hci_test_bit (HCI_UP, &dev_info.flags)) {
		BLTS_ERROR ("Device isn't up?\n");
		ret = -1;
		goto DONE;
	}

	BLTS_DEBUG ("Setting simple pairing mode\n");
	mode = 0x01;
	ret = hci_write_simple_pairing_mode (ctx->hci_fd, mode, 10000);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to enable secure simple "
				    "pairing mode");
		goto DONE;
	}

	if (hci_write_class_of_dev (ctx->hci_fd, 0x010204, 1000) < 0)
		BLTS_WARNING ("Warning: failed to set local device class\n");

	ret = hci_write_local_name (
	    ctx->hci_fd,
	    master ? "blts-simple-pair-master" : "blts-simple-pair-slave",
	    1000);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to set local name");
		goto DONE;
	}

	/* Clean stored keys if any */
	if (hci_delete_stored_link_key (
		ctx->hci_fd, BDADDR_ANY, 1, 1000) < 0) {
		BLTS_WARNING ("Warning: failed to clear stored link keys\n");
	}

	/* According to specs, this needs to be done after the simple pairing
	 * mode has been written
	 */
	mode = SCAN_INQUIRY | SCAN_PAGE;

	ret = hci_send_cmd (ctx->hci_fd, OGF_HOST_CTL,
			    OCF_WRITE_SCAN_ENABLE, 1, &mode);
	if (ret < 0) {
		BLTS_LOGGED_PERROR ("WRITE_SCAN_ENABLE failed");
		goto DONE;
	}

	ret = 0;

DONE:
/* 	if (ctx->hci_fd > 0) { */
/* 		hci_close_dev (ctx->hci_fd); */
/* 		ctx->hci_fd = -1; */
/* 	} */

	FUNC_LEAVE();

	return ret;
}

/* Try connecting to a device with secure simple pairing */
static int
blts_simple_pairing_master (blts_simple_pairing_data *data)
{
	int ret = 0;
	int timeout = 0;
	struct bt_ctx *ctx;
	auth_requested_cp auth_param;
	char remote_name[256] = {'\0'};
	BtFuteAgent *agent;

	ctx = data->ctx;

	link_key_neg_reply_cp neg_reply = {
		.bdaddr = ctx->remote_mac,
	};

	FUNC_ENTER();

	if (ctx->dev_id < 0) {
		ctx->dev_id = hci_get_route (NULL);
	}

	if (ctx->dev_id < 0) {
		BLTS_ERROR ("Failed to get an adapter id\n");
		ret = -1;
		goto DONE;
	}

	BLTS_DEBUG ("Running agent...\n");
	agent = bt_fute_agent_new (ctx->dev_id);
	ret = bt_fute_agent_run (agent);
	if (ret < 0) {
		BLTS_ERROR ("Failed to run agent\n");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

	BLTS_DEBUG ("Forming L2CAP Connection\n");
	ret = blts_simple_pairing_l2cap_connect (data, 1);

	if (ret < 0) {
		BLTS_ERROR ("Failed to connect to remote\n");
		goto DONE;
	}

	sleep (10);

#if 0
	ctx->hci_fd = hci_open_dev (ctx->dev_id);

	if (ctx->hci_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open device");
		ret = -1;
		goto DONE;
	}

	timeout = (ctx->test_timeout.tv_sec * 1E3
		   + ctx->test_timeout.tv_nsec * 1E-6);

	BLTS_DEBUG ("Setting simple pairing mode...");
	ret = hci_write_simple_pairing_mode (ctx->hci_fd, 0x01, timeout);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to write simple pairing mode");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

	BLTS_DEBUG ("Forming HCI connection...");

	ret = hci_create_connection (
	    ctx->hci_fd, &ctx->remote_mac, ACL_PTYPE_MASK,
	    0x00, 0x01, &ctx->conn_handle, timeout);

	if (ret < 0) {
		BLTS_ERROR ("HCI connection failed\n");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

	BLTS_DEBUG ("Getting remote name...");
	ret = hci_read_remote_name (
	    ctx->hci_fd, &ctx->remote_mac, 256, remote_name, timeout);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to read remote name");
		goto DONE;
	}
	BLTS_DEBUG ("%s\n", remote_name);

/* 	BLTS_DEBUG ("Exchanging keys..."); */
/* 	ret = hci_change_link_key (ctx->hci_fd, ctx->conn_handle, timeout); */

/* 	if (ret < 0) { */
/* 		BLTS_LOGGED_PERROR ("Failed to exchange keys"); */
/* 		goto DONE; */
/* 	} */
/* 	BLTS_DEBUG ("done\n"); */

/* 	ret = wait_until_event (ctx->hci_fd, EVT_READ_REMOTE_, timeout); */

/* 	if (ret <= 0) { */
/* 		BLTS_ERROR ("No connection complete event.\n"); */
/* 		goto DONE; */
/* 	} */

	auth_param.handle = ctx->conn_handle;

	BLTS_DEBUG ("Sending AUTH_REQUESTED\n");
	ret = hci_send_cmd (ctx->hci_fd, OGF_LINK_CTL, OCF_AUTH_REQUESTED,
			    AUTH_REQUESTED_CP_SIZE, &auth_param);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("AUTH_REQUESTED failed");
		goto DONE;
	}

	ret = wait_until_event (ctx->hci_fd, EVT_LINK_KEY_REQ, timeout);
	if (ret <= 0) {
		BLTS_ERROR ("No link key request.\n");
		goto DONE;
	}

	BLTS_DEBUG ("Sending LINK_KEY_NEG_REPLY\n");
	if (hci_send_cmd (ctx->hci_fd, OGF_LINK_CTL, OCF_LINK_KEY_NEG_REPLY,
			  LINK_KEY_NEG_REPLY_CP_SIZE, &neg_reply) < 0) {
		BLTS_ERROR ("LINK_KEY_NEG_REPLY failed\n");
		ret = errno ? -errno : -1;
		goto DONE;
	}

	ret = wait_until_event (
	    ctx->hci_fd, EVT_IO_CAPABILITY_REQUEST, timeout);

	if (ret <= 0) {
		BLTS_ERROR ("Error: No IO Capability request\n");
		ret = -1;
		goto DONE;
	}

/* 	ret = wait_until_event ( */
/* 	    ctx->hci_fd, EVT_IO_CAPABILITY_REQUEST, timeout); */

/* 	if (ret <= 0) { */
/* 		BLTS_ERROR ("Error: No IO Capability request\n"); */
/* 		ret = -1; */
/* 		goto DONE; */
/* 	} */

/* 	ret = generic_client_test_echo (data->l2cap_fd); */

/* 	if (ret < 0) { */
/* 		BLTS_ERROR ("Data exchange failed\n"); */
/* 		goto DONE; */
/* 	} */
#endif

DONE:
	if (agent) {
		bt_fute_agent_stop (agent);
		bt_fute_agent_unref (agent);
	}

	FUNC_LEAVE();

	return ret;
}

/* Try accepting a device with secure simple pairing */
static int
blts_simple_pairing_slave (blts_simple_pairing_data *data)
{
	int ret = 0;

	FUNC_ENTER();


	ret = 0;

DONE:

	FUNC_LEAVE();

	return ret;
}

/* Run it */
int
blts_simple_pairing_run (bt_data *user_data, int master)
{
	int ret = 0;
	struct bt_ctx *ctx;
	blts_simple_pairing_data *data;

	FUNC_ENTER();

	ctx = bt_ctx_new ();
	data = blts_simple_pairing_data_new (ctx);

	if (!data || !ctx) {
		BLTS_LOGGED_PERROR ("Failed to create context!");
		return -1;
	}

	str2ba (user_data->mac_address, &ctx->remote_mac);

	ctx->dev_id = user_data->dev_id;
	ctx->remote_port = 0x1234;
	data->role = master; /* Initial wish of role */

        /* XXX:
	 * - We need an adapter like functionality here, that can
	 *   handle the incoming events, and update flags for role
	 *   switches, so the test code can adopt into possible role changes.
	 */

	/* XXX: This code is probably wrong... */
	/* ret = blts_simple_pairing_init (data); */

	if (!ret && master)
		ret = blts_simple_pairing_master (data);
	else if (!ret)
		ret = blts_simple_pairing_slave (data);

	if (ctx) {
		if (ctx->hci_fd > 0) {
			hci_close_dev (ctx->hci_fd);
			ctx->hci_fd = -1;
		}

		bt_ctx_free (ctx);
	}

	if (data)
		blts_simple_pairing_data_free (&data);

	FUNC_LEAVE();

	return ret;
}

/* L2CAP "secure" server for simple pairing cases */
int
blts_simple_pairing_l2cap_server (bt_data *user_data)
{
	int ret = 0;
	struct bt_ctx *ctx;
	blts_simple_pairing_data *data;
	socklen_t len;
	struct sockaddr_l2 address;

	FUNC_ENTER();

	ctx = bt_ctx_new ();
	data = blts_simple_pairing_data_new (ctx);

	if (!data || !ctx) {
		BLTS_LOGGED_PERROR ("Failed to create context!");
		ret = -1;
		goto DONE;
	}

	ctx->dev_id     = user_data->dev_id;
	ctx->local_mac  = *BDADDR_ANY;
	ctx->local_port = 0x1234;

	ret = blts_simple_pairing_init (data, 0);

	if (ret < 0) {
		BLTS_ERROR ("Failed to init simple pairing\n");
		goto DONE;
	}

	ret = blts_simple_pairing_create_l2cap_socket (data, 0);

	if (ret < 0) {
		BLTS_ERROR ("Failed to create server socket\n");
		goto DONE;
	}

	len = sizeof (struct sockaddr_l2);
	memset (&address, 0, len);
	address.l2_family = AF_BLUETOOTH;
	address.l2_bdaddr = ctx->local_mac;
	address.l2_psm    = htobs (ctx->local_port);

	ret = bind (data->l2cap_fd, (struct sockaddr *) &address, len);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to bind server socket");
		goto DONE;
	}

	ret = listen (data->l2cap_fd, 1);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to listen socket");
		goto DONE;
	}

	ret = l2cap_echo_server_wait_handle_one (ctx, data->l2cap_fd);

	if (ret < 0) {
		BLTS_ERROR ("Failed to handle incoming data\n");
		goto DONE;
	}

DONE:
	if (ctx) {
		if (ctx->hci_fd > 0) {
			hci_close_dev (ctx->hci_fd);
			ctx->hci_fd = -1;
		}

		bt_ctx_free (ctx);
	}
	if (data)
		blts_simple_pairing_data_free (&data);

	FUNC_LEAVE();

	return ret;
}

/* OOB initiated simple pairing */
int
blts_simple_pairing_oob_run (bt_data *user_data, int master)
{
	int ret = -1;
	int timeout = 0;
	struct bt_ctx *ctx;
	blts_simple_pairing_data *data;
	uint8_t hash = 0;
	uint8_t randomizer = 0;

	FUNC_ENTER();

	ctx = bt_ctx_new ();
	data = blts_simple_pairing_data_new (ctx);

	if (!data || !ctx) {
		BLTS_LOGGED_PERROR ("Failed to create context!");
		return -1;
	}

	str2ba (user_data->mac_address, &ctx->remote_mac);

	ctx->dev_id = user_data->dev_id;
	ctx->remote_port = 0x1234;

	if (ctx->dev_id < 0) {
		ctx->dev_id = hci_get_route (NULL);
	}

	if (ctx->dev_id < 0) {
		BLTS_ERROR ("Failed to get an adapter id\n");
		ret = -1;
		goto DONE;
	}

	ctx->hci_fd = hci_open_dev (ctx->dev_id);

	if (ctx->hci_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open device");
		ret = -1;
		goto DONE;
	}

	timeout = (ctx->test_timeout.tv_sec * 1E3
		   + ctx->test_timeout.tv_nsec * 1E-6);

	BLTS_DEBUG ("Reading local OOB data...");
	ret = hci_read_local_oob_data (ctx->hci_fd, &hash, &randomizer,
				       timeout);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to read local OOB data");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");
	BLTS_DEBUG ("Hash: %u, randomizer: %u\n", hash, randomizer);

	BLTS_DEBUG ("Setting simple pairing mode...");
	ret = hci_write_simple_pairing_mode (ctx->hci_fd, 0x01, timeout);

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to write simple pairing mode");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

	/* Transfer OOB data */

DONE:
	if (ctx) {
		if (ctx->hci_fd > 0) {
			hci_close_dev (ctx->hci_fd);
			ctx->hci_fd = -1;
		}

		bt_ctx_free (ctx);
	}
	if (data)
		blts_simple_pairing_data_free (&data);

	return ret;
}
