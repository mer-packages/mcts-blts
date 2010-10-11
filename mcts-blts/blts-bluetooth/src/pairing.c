/* pairing.c -- bluetoothd-less pairing

   Copyright (C) 2000-2010, Nokia Corporation.
   Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
   Copyright (C) 2003-2007  Marcel Holtmann <marcel@holtmann.org>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.


   Includes Bluetooth event interpretation from BlueZ HCIDump source.
*/

#define _GNU_SOURCE

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <stdint.h>

#include <blts_log.h>

#include "pairing.h"
#include "bt_ctx.h"
#include "hci.h"

/* stores last seen connection bdaddr */
bdaddr_t glob_connected_ba;

/* Event interpretation (from Bluez hcidump) */
#define EVENT_NUM 61
static char *event_str[EVENT_NUM + 1] = {
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
 * Low level init + reset BT adapter #devnum. master -> LM setting. See BT spec
 * on what the magic numbers do.
 * Return negative on error, otherwise 0.
 */
static int ll_bt_init(int devnum, int master)
{
	int dd, ret = 0;
	struct hci_dev_req dr;
	struct hci_dev_info di;
	uint8_t lap[] = { 0x33,0x8b,0x9e };
	uint8_t scan_page;

	dd = hci_open_dev(devnum);
	if (!dd) {
		log_print("Failure in adapter device open - %s\n", strerror(errno));
		ret = -errno;
		goto done;
	}

	/* First, reset the adapter. We'll catch errors here on the devinfo call. */
	hci_send_cmd(dd, OGF_HOST_CTL, OCF_RESET, 0, 0);

	usleep(100000);
	ioctl(dd, HCIDEVDOWN, devnum);
	usleep(100000);

	/* LM options */
	memset(&dr, 0, sizeof(dr));

	dr.dev_id = devnum;
	dr.dev_opt = HCI_LM_ACCEPT | HCI_LM_MASTER |
		HCI_LM_AUTH | HCI_LM_ENCRYPT |
		HCI_LM_TRUSTED | HCI_LM_SECURE;
	ioctl(dd, HCISETLINKMODE, (unsigned long) &dr);

	/* Policy (should we disable role switch?) */
	dr.dev_opt = HCI_LP_RSWITCH | HCI_LP_SNIFF | HCI_LP_HOLD | HCI_LP_PARK;
	ioctl(dd, HCISETLINKPOL, (unsigned long) &dr);

	/* We can now bring the device up and continue with libbluetooth code */
	ioctl(dd, HCIDEVUP, devnum);

	if (hci_devinfo(devnum, &di) < 0) {
		log_print("Adapter init failed.\n");
		ret = -errno;
		goto done;
	}

	if (hci_test_bit(HCI_RAW, &di.flags)) {
		log_print("Warning: adapter in raw mode.\n");
	}

	if (hci_write_local_name(dd, master ? "bttest" : "bttest-cli", 1000) < 0)
		log_print("Warning: failed to set local name\n");

	if (hci_write_class_of_dev(dd, 0x010204, 1000) < 0)
		log_print("Warning: failed to set local device class\n");

	/* Clean stored keys if any */
	if (hci_delete_stored_link_key(dd, BDADDR_ANY, 1, 1000) < 0) {
		log_print("Warning: failed to clear stored link keys\n");
	}


	/* Go visible */
	if (hci_write_current_iac_lap(dd, 1, lap, 1000) < 0) {
		log_print("Fatal: WRITE_CURRENT_IAC_LAP failed\n");
		ret = -errno;
		goto done;
	}

	scan_page = master ? SCAN_PAGE : SCAN_INQUIRY | SCAN_PAGE;
	if (hci_send_cmd(dd, OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE, 1, &scan_page) < 0) {
		log_print("Fatal: WRITE_SCAN_ENABLE failed\n");
		ret = -errno;
		goto done;
	}
	/* If not supported, this will just fail */
	hci_write_simple_pairing_mode(dd, 0, 1000);

done:

	hci_close_dev(dd);

	return ret;
}

/*
 * Bring down BT adapter #devnum.
 */
static int ll_bt_deinit(int devnum)
{
	int dd = hci_open_dev(devnum);
	if (dd < 0)
		return -errno;
	ioctl(dd, HCIDEVDOWN, devnum);
	return hci_close_dev(devnum);
}

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

	log_print("Waiting for event 0x%.2x (\"%s\")\n", event, event_str[event]);

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
			log_print("Failed waiting for event\n");
			ret = errno?-errno:-1;
			break;
		}
		gettimeofday(&tv, 0);
		t_diff = t_end - tv.tv_sec - 1E-6 * tv.tv_usec;
		if (t_diff <= 0) {
			log_print("Timed out waiting for event\n");
			ret = -ETIMEDOUT;
			break;
		}

		timeout = t_diff * 1E3;

		hdr = (void *) (buf + 1);
		log_print("> Event 0x%.2x (\"%s\") (%lf s)\n",hdr->evt,
			event_str[hdr->evt], t_end - t_diff - t_start);
		if (hdr->evt == event) {
			if (event == EVT_CONN_COMPLETE) {
				evt_conn_complete* params = (HCI_EVENT_HDR_SIZE+1+(void*)buf);
				bacpy(&glob_connected_ba, &(params->bdaddr));
			}
			ret = 0;
			break;
		}
	}

	if (buf)
		free(buf);

	return ret;
}


static int do_pin_exchange(int dd, pin_code_reply_cp *reply, int timeout)
{
	int ret;

	ret = wait_until_event(dd, EVT_PIN_CODE_REQ, timeout);
	if (ret) {
		log_print("Pin code not requested\n");
		return ret;
	}

	log_print("Sending PIN_CODE_REPLY\n");
	if (hci_send_cmd(dd, OGF_LINK_CTL, OCF_PIN_CODE_REPLY,
		PIN_CODE_REPLY_CP_SIZE, reply) < 0) {
		log_print("PIN_CODE_REPLY failed\n");
		return errno ? -errno : -1;
	}

	ret = wait_until_event(dd, EVT_LINK_KEY_NOTIFY, timeout);
	if (ret) {
		log_print("No LINK_KEY_NOTIFY after pin entry.\n");
		return ret;
	}

	return 0;
}

/* For some reason, this is missing from libbluetooth */
typedef	struct {
	bdaddr_t bdaddr;
} __attribute__((packed)) link_key_neg_reply_cp;
#define LINK_KEY_NEG_REPLY_CP_SIZE 6

/*
 * Pairing sequence, master end
 */
static int do_pairing_master(struct bt_ctx *ctx)
{
	int dd, ret = -1;
	auth_requested_cp auth_param;

	if (!ctx)
		return -EINVAL;

	int timeout = (ctx->test_timeout).tv_sec * 1E3
		+ (ctx->test_timeout).tv_nsec * 1E-6;

	/* PIN = "1234" */
	pin_code_reply_cp pin_reply = {
		.bdaddr = ctx->remote_mac,
		.pin_len = 4,
		.pin_code = { 0x31, 0x32, 0x33, 0x34,
			      0, 0, 0, 0,
			      0, 0, 0, 0,
			      0, 0, 0, 0 },
	};

	link_key_neg_reply_cp neg_reply = {
		.bdaddr = ctx->remote_mac,
	};

	log_print("Connecting to remote.\n");
	if (hci_connect_remote(ctx) < 0) {
		log_print("Initial connect failed\n");
		return -1;
	}
	dd = ctx->hci_fd;
	auth_param.handle = ctx->conn_handle;

	log_print("Sending AUTH_REQUESTED\n");
	if (hci_send_cmd(dd, OGF_LINK_CTL, OCF_AUTH_REQUESTED,
		AUTH_REQUESTED_CP_SIZE, &auth_param) < 0) {
		log_print("AUTH_REQUESTED failed\n");
		ret = errno ? -errno : -1;
		goto done;
	}

	ret = wait_until_event(dd, EVT_LINK_KEY_REQ, timeout);
	if (ret) {
		log_print("No link key request.\n");
		goto done;
	}

	log_print("Sending LINK_KEY_NEG_REPLY\n");
	if (hci_send_cmd(dd, OGF_LINK_CTL, OCF_LINK_KEY_NEG_REPLY, 6, &neg_reply) < 0) {
		log_print("LINK_KEY_NEG_REPLY failed\n");
		ret = errno ? -errno : -1;
		goto done;
	}

	ret = do_pin_exchange(dd, &pin_reply, timeout);
	if (ret) {
		log_print("Pairing failed.\n");
		goto done;
	}

	/* Some devices don't send this before doing no-activity timeout disconnect,
	   so we don't fail here, just log what happens. */
 	wait_until_event(dd, EVT_AUTH_COMPLETE, timeout);
/* 	if (ret) { */
/* 		log_print("No AUTH_COMPLETE.\n"); */
/* 		goto done; */
/* 	} */
	log_print("Pairing complete.\n");

done:
	hci_close_dev(dd);
	return ret;
}

/*
 * Pairing sequence, slave end
 */
static int do_pairing_slave(struct bt_ctx *ctx)
{
	int dd, ret = -1;
	char addr[18];

	if (!ctx)
		return -EINVAL;

	/* PIN = "1234" */
	int timeout = (ctx->test_timeout).tv_sec * 1E3
		+ (ctx->test_timeout).tv_nsec * 1E-6;

	dd = hci_open_dev(ctx->dev_id);
	if (dd < 0) {
		logged_perror("Failed to open device");
		return -errno;
	}

	ret = wait_until_event(dd, EVT_CONN_COMPLETE, timeout);
	if (ret) {
		log_print("No connection arrived, check master\n");
		goto done;
	}

	ba2str(&glob_connected_ba, addr);
	log_print("Incoming connection from %s\n", addr);
	pin_code_reply_cp pin_reply = {
		.bdaddr = glob_connected_ba,
		.pin_len = 4,
		.pin_code = { 0x31, 0x32, 0x33, 0x34,
			      0, 0, 0, 0,
			      0, 0, 0, 0,
			      0, 0, 0, 0 },
	};

	/* now, wait for the pin query*/
	ret = do_pin_exchange(dd, &pin_reply, timeout);
	if (ret)
		log_print("Pairing failed.\n");
	else
		log_print("Pairing complete.\n");

done:
	hci_close_dev(dd);

	return ret;
}


/*
 * Test case: bring up bt adapters on both ends without bluetoothd, and
 * establish authenticated connection using pin query sequence.
 * ctx needs local and remote mac, adapter number and timeout; set master
 * for one end, unset for the other.
 * 0 success, <0 fail.
 */
int pairing_no_btd(struct bt_ctx *ctx, int master)
{
	int ret = -1;
	if (!ctx)
		return -EINVAL;

	if (ll_bt_init(ctx->dev_id, master) < 0) {
		log_print("Device init failed.\n");
		return errno ? -errno : -1;
	}

	if (master)
		ret = do_pairing_master(ctx);
	else
		ret = do_pairing_slave(ctx);

	ll_bt_deinit(ctx->dev_id);

	return ret;
}

