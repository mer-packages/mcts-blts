/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* bt_fute_agent.c -- Bluetooth pairing agent for tests

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
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <blts_log.h>

#include "bt_fute_agent.h"

/* Private data implementation */
struct _BtFuteAgent {

	/* BT device info */
	int adapter;

	int hci_fd;
	int raw_hci;

	BtFuteAgentRole role;

	/* Thread info */
	pthread_t thread_id;
	int stop;
	pthread_mutex_t mutex;

	/* Data references */
	int ref_count;
};

/* Data packet */
typedef struct {
	uint16_t       len;
	uint8_t        in;
	uint32_t       time_sec;
	uint32_t       time_usec;
	unsigned char *payload;
} __attribute__((packed)) bt_fute_hci_packet;


/* Gets command payload data from a pointer */
#define FUTE_CMD_PAYLOAD(PTR) (void *)(			\
	    PTR + HCI_TYPE_LEN + HCI_COMMAND_HDR_SIZE)

/* Gets event payload data from a pointer */
#define FUTE_EVENT_PAYLOAD(PTR) (void *)(	\
	    PTR + HCI_TYPE_LEN + HCI_EVENT_HDR_SIZE)

/* Event interpretation (from Bluetooth specification) */
static const char *FUTE_EVENT_STR[] = {
	NULL, // 0x00
	"Inquiry Complete", // 0x01
	"Inquiry Result", // 0x02
	"Connection Complete", // 0x03
	"Connection Request", // 0x04
	"Disconnection Complete", // 0x05
	"Authentication Complete", // 0x06
	"Remote Name Request Complete", // 0x07
	"Encryption Change", // 0x08
	"Change Connection Link Key Complete", // 0x09
	"Master Link Key Complete", // 0x0a
	"Read Remote Supported Features Complete", // 0x0b
	"Read Remote Version Info Complete", // 0x0c
	"QoS Setup Complete", // 0x0d
	"Command Complete", // 0x0e
	"Command Status", // 0x0f
	"Hardware Error", // 0x10
	"Flush Occurred", // 0x11
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
	"Read Remote Extended Features Complete", // 0x23
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
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
	NULL,
	"Link Supervision Timeout Change",
	"Enhanced Flush Complete",
	NULL,
	"User Passkey Notification",
	"Keypress Notification",
	"Remote Host Supported Features Notification",
};

/* Command interpretation */
static const char *FUTE_LINK_CTL_OCF[] = {
	NULL, // 0x0000
	"Inquiry", // 0x0001
	"Inquiry cancel", // 0x0002
	"Periodic inquiry", // 0x0003
	"Exit periodic inquiry", // 0x0004
	"Create connection", // 0x0005
	"Disconnect", // 0x0006
	"Add SCO", // 0x0007
	"Cancel create connection", // 0x0008
	"Accept connection request", // 0x0009
	"Reject connection request", // 0x000a
	"Link key reply", // 0x000b
	"Link key negative reply", // 0x000c
	"Pin code reply", // 0x000d
	"Pin code negative reply", // 0x000e
	"Set connection ptype", // 0x000f
	NULL, // 0x0010
	"Auth requested", // 0x0011
	NULL, // 0x0012
	"Set connection encrypt", // 0x0013
	NULL, // 0x0014
	"Change connection link key", // 0x0015
	NULL, // 0x0016
	"Master link key", // 0x0017
	NULL, // 0x0018
	"Remote name request", // 0x0019
	"Remote name request cancel", // 0x001a
	"Read remote features", // 0x001b
	"Read remote extended features", // 0x001c
	"Read remote version", // 0x001d
	NULL, // 0x001e
	"Read clock offset", // 0x001f
	"Read LMP handle", // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	"Setup synchronous connection", // 0x0028
	"Accept synchronous connection request", // 0x0029
	"Reject synchronous connection request", // 0x002a
	"IO capability reply", // 0x002b
	"User confirm reply", // 0x002c
	"User confirm negative reply", // 0x002d
	"User passkey reply", // 0x002e
	"User passkey negative reply", // 0x002f
	"Read remote out of band data reply", // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	"Read remote out of band data negative reply", // 0x0033
	"IO capability negative reply", // 0x0034
};

static const char *LINK_POLICY_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *HOST_CTL_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *INFO_PARAM_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *STATUS_PARAM_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *TESTING_CMD_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *LE_CTL_OCF[] = {
	NULL, // 0x0000
	NULL, // 0x0001
	NULL, // 0x0002
	NULL, // 0x0003
	NULL, // 0x0004
	NULL, // 0x0005
	NULL, // 0x0006
	NULL, // 0x0007
	NULL, // 0x0008
	NULL, // 0x0009
	NULL, // 0x000a
	NULL, // 0x000b
	NULL, // 0x000c
	NULL, // 0x000d
	NULL, // 0x000e
	NULL, // 0x000f
	NULL, // 0x0010
	NULL, // 0x0011
	NULL, // 0x0012
	NULL, // 0x0013
	NULL, // 0x0014
	NULL, // 0x0015
	NULL, // 0x0016
	NULL, // 0x0017
	NULL, // 0x0018
	NULL, // 0x0019
	NULL, // 0x001a
	NULL, // 0x001b
	NULL, // 0x001c
	NULL, // 0x001d
	NULL, // 0x001e
	NULL, // 0x001f
	NULL, // 0x0020
	NULL, // 0x0021
	NULL, // 0x0022
	NULL, // 0x0023
	NULL, // 0x0024
	NULL, // 0x0025
	NULL, // 0x0026
	NULL, // 0x0027
	NULL, // 0x0028
	NULL, // 0x0029
	NULL, // 0x002a
	NULL, // 0x002b
	NULL, // 0x002c
	NULL, // 0x002d
	NULL, // 0x002e
	NULL, // 0x002f
	NULL, // 0x0030
	NULL, // 0x0031
	NULL, // 0x0032
	NULL, // 0x0033
	NULL, // 0x0034
};

static const char *FUTE_HCI_ERRORS[] = {
	"Success", // 0x00
	"Unknown command", // 0x01
	"No connection", // 0x02
	"Hardware failure", // 0x03
	"Page timeout", // 0x04
	"Authentication failure", // 0x05
	"PIN or key missing", // 0x06
	"Memory full", // 0x07
	"Connection timeout", // 0x08
	"Max number of connections", // 0x09
	"Max number of SCO connections", // 0x0a
	"ACL connection exists", // 0x0b
	"HCI command disallowed", // 0x0c
	"Rejected, limited resources", // 0x0d
	"Rejected, security", // 0x0e
	"Rejected, personal", // 0x0f
	"Host timeout", // 0x10
	"Unsupported feature", // 0x11
	"Invalid parameters", // 0x12
	"Remote user ended connection", // 0x13
	"Remote end low on resources", // 0x14
	"Remote end powered off", // 0x15
	"Connection terminated", // 0x16
	"Repeated attempts", // 0x17
	"Pairing not allowed", // 0x18
	"Unknown LMP PDU", // 0x19
	"Unsupported remote feature", // 0x1a
	"SCO offset rejected", // 0x1b
	"SCO interval rejected", // 0x1c
	"Air mode rejected", // 0x1d
	"Invalid LMP parameters", // 0x1e
	"Unspecified error", // 0x1f
	"Unsupported LMP parameter value", // 0x20
	"Role change not allowed", // 0x21
	"LMP response timeout", // 0x22
	"LMP error, transaction collision", // 0x23
	"LMP PDU not allowed", // 0x24
	"Encryption mode not accepted", // 0x25
	"Unit link key used", // 0x26
	"QoS not supported", // 0x27
	"Instant passed", // 0x28
	"Pairing not supported", // 0x29
	"Transaction collision", // 0x2a
	NULL, // 0x2b
	"QoS, unacceptable parameter", // 0x2c
	"QoS rejected", // 0x2d
	"Classification not supported", // 0x2e
	"Insufficient security", // 0x2f
	"Parameter out of range", // 0x30
	NULL, // 0x31
	"Role switch pending", // 0x32
	NULL, // 0x33
	"Slot violation", // 0x34
	"Role switch failed", // 0x35
	"EIR too large", // 0x36
	"Simple pairing not supported", // 0x37
	"Host busy pairing", // 0x38
};

/* Private functions */

/* Check if the thread should stop */
static int
bt_fute_get_stop_thread (BtFuteAgent *agent)
{
	int stop = 0;

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return 1;
	}

	pthread_mutex_lock (&agent->mutex);

	stop = agent->stop;

	pthread_mutex_unlock (&agent->mutex);

	return stop;
}

/* Opens the hci device */
static int
bt_fute_open_hci_dev (BtFuteAgent *agent)
{
	int ret = 0, opt = 0;
	struct hci_filter filter;
	struct hci_dev_info dev_info;
	struct sockaddr_hci address;

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return -1;
	}

	FUNC_ENTER();

	BLTS_DEBUG ("Opening device...");
	agent->hci_fd = hci_open_dev (agent->adapter);

	if (agent->hci_fd < 0) {
		BLTS_LOGGED_PERROR ("Failed to open device");
		ret = -1;
		goto DONE;
	}

        if (hci_devinfo (agent->adapter, &dev_info) < 0) {
		BLTS_LOGGED_PERROR ("Can't get device info");
		hci_close_dev (agent->hci_fd);
		agent->hci_fd = 0;
		ret = -1;
		goto DONE;
	}

	BLTS_TRACE ("Mapped to device: %s\n", dev_info.name);

	opt = hci_test_bit (HCI_RAW, &dev_info.flags);
        if (ioctl (agent->hci_fd, HCISETRAW, opt) < 0) {
                if (errno == EACCES) {
			BLTS_LOGGED_PERROR ("Can't access device");
			hci_close_dev (agent->hci_fd);
			agent->hci_fd = 0;
			ret = -1;
			goto DONE;
		}
	}

	hci_close_dev (agent->hci_fd);
	agent->hci_fd = 0;

	/* Open a raw socket instead of hci socket */
	agent->hci_fd = socket (AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	BLTS_DEBUG ("done\n");

        /* Set direction info */
	opt = 1;
	ret = setsockopt (agent->hci_fd, SOL_HCI, HCI_DATA_DIR, &opt,
			  sizeof (opt));
	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Can't enable data direction info");
		goto DONE;
	}

	/* Enable time stamps */
	opt = 1;
	ret = setsockopt (agent->hci_fd, SOL_HCI, HCI_TIME_STAMP, &opt,
			  sizeof (opt));
	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Can't enable time stamp");
		goto DONE;
	}

	BLTS_DEBUG ("Setting filters...");
	hci_filter_clear (&filter);
	hci_filter_all_ptypes (&filter);
	hci_filter_all_events (&filter);
        if (setsockopt (agent->hci_fd, SOL_HCI, HCI_FILTER, &filter,
			sizeof (filter)) < 0) {
		BLTS_LOGGED_PERROR ("Can't set filter");
		ret = -1;
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

	BLTS_DEBUG ("Binding to socket...");
	memset (&address, 0, sizeof (struct sockaddr_hci));
	address.hci_family = AF_BLUETOOTH;
	address.hci_dev = agent->adapter;

	ret = bind (agent->hci_fd, (struct sockaddr *) &address,
		    sizeof (struct sockaddr_hci));

	if (ret < 0) {
		BLTS_LOGGED_PERROR ("Failed to bind");
		goto DONE;
	}
	BLTS_DEBUG ("done\n");

DONE:
	FUNC_LEAVE();

	return ret;
}

/* Read data from socket */
static int
bt_fute_agent_read_data (BtFuteAgent *agent, bt_fute_hci_packet *data)
{
	struct cmsghdr *hdr;
	struct msghdr   message;
	struct iovec    iov;
	struct timeval *ts;

	char *buffer = NULL;
	char *ctrl   = NULL;
	int   len    = 0;
	int   ret    = 0;

	if (!agent) {
		BLTS_DEBUG ("No agent given in\n");
		return -1;
	}

	pthread_mutex_lock (&agent->mutex);

	FUNC_ENTER();

	if (!agent->hci_fd) {
		BLTS_DEBUG ("Socket has been disconnected\n");
		ret = -1;
		goto DONE;
	}

	ctrl = malloc (100);

	if (!ctrl) {
		BLTS_LOGGED_PERROR ("Can't allocate control buffer");
		ret = -1;
		goto DONE;
	}

	buffer = malloc (HCI_MAX_FRAME_SIZE);

	if (!buffer) {
		BLTS_LOGGED_PERROR ("Can't allocate data buffer");
		ret = -1;
		goto DONE;
	}

	memset (buffer, 0, HCI_MAX_FRAME_SIZE);
	memset (&message, 0, sizeof (message));

	iov.iov_base = buffer;
	iov.iov_len  = HCI_MAX_FRAME_SIZE;

	message.msg_iov    = &iov;
	message.msg_iovlen = 1;

	message.msg_control    = ctrl;
	message.msg_controllen = 100;

	len = recvmsg (agent->hci_fd, &message, MSG_DONTWAIT);

	if (len < 0) {
		if (errno == EINTR || errno == EAGAIN) {
			BLTS_DEBUG ("Interrupt\n");
			ret = 0;
			goto DONE;
		}

		BLTS_LOGGED_PERROR ("Receive failed");
		ret = -1;
		goto DONE;
	}

	if (!len) {
		BLTS_LOGGED_PERROR ("Disconnect");
		ret = -1;
		goto DONE;
	}

	data->len = len;

	BLTS_TRACE ("Read %d bytes\n", len);

	/* Resolve direction and time stamps */
	hdr = CMSG_FIRSTHDR (&message);
	while (hdr) {
		if (hdr->cmsg_type == HCI_CMSG_DIR) {
			data->in = *(CMSG_DATA (hdr));
		} else if (hdr->cmsg_type == HCI_CMSG_TSTAMP) {
			ts = (struct timeval *)CMSG_DATA (hdr);
			data->time_sec  = ts->tv_sec;
			data->time_usec = ts->tv_usec;
		}

		hdr = CMSG_NXTHDR (&message, hdr);
	}

	/* Then copy the data... */
	data->payload = malloc (len);
	if (!data->payload) {
		BLTS_LOGGED_PERROR ("Failed to allocate payload");
		ret = -1;
		goto DONE;
	}

	memcpy (data->payload, buffer, len);

DONE:
	if (buffer)
		free (buffer);
	if (ctrl)
		free (ctrl);

	FUNC_LEAVE();

	pthread_mutex_unlock (&agent->mutex);

	return ret;
}

/* Print the ogf name based on opcode */
static const char *
bt_fute_agent_opcode_to_ogf_str (uint16_t opcode)
{
	uint16_t code = btohs (opcode);
	uint8_t  ogf  = cmd_opcode_ogf (code);

	switch (ogf) {
	case OGF_LINK_CTL:     return "Link control";
	case OGF_LINK_POLICY:  return "Link policy";
	case OGF_HOST_CTL:     return "Host control";
	case OGF_INFO_PARAM:   return "Info param";
	case OGF_STATUS_PARAM: return "Status param";
	case OGF_TESTING_CMD:  return "Testing command";
	case OGF_LE_CTL:       return "Low energy control";
	case OGF_VENDOR_CMD:   return "Vendor specific";
	default: break;
	}

	return "Unknown";
}

/* Print the ocf name based on opcode */
static const char *
bt_fute_agent_opcode_to_ocf_str (uint16_t opcode)
{
	uint16_t code = btohs (opcode);
	uint8_t  ogf  = cmd_opcode_ogf (code);
	uint16_t ocf  = cmd_opcode_ocf (code);

	switch (ogf) {
	case OGF_LINK_CTL:     return FUTE_LINK_CTL_OCF[ocf];
	case OGF_LINK_POLICY:  return LINK_POLICY_OCF[ocf];
	case OGF_HOST_CTL:     return HOST_CTL_OCF[ocf];
	case OGF_INFO_PARAM:   return INFO_PARAM_OCF[ocf];
	case OGF_STATUS_PARAM: return STATUS_PARAM_OCF[ocf];
	case OGF_TESTING_CMD:  return TESTING_CMD_OCF[ocf];
	case OGF_LE_CTL:       return LE_CTL_OCF[ocf];
	case OGF_VENDOR_CMD:   return "Vendor specific";
	default: break;
	}

	return "Unknown";
}

/* Print page scan repetition mode */
static const char *
bt_fute_agent_pscan_rep_mode_to_str (uint16_t mode)
{
	switch (mode) {
	case 0x00:
		return "R0";
	case 0x01:
		return "R1";
	case 0x02:
		return "R2";
	default:
		break;
	}

	return "Reserved";
}

/* Print retrans effort */
static const char *
bt_fute_agent_retrans_effort_to_str (uint8_t effort)
{
	switch (effort) {
	case 0x00:
		return "No retransmissions";
	case 0x01:
		return "Optimize for power consumption";
	case 0x02:
		return "Optimize for link quality";
	case 0xFF:
		return "Don't care";
	default:
		break;
	}

	return "Reserved";
}

/* Print io capabilities */
static const char *
bt_fute_agent_caps_to_str (uint8_t caps)
{
	switch (caps) {
	case 0x00:
		return "DisplayOnly";
	case 0x01:
		return "DisplayYesNo";
	case 0x02:
		return "KeyboardOnly";
	case 0x03:
		return "NoInputNoOutput";
	default:
		break;
	}

	return "Reserved";
}

/* Print authentication method */
static const char *
bt_fute_agent_auth_to_str (uint8_t auth)
{
	switch (auth) {
	case 0x00:
		return "No MITM, no bonding. Numeric comparison with automatic accept allowed.";
	case 0x01:
		return "MITM required, no bonding. Use IO caps to determine authentication procedure.";
	case 0x02:
		return "No MITM, dedicated bonding. Numeric comparison with automatic accept allowed.";
	case 0x03:
		return "MITM required, dedicated bonding. Use IO caps to determine authentication procedure.";
	case 0x04:
		return "No MITM, general bonding. Numeric comparison with automatic accept allowed.";
	case 0x05:
		return "MITM required, general bonding. Use IO caps to determinate authentication procedure.";
	default: break;
	}
	return "Reserved";
}

/* Print HCI device mode */
static const char *
bt_fute_agent_mode_to_str (uint8_t mode)
{
	switch (mode) {
	case 0x00: return "Active";
	case 0x01: return "Hold";
	case 0x02: return "Sniff";
	case 0x03: return "Park";
	default: break;
	}

	return "Reserved";
}

/* Print link key type */
static const char *
bt_fute_agent_key_type_to_str (uint8_t type)
{
	switch (type) {
	case 0x00: return "Combination key";
	case 0x01: return "Local Unit key";
	case 0x02: return "Remote Unit key";
	case 0x03: return "Debug Combination key";
	case 0x04: return "Unauthenticated Combination key";
	case 0x05: return "Authenticated Combination key";
	case 0x06: return "Changed Combination key";
	default: break;
	}

	return "Reserved";
}

/* Air mode to string */
static const char *
bt_fute_agent_air_mode_to_str (uint8_t mode)
{
	switch (mode) {
	case 0x00: return "µ-law log";
	case 0x01: return "A-law log";
	case 0x02: return "CVSD";
	case 0x03: return "Transparent";
	default: break;
	}

	return "Reserved";
}

/* Dump payload for link control command */
static int
bt_fute_agent_dump_link_ctl_cmd (
    int hci_ocf, bt_fute_hci_packet *data, hci_command_hdr *cmd)
{
	inquiry_cp                  *inq;
	periodic_inquiry_cp         *per_inq;
	create_conn_cp              *conn;
	disconnect_cp               *disconn;
	add_sco_cp                  *sco;
	create_conn_cancel_cp       *cancel_conn;
	accept_conn_req_cp          *accept_conn;
	reject_conn_req_cp          *reject_conn;
	link_key_reply_cp           *link_key_reply;
	pin_code_reply_cp           *pin_reply;
	set_conn_ptype_cp           *conn_ptype;
	auth_requested_cp           *auth_req;
	set_conn_encrypt_cp         *conn_encrypt;
	change_conn_link_key_cp     *conn_key;
	master_link_key_cp          *master_key;
	remote_name_req_cp          *remote_name;
	remote_name_req_cancel_cp   *remote_name_cancel;
	read_remote_features_cp     *remote_feat;
	read_remote_ext_features_cp *remote_ext_feat;
	read_remote_version_cp      *remote_version;
	read_clock_offset_cp        *read_clock;
	setup_sync_conn_cp          *sync_conn;
	accept_sync_conn_req_cp     *sync_conn_req;
	reject_sync_conn_req_cp     *rej_sync_conn;
	io_capability_reply_cp      *io_caps;
	user_confirm_reply_cp       *confirm;
	user_passkey_reply_cp       *pass;
	remote_oob_data_reply_cp    *oob;
	io_capability_neg_reply_cp  *io_neg;

	bdaddr_t *baddr;
	char *tmp = NULL;
	char ba[18] = {'\0'};
	uint16_t *handle;

	int ret = 0;

	switch (hci_ocf) {
	case OCF_INQUIRY:
		if (cmd->plen != INQUIRY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		inq = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tlap: 0x%.2x 0x%.2x 0x%.2x ",
			    inq->lap[0], inq->lap[1], inq->lap[2]);
		BLTS_DEBUG ("length in 1.28s units: %d ", inq->length);
		BLTS_DEBUG ("num rsp: %d\n", inq->num_rsp);
		break;

	case OCF_INQUIRY_CANCEL:
	case OCF_EXIT_PERIODIC_INQUIRY:
		if (cmd->plen) {
			BLTS_DEBUG ("Bluez stack has no defined "
				    "payload for the command, but "
				    "there's a hidden payload?!\n");
			ret = -1;
		}
		break;

	case OCF_PERIODIC_INQUIRY:
		if (cmd->plen != PERIODIC_INQUIRY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		per_inq = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tmax period in 1.28s units: %d "
			    "min period in 1.28s units: %d\n",
			    btohs (per_inq->max_period),
			    btohs (per_inq->min_period));
		BLTS_DEBUG ("\tlap: 0x%.2x 0x%.2x 0x%.2x ",
			    per_inq->lap[0], per_inq->lap[1],
			    per_inq->lap[2]);
		BLTS_DEBUG ("length in 1.28s units: %d ", inq->length);
		BLTS_DEBUG ("num rsp: %d\n", inq->num_rsp);
		break;

	case OCF_CREATE_CONN:
		if (cmd->plen != CREATE_CONN_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		conn = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&conn->bdaddr, ba);
		tmp = hci_ptypetostr (btohs (conn->pkt_type));
		BLTS_DEBUG ("\taddress: %s\n", ba);
		BLTS_DEBUG ("\tpacket types: %s\n"
			    "\tpage scan repetition mode: %s\n"
			    "\tpscan mode (0x%.2x): %s\n"
			    "\tclock offset: 0x%.4x, allow role "
			    "switch: %d\n",
			    tmp,
			    bt_fute_agent_pscan_rep_mode_to_str (
				conn->pscan_rep_mode),
			    conn->pscan_mode,
			    conn->pscan_mode != 0x00 ?
			    "ERROR" : "Reserved",
			    btohs (conn->clock_offset),
			    conn->role_switch);
		break;

	case OCF_DISCONNECT:
		if (cmd->plen != DISCONNECT_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		disconn = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d reason (0x%.2x) %s\n",
			    btohs (disconn->handle),
			    disconn->reason,
			    FUTE_HCI_ERRORS[disconn->reason]);
		break;

	case OCF_ADD_SCO:
		if (cmd->plen != ADD_SCO_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sco = FUTE_CMD_PAYLOAD (data->payload);
		tmp = hci_scoptypetostr (btohs (sco->pkt_type));
		BLTS_DEBUG ("\thandle: %d packet types: %s\n",
			    btohs (sco->handle), tmp);
		break;

	case OCF_CREATE_CONN_CANCEL:
		if (cmd->plen != CREATE_CONN_CANCEL_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		cancel_conn = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&cancel_conn->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case OCF_ACCEPT_CONN_REQ:
		if (cmd->plen != ACCEPT_CONN_REQ_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		accept_conn = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&accept_conn->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, role: %s\n", ba,
			    accept_conn->role == BT_FUTE_ROLE_MASTER ?
			    "MASTER" : "SLAVE");
		break;

	case OCF_REJECT_CONN_REQ:
		if (cmd->plen != REJECT_CONN_REQ_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		reject_conn = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&reject_conn->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, reason: (0x%.2x) %s\n",
			    ba, reject_conn->reason,
			    FUTE_HCI_ERRORS[reject_conn->reason]);
		break;

	case OCF_LINK_KEY_REPLY:
		if (cmd->plen != LINK_KEY_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		link_key_reply = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&link_key_reply->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, key:", ba);
		for (int i = 0; i < 16; i++) {
			BLTS_DEBUG (
			    " 0x%x", link_key_reply->link_key[i]);
		}
		BLTS_DEBUG ("\n");
		break;

	case OCF_LINK_KEY_NEG_REPLY:
	case OCF_PIN_CODE_NEG_REPLY:
	case OCF_USER_CONFIRM_NEG_REPLY:
	case OCF_USER_PASSKEY_NEG_REPLY:
	case OCF_REMOTE_OOB_DATA_NEG_REPLY:
		baddr = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (baddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case OCF_PIN_CODE_REPLY:
		if (cmd->plen != PIN_CODE_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		pin_reply = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&pin_reply->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, pin: ", ba);
		for (int i = 0; i < pin_reply->pin_len; i++) {
			BLTS_DEBUG ("%d", pin_reply->pin_code[i]);
		}
		BLTS_DEBUG ("\n");
		break;

	case OCF_SET_CONN_PTYPE:
		if (cmd->plen != SET_CONN_PTYPE_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		conn_ptype = FUTE_CMD_PAYLOAD (data->payload);
		tmp = hci_ptypetostr (btohs (conn_ptype->pkt_type));
		BLTS_DEBUG ("\thandle: %d, ptype (0x%.4x): %s\n",
			    btohs (conn_ptype->handle),
			    btohs (conn_ptype->pkt_type),
			    tmp);
		break;

	case OCF_AUTH_REQUESTED:
		if (cmd->plen != AUTH_REQUESTED_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		auth_req = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n",
			    btohs (auth_req->handle));
		break;

	case OCF_SET_CONN_ENCRYPT:
		if (cmd->plen != SET_CONN_ENCRYPT_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		conn_encrypt = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d, encrypt: %d\n",
			    btohs (conn_encrypt->handle),
			    conn_encrypt->encrypt);
		break;

	case OCF_CHANGE_CONN_LINK_KEY:
		if (cmd->plen != CHANGE_CONN_LINK_KEY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		conn_key = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n",
			    btohs (conn_key->handle));
		break;

	case OCF_MASTER_LINK_KEY:
		if (cmd->plen != MASTER_LINK_KEY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		master_key = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tuse keys: %s\n",
			    master_key->key_flag == 0x00 ?
			    "Semi-permanent" :
			    "Temporary");
		break;

	case OCF_REMOTE_NAME_REQ:
		if (cmd->plen != REMOTE_NAME_REQ_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_name = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&remote_name->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n"
			    "\tpage scan repetition mode: %s\n"
			    "\tpscan mode (0x%.2x): %s\n"
			    "\tclock offset: 0x%.4x\n",
			    ba,
			    bt_fute_agent_pscan_rep_mode_to_str (
				remote_name->pscan_rep_mode),
			    remote_name->pscan_mode,
			    remote_name->pscan_mode != 0x00 ?
			    "ERROR" : "Reserved",
			    btohs (remote_name->clock_offset));
		break;

	case OCF_REMOTE_NAME_REQ_CANCEL:
		if (cmd->plen != REMOTE_NAME_REQ_CANCEL_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_name_cancel = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&remote_name_cancel->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case OCF_READ_REMOTE_FEATURES:
		if (cmd->plen != READ_REMOTE_FEATURES_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_feat = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n",
			    btohs (remote_feat->handle));
		break;

	case OCF_READ_REMOTE_EXT_FEATURES:
		if (cmd->plen != READ_REMOTE_EXT_FEATURES_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_ext_feat = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d, page num: 0x%.2x\n",
			    btohs (remote_ext_feat->handle),
			    remote_ext_feat->page_num);
		break;

	case OCF_READ_REMOTE_VERSION:
		if (cmd->plen != READ_REMOTE_VERSION_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_version = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n",
			    btohs (remote_version->handle));
		break;

	case OCF_READ_CLOCK_OFFSET:
		if (cmd->plen != READ_CLOCK_OFFSET_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		read_clock = FUTE_CMD_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n",
			    btohs (read_clock->handle));
		break;

	case OCF_READ_LMP_HANDLE:
		handle = FUTE_CMD_PAYLOAD (data->payload);
		if (handle)
			BLTS_DEBUG ("\thandle: %d\n", btohs (*handle));
		break;

	case OCF_SETUP_SYNC_CONN:
		if (cmd->plen != SETUP_SYNC_CONN_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sync_conn = FUTE_CMD_PAYLOAD (data->payload);
		tmp = hci_ptypetostr (btohs (sync_conn->pkt_type));
		BLTS_DEBUG ("\thandle: %d, tx bandwidth: %d, rx "
			    "bandwidth: %d\n\tmax latency: %d, "
			    "voice setting: 0x%.4x, retrans effort: %s"
			    "\n\tpacket types: %s\n",
			    btohs (sync_conn->handle),
			    btohl (sync_conn->tx_bandwith),
			    btohl (sync_conn->rx_bandwith),
			    btohs (sync_conn->max_latency),
			    btohs (sync_conn->voice_setting),
			    bt_fute_agent_retrans_effort_to_str (
				sync_conn->retrans_effort),
			    tmp);
		break;

	case OCF_ACCEPT_SYNC_CONN_REQ:
		if (cmd->plen != ACCEPT_SYNC_CONN_REQ_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sync_conn_req = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&sync_conn_req->bdaddr, ba);
		tmp = hci_ptypetostr (btohs (sync_conn_req->pkt_type));
		BLTS_DEBUG ("\taddress: %s\n\ttx bandwidth: %d, rx "
			    "bandwidth: %d, max latency: %d, "
			    "voice setting: 0x%.4x, retrans effort: %s"
			    "\n\tpacket types: %s\n",
			    ba,
			    btohl (sync_conn_req->tx_bandwith),
			    btohl (sync_conn_req->rx_bandwith),
			    btohs (sync_conn_req->max_latency),
			    btohs (sync_conn_req->voice_setting),
			    bt_fute_agent_retrans_effort_to_str (
				sync_conn_req->retrans_effort),
			    tmp);
		break;

	case OCF_REJECT_SYNC_CONN_REQ:
		if (cmd->plen != REJECT_SYNC_CONN_REQ_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		rej_sync_conn = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&rej_sync_conn->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, reason: %s\n",
			    ba,
			    FUTE_HCI_ERRORS[rej_sync_conn->reason]);
		break;

	case OCF_IO_CAPABILITY_REPLY:
		if (cmd->plen != IO_CAPABILITY_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		io_caps = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&io_caps->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n"
			    "\tcapability: %s, oob data present: %d\n"
			    "\tauthentication: %s\n",
			    ba,
			    bt_fute_agent_caps_to_str (
				io_caps->capability),
			    io_caps->oob_data,
			    bt_fute_agent_auth_to_str (
				io_caps->authentication));
		break;

	case OCF_USER_CONFIRM_REPLY:
		if (cmd->plen != USER_CONFIRM_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		confirm = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&confirm->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case OCF_USER_PASSKEY_REPLY:
		if (cmd->plen != USER_PASSKEY_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		pass = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&pass->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, passkey: %d\n",
			    ba, btohl (pass->passkey));
		break;

	case OCF_REMOTE_OOB_DATA_REPLY:
		if (cmd->plen != REMOTE_OOB_DATA_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		oob = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&oob->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n\thash: 0x", ba);
		for (int i = 0; i < 16; i++) {
			BLTS_DEBUG ("%.2x", oob->hash[i]);
		}
		BLTS_DEBUG ("\n\trandomizer: 0x");
		for (int i = 0; i < 16; i++) {
			BLTS_DEBUG ("%.2x", oob->randomizer[i]);
		}
		BLTS_DEBUG ("\n");
		break;

	case OCF_IO_CAPABILITY_NEG_REPLY:
		if (cmd->plen != IO_CAPABILITY_NEG_REPLY_CP_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		io_neg = FUTE_CMD_PAYLOAD (data->payload);
		ba2str (&io_neg->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, reason: %s\n", ba,
			    FUTE_HCI_ERRORS[io_neg->reason]);
		break;

	default:
		BLTS_DEBUG ("Unknown payload type\n");
		break;
	}

DONE:
	if (tmp)
		free (tmp);
	tmp = NULL;

	return ret;
}

/* Handle the payload details of a command. Essentially, all that we need to
 * do with these is to debug it.
 */
static int
bt_fute_agent_print_cmd_payload (
    BtFuteAgent *agent, bt_fute_hci_packet *data, hci_command_hdr *cmd)
{
	int ret = 0;
	int hci_ogf, hci_ocf;

	hci_ogf = cmd_opcode_ogf (cmd->opcode);
	hci_ocf = cmd_opcode_ocf (cmd->opcode);

	if (hci_ogf == OGF_LINK_CTL) {
		ret = bt_fute_agent_dump_link_ctl_cmd (hci_ocf, data, cmd);
	} else {
		BLTS_DEBUG ("Payload dump not implemented\n");
	}

	return ret;
}

/* Handle a command packet, specifically hci command packet. Digs out ogf and
 * ocf, hopefully at some point also a something from actual payload.
 */
static int
bt_fute_agent_handle_hci_command (
    BtFuteAgent *agent, bt_fute_hci_packet *data)
{
	int ret = 0;
	hci_command_hdr *hci_command = NULL;
	int hci_ocf = 0, hci_ogf = 0;
	const char *ocf_name;
	const char *ogf_name;

	BLTS_DEBUG ("command: ");
	hci_command = (void *)(data->payload + HCI_TYPE_LEN);

	hci_ocf = cmd_opcode_ocf (btohs (hci_command->opcode));
	hci_ogf = cmd_opcode_ogf (btohs (hci_command->opcode));

	/* hci_cmdtostr() doesn't work?? so have to do it manually :( */
	ogf_name = bt_fute_agent_opcode_to_ogf_str (hci_command->opcode);
	ocf_name = bt_fute_agent_opcode_to_ocf_str (hci_command->opcode);

	BLTS_DEBUG ("%s (0x%.2x) %s (0x%.4x) payload size %d\n",
		    ogf_name, hci_ogf, ocf_name, hci_ocf,
		    hci_command->plen);

	ret = bt_fute_agent_print_cmd_payload (agent, data, hci_command);

	return ret;
}

/* Handle a hci event. Print out the event name and all corresponding
 * data.
 */
static int
bt_fute_agent_handle_hci_event (BtFuteAgent *agent, bt_fute_hci_packet *data)
{
	hci_event_hdr *evt = NULL;

	/* Event payload structs */
	inquiry_info                          *inq;
	evt_conn_complete                     *conn_complete;
	evt_conn_request                      *conn_req;
	evt_disconn_complete                  *discon;
	evt_auth_complete                     *auth;
	evt_remote_name_req_complete          *remote_name;
	evt_encrypt_change                    *encrypt;
	evt_change_conn_link_key_complete     *link_key;
	evt_master_link_key_complete          *master_key;
	evt_read_remote_features_complete     *remote_feat;
	evt_read_remote_version_complete      *remote_version;
	evt_qos_setup_complete                *qos;
	evt_cmd_complete                      *cmd;
	evt_cmd_status                        *status;
	evt_hardware_error                    *hw_error;
	evt_flush_occured                     *flush;
	evt_role_change                       *role_change;
	evt_num_comp_pkts                     *comp_pkts;
	evt_mode_change                       *mode_change;
	evt_return_link_keys                  *return_keys;
	evt_pin_code_req                      *req_pin;
	evt_link_key_req                      *req_key;
	evt_link_key_notify                   *notify_key;
	evt_data_buffer_overflow              *buffer_overflow;
	evt_max_slots_change                  *slots_change;
	evt_read_clock_offset_complete        *clock_offset;
	evt_conn_ptype_changed                *ptype_changed;
	evt_qos_violation                     *qos_violation;
	evt_pscan_rep_mode_change             *pscan_rep_mode;
	evt_flow_spec_complete                *flow_spec;
	inquiry_info_with_rssi                *inq_rssi;
	inquiry_info_with_rssi_and_pscan_mode *inq_rssi_pscan;
	evt_read_remote_ext_features_complete *remote_ext_feat;
	evt_sync_conn_complete                *sync_conn_complete;
	evt_sync_conn_changed                 *sync_conn_changed;
	evt_sniff_subrating                   *sniff;
	extended_inquiry_info                 *ext_inq_info;
	evt_encryption_key_refresh_complete *encrypt_key_ref;
	evt_io_capability_request *io_cap_req;
	evt_io_capability_response *io_caps;
	evt_user_confirm_request *user_req;
	evt_user_passkey_request *passkey_req;
	evt_remote_oob_data_request *oob_req;
	evt_simple_pairing_complete *simple_pairing;
	evt_link_supervision_timeout_changed *sup_timeout;
	evt_enhanced_flush_complete *enh_flush;
	evt_user_passkey_notify *passkey_notify;
	evt_keypress_notify *keypress_notify;
	evt_remote_host_features_notify *remote_feats_notify;
	evt_le_meta_event *le_meta;
	evt_stack_internal *bluez_internal;

	uint8_t *ev_status;
	bdaddr_t *baddr;
	char *tmp = NULL;
	char ba[18] = {'\0'};
	uint16_t *handle;
	int handle_size, inquiry_size;
	uint8_t *count;

	/* Not defined in hci.h */
	typedef struct {
		bdaddr_t bdaddr;
		uint8_t  key[16];
	} __attribute__((packed)) a_link_key;

	a_link_key *a_key;
	int a_key_size;

	int ret = 0;

	BLTS_DEBUG ("event: ");
	evt = (void *)(data->payload + HCI_TYPE_LEN);

	BLTS_DEBUG ("%s (0x%.2x)\n", FUTE_EVENT_STR[evt->evt], evt->evt);

	/* Then the payload */
	switch (evt->evt) {
	case EVT_INQUIRY_COMPLETE:
		ev_status = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s\n", FUTE_HCI_ERRORS[*ev_status]);
		break;

	case EVT_INQUIRY_RESULT:
		count = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\t%d infos\n", *count);
		for (int i = 0; i < *count; i++) {
			BLTS_DEBUG ("\t---- info[%d] ----\n", i);
			inq = FUTE_EVENT_PAYLOAD (
			    data->payload + 1 + (i * INQUIRY_INFO_SIZE));
			memset (&ba, 0, sizeof (ba));
			ba2str (&inq->bdaddr, ba);
			if (tmp) {
				free (tmp);
				tmp = NULL;
			}
			BLTS_DEBUG ("\taddress: %s\n"
				    "\tpage scan repetition mode: %s\n"
				    "\tclass of device: 0x%.2x%.2x%.2x\n"
				    "\tclock offset: 0x%.4x\n",
				    ba,
				    bt_fute_agent_pscan_rep_mode_to_str (
					inq->pscan_rep_mode),
				    inq->dev_class[2], inq->dev_class[1],
				    inq->dev_class[0],
				    btohs (inq->clock_offset));
		}
		break;

	case EVT_CONN_COMPLETE:
#if 0
		/* BlueZ bug?? */
		if (evt->plen != EVT_CONN_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size (%d) doesn't "
				    "match expected (%d). "
				    "Api struct size is %d\n",
				    evt->plen, EVT_CONN_COMPLETE_SIZE,
				    sizeof (*conn_complete));
			ret = -1;
			goto DONE;
		}
#else
		if (evt->plen != sizeof (*conn_complete)) {
			BLTS_ERROR ("Error: Payload size (%d) doesn't "
				    "match expected (%d). "
				    "Api struct size is %d\n",
				    evt->plen, EVT_CONN_COMPLETE_SIZE,
				    sizeof (*conn_complete));
			ret = -1;
			goto DONE;
		}
#endif
		conn_complete = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&conn_complete->bdaddr, ba);
		BLTS_DEBUG ("\tstatus: %s\n"
			    "\thandle: %d, address: %s\n"
			    "\tlink type: %s, encryption mode: %d\n",
			    FUTE_HCI_ERRORS[conn_complete->status],
			    btohs (conn_complete->handle), ba,
			    conn_complete->link_type == 0x00 ? "SCO" : "ACL",
			    conn_complete->encr_mode);
		break;

	case EVT_CONN_REQUEST:
		if (evt->plen != EVT_CONN_REQUEST_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		conn_req = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&conn_req->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n"
			    "\tclass of device: 0x%.2x%.2x%.2x\n"
			    "\tlink type: %s\n",
			    ba, conn_req->dev_class[2], conn_req->dev_class[1],
			    conn_req->dev_class[0],
			    conn_req->link_type == 0x00 ? "SCO" : "ACL");
		break;

	case EVT_DISCONN_COMPLETE:
		if (evt->plen != EVT_DISCONN_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		discon = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s\n"
			    "\thandle: %d\n"
			    "\treason: %s\n",
			    FUTE_HCI_ERRORS[discon->status],
			    btohs (discon->handle),
			    FUTE_HCI_ERRORS[discon->reason]);
		break;

	case EVT_AUTH_COMPLETE:
		if (evt->plen != EVT_AUTH_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		auth = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n",
			    FUTE_HCI_ERRORS[auth->status],
			    btohs (auth->handle));
		break;

	case EVT_REMOTE_NAME_REQ_COMPLETE:
		if (evt->plen != EVT_REMOTE_NAME_REQ_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_name = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&remote_name->bdaddr, ba);
		BLTS_DEBUG ("\tstatus: %s\n\taddress: %s, name: %s\n",
			    FUTE_HCI_ERRORS[remote_name->status],
			    ba, remote_name->name);
		break;

	case EVT_ENCRYPT_CHANGE:
		if (evt->plen != EVT_ENCRYPT_CHANGE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		encrypt = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d, encrypt: %d\n",
			    FUTE_HCI_ERRORS[encrypt->status],
			    btohs (encrypt->handle), encrypt->encrypt);
		break;

	case EVT_CHANGE_CONN_LINK_KEY_COMPLETE:
		if (evt->plen != EVT_CHANGE_CONN_LINK_KEY_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		link_key = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n",
			    FUTE_HCI_ERRORS[link_key->status],
			    btohs (link_key->handle));
		break;

	case EVT_MASTER_LINK_KEY_COMPLETE:
		if (evt->plen != EVT_MASTER_LINK_KEY_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		master_key = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n\tkey flag: %s\n",
			    FUTE_HCI_ERRORS[master_key->status],
			    btohs (master_key->handle),
			    master_key->key_flag == 0x00 ?
			    "Using Semi-permanent Link key" :
			    "Using Temporary Link key");
		break;

	case EVT_READ_REMOTE_FEATURES_COMPLETE:
		if (evt->plen != EVT_READ_REMOTE_FEATURES_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_feat = FUTE_EVENT_PAYLOAD (data->payload);
		tmp = lmp_featurestostr (remote_feat->features, "\t", 64);
		BLTS_DEBUG ("\tstatus: %s, handle: %d, features: %s\n",
			    FUTE_HCI_ERRORS[remote_feat->status],
			    btohs (remote_feat->handle), tmp);
		break;

	case EVT_READ_REMOTE_VERSION_COMPLETE:
		if (evt->plen != EVT_READ_REMOTE_VERSION_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_version = FUTE_EVENT_PAYLOAD (data->payload);
		tmp = lmp_vertostr (remote_version->lmp_ver);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n\tversion: %s\n"
			    "\tcompany id: 0x%.4x, subversion: 0x%.4x\n",
			    FUTE_HCI_ERRORS[remote_version->status],
			    btohs (remote_version->handle),
			    tmp,
			    btohs (remote_version->manufacturer),
			    btohs (remote_version->lmp_subver));
		break;

	case EVT_QOS_SETUP_COMPLETE:
		if (evt->plen != EVT_QOS_SETUP_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		qos = FUTE_EVENT_PAYLOAD (data->payload);

		/* XXX: Skipping this one ... */

		break;

	case EVT_CMD_COMPLETE:
		if (evt->plen != EVT_CMD_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		cmd = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tHCI commands allowed: %d\n"
			    "\topcode (0x%.4x): %s (0x%.2x) %s (0x%.4x)\n",
			    cmd->ncmd, btohs (cmd->opcode),
			    bt_fute_agent_opcode_to_ogf_str (cmd->opcode),
			    cmd_opcode_ogf (btohs (cmd->opcode)),
			    bt_fute_agent_opcode_to_ocf_str (cmd->opcode),
			    cmd_opcode_ocf (btohs (cmd->opcode)));

		/* TODO:
		 * - Command return values to string
		 * - A test case to send commands, so we can see what happens
		 */
		break;

	case EVT_CMD_STATUS:
		if (evt->plen != EVT_CMD_STATUS_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		status = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, HCI commands allowed: %d\n"
			    "\topcode (0x%.4x): %s (0x%.2x) %s (0x%.4x)\n",
			    status->status == 0x00 ?
			    "Pending" : FUTE_HCI_ERRORS[status->status],
			    status->ncmd,
			    btohs (status->opcode),
			    bt_fute_agent_opcode_to_ogf_str (status->opcode),
			    cmd_opcode_ogf (btohs (status->opcode)),
			    bt_fute_agent_opcode_to_ocf_str (status->opcode),
			    cmd_opcode_ocf (btohs (status->opcode)));
		break;

	case EVT_HARDWARE_ERROR:
		if (evt->plen != EVT_HARDWARE_ERROR_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		hw_error = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thardware error: 0x%.2x\n", hw_error->code);
		break;

	case EVT_FLUSH_OCCURRED:
		if (evt->plen != EVT_FLUSH_OCCURRED_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		flush = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n", btohs (flush->handle));
		break;

	case EVT_ROLE_CHANGE:
		if (evt->plen != EVT_ROLE_CHANGE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		role_change = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&role_change->bdaddr, ba);
		BLTS_DEBUG ("\tstatus: %s, address: %s\n"
			    "\tnew role (0x%.2x): %s\n",
			    FUTE_HCI_ERRORS[role_change->status], ba,
			    role_change->role,
			    role_change->role == BT_FUTE_ROLE_MASTER ?
			    "Master" : "Slave");
		agent->role = role_change->role;
		break;

	case EVT_NUM_COMP_PKTS:
#if 0
		/* XXX: This can't work as the rest of the data is
		 *      of variable length
		 */
		if (evt->plen != EVT_NUM_COMP_PKTS_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
#endif
		comp_pkts = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tnumber of handles: %d\n", comp_pkts->num_hndl);

		handle_size = sizeof (uint16_t); /* Aka 2 octets */

		for (int i = 0; i < comp_pkts->num_hndl; i++) {
			handle = FUTE_EVENT_PAYLOAD (
			    data->payload + EVT_NUM_COMP_PKTS_SIZE +
			    (i * handle_size));
			if (handle) {
				BLTS_DEBUG ("\thandle[%d]: %d\n",
					    btohs (*handle));
			} else {
				BLTS_ERROR ("Error: Failed to get handle\n");
				ret = -1;
				goto DONE;
			}
		}
		break;

	case EVT_MODE_CHANGE:
		if (evt->plen != EVT_MODE_CHANGE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		mode_change = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n"
			    "\tcurrent mode (0x%.2x): %s, interval: %d\n",
			    FUTE_HCI_ERRORS[mode_change->status],
			    btohs (mode_change->handle),
			    mode_change->mode,
			    bt_fute_agent_mode_to_str (mode_change->mode),
			    btohs (mode_change->interval));
		break;

	case EVT_RETURN_LINK_KEYS:
#if 0
		/* This can't work as the link keys are of variable length,
		 * based on how many of them are returned.
		 */
		if (evt->plen != EVT_RETURN_LINK_KEYS_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
#endif
		return_keys = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tnumber of keys: %d\n", return_keys->num_keys);

		a_key_size = sizeof (*a_key);

		for (int i = 0; i < return_keys->num_keys; i++) {
			a_key = FUTE_EVENT_PAYLOAD (
			    data->payload + EVT_RETURN_LINK_KEYS_SIZE +
			    (i * a_key_size));
			memset (&ba, 0, sizeof (ba));
			ba2str (&a_key->bdaddr, ba);
			BLTS_DEBUG ("\taddress[%d]: %s\n\tkey[%d]: 0x",
				    i, ba, i);

			for (int j = 15; j >= 0; j--) {
				BLTS_DEBUG ("%.2x", a_key->key[j]);
			}
			BLTS_DEBUG ("\n");
		}
		
		break;

	case EVT_PIN_CODE_REQ:
		if (evt->plen != EVT_PIN_CODE_REQ_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		req_pin = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&req_pin->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case EVT_LINK_KEY_REQ:
		if (evt->plen != EVT_LINK_KEY_REQ_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		req_key = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&req_key->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case EVT_LINK_KEY_NOTIFY:
		if (evt->plen != EVT_LINK_KEY_NOTIFY_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		notify_key = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&notify_key->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, link key: 0x", ba);

		for (int i = 15; i >= 0; i--) {
			BLTS_DEBUG ("%.2x", notify_key->link_key[i]);
		}
		BLTS_DEBUG ("\n\tkey type (0x%.2x): %s\n",
			    notify_key->key_type,
			    bt_fute_agent_key_type_to_str (
				notify_key->key_type));
		break;

	case EVT_LOOPBACK_COMMAND:
		/* This is an interesting one, need to handle the payload,
		 * like it's a command (to some extent). This is due to
		 * how it's defined, the loopback echoes back all commands
		 * sent to the adapter if the loopback mode is enabled.
		 */

		/* XXX: Skipping this for now, not a high priority one...
		 *      handling this should be relatively simple though
		 */
		break;

	case EVT_DATA_BUFFER_OVERFLOW:
		if (evt->plen != EVT_DATA_BUFFER_OVERFLOW_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		buffer_overflow = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tlink type (0x%.2x): %s\n",
			    buffer_overflow->link_type,
			    buffer_overflow->link_type == 0x00 ?
			    "Synchronous buffer oveflow (voice channels)" :
			    "ACL buffer overflow (data channels)");
		break;

	case EVT_MAX_SLOTS_CHANGE:
		if (evt->plen != EVT_MAX_SLOTS_CHANGE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		slots_change = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d, max slots: 0x%.2x\n",
			    btohs (slots_change->handle),
			    slots_change->max_slots);
		break;

	case EVT_READ_CLOCK_OFFSET_COMPLETE:
		if (evt->plen != EVT_READ_CLOCK_OFFSET_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		clock_offset = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d, clock offset: 0x%.4x\n",
			    FUTE_HCI_ERRORS[clock_offset->status],
			    btohs (clock_offset->handle),
			    btohs (clock_offset->clock_offset));
		break;

	case EVT_CONN_PTYPE_CHANGED:
		if (evt->plen != EVT_CONN_PTYPE_CHANGED_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		ptype_changed = FUTE_EVENT_PAYLOAD (data->payload);
		tmp = hci_ptypetostr (btohs (ptype_changed->ptype));
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n\tpacket types: %s\n",
			    FUTE_HCI_ERRORS[ptype_changed->status],
			    btohs (ptype_changed->handle), tmp);
		break;

	case EVT_QOS_VIOLATION:
		if (evt->plen != EVT_QOS_VIOLATION_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		qos_violation = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\thandle: %d\n", btohs (qos_violation->handle));
		break;

	case EVT_PSCAN_REP_MODE_CHANGE:
		if (evt->plen != EVT_PSCAN_REP_MODE_CHANGE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		pscan_rep_mode = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&pscan_rep_mode->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, page scan repetition mode "
			    "(0x%.2x): %s\n", ba,
			    pscan_rep_mode->pscan_rep_mode,
			    bt_fute_agent_pscan_rep_mode_to_str (
				pscan_rep_mode->pscan_rep_mode));
		break;

	case EVT_FLOW_SPEC_COMPLETE:
		if (evt->plen != EVT_FLOW_SPEC_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		flow_spec = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d, direction (0x%.2x): "
			    "%s\n", FUTE_HCI_ERRORS[flow_spec->status],
			    btohs (flow_spec->handle), flow_spec->direction,
			    flow_spec->direction == 0x00 ?
			    "Outgoing flow" :
			    "Incoming flow");
		/* XXX: Rest of the QoS parameters */
		break;

	case EVT_INQUIRY_RESULT_WITH_RSSI:
		/* This event *should* be generated only if the inquiry mode
		 * parameter of the last write inquiry mode command was set
		 * to 0x01. So, essentially, or potentially, this is not of
		 * concern to us?
		 *
		 * Anyhow, BlueZ api seems to have two different structs
		 * defined for the payload, which means that handling the
		 * event is a bit interesting to say the least...
		 */
		count = FUTE_EVENT_PAYLOAD (data->payload);
		if (!count) {
			BLTS_ERROR ("Error: couldn't get inquiry result count\n");
			ret = -1;
			goto DONE;
		}

		BLTS_DEBUG ("\tnumber of results: %d\n", *count);

		if (*count == 0) {
			BLTS_ERROR ("Error: no results\n");
			ret = -1;
			goto DONE;
		}

		inquiry_size = (evt->plen - 1) / *count;

		BLTS_DEBUG ("\tresult size should be %d\n", inquiry_size);

		for (int i = 0; i < *count; i++) {
			BLTS_DEBUG ("\t--- result[%d] ---\n", i);
			memset (&ba, 0, sizeof (ba));

			if (inquiry_size == INQUIRY_INFO_WITH_RSSI_SIZE) {
				inq_rssi = FUTE_EVENT_PAYLOAD (
				    data->payload + 1 +
				    (i * INQUIRY_INFO_WITH_RSSI_SIZE));
				ba2str (&inq_rssi->bdaddr, ba);
				BLTS_DEBUG (
				    "\taddress: %s, page scan "
				    "repetition mode: %s\n"
				    "\tpage scan period mode: 0x%.2x\n"
				    "\tdevice class: 0x%.2x%.2x%.2x, "
				    "clock offset: 0x%.4x, rssi: %d\n",
				    ba,
				    bt_fute_agent_pscan_rep_mode_to_str (
					inq_rssi->pscan_rep_mode),
				    inq_rssi->pscan_period_mode,
				    inq_rssi->dev_class[2],
				    inq_rssi->dev_class[1],
				    inq_rssi->dev_class[0],
				    btohs (inq_rssi->clock_offset),
				    inq_rssi->rssi);
			} else if (
			    inquiry_size ==
			    INQUIRY_INFO_WITH_RSSI_AND_PSCAN_MODE_SIZE) {
				inq_rssi_pscan = FUTE_EVENT_PAYLOAD (
				    data->payload + 1 +
				    (i * INQUIRY_INFO_WITH_RSSI_AND_PSCAN_MODE_SIZE));
				ba2str (&inq_rssi_pscan->bdaddr, ba);
				BLTS_DEBUG (
				    "\taddress: %s, page scan "
				    "repetition mode: %s\n"
				    "\tpage scan period mode: 0x%.2x, "
				    "page scan mode: 0x%.2x\n"
				    "\tdevice class: 0x%.2x%.2x%.2x, "
				    "clock offset: 0x%.4x, rssi: %d\n",
				    ba,
				    bt_fute_agent_pscan_rep_mode_to_str (
					inq_rssi_pscan->pscan_rep_mode),
				    inq_rssi_pscan->pscan_period_mode,
				    inq_rssi_pscan->pscan_mode,
				    inq_rssi_pscan->dev_class[2],
				    inq_rssi_pscan->dev_class[1],
				    inq_rssi_pscan->dev_class[0],
				    btohs (inq_rssi_pscan->clock_offset),
				    inq_rssi_pscan->rssi);
			} /* Else we fail */
		}
		break;

	case EVT_READ_REMOTE_EXT_FEATURES_COMPLETE:
		if (evt->plen != EVT_READ_REMOTE_EXT_FEATURES_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_ext_feat = FUTE_EVENT_PAYLOAD (data->payload);
		tmp = lmp_featurestostr (remote_ext_feat->features, "\t", 64);
		BLTS_DEBUG ("\tstatus: %s, handle: %d, page number: 0x%.2d\n"
			    "\tmaximum page number: 0x%.2x\n"
			    "\textended features: %s\n",
			    FUTE_HCI_ERRORS[remote_ext_feat->status],
			    btohs (remote_ext_feat->handle),
			    remote_ext_feat->page_num,
			    remote_ext_feat->max_page_num,
			    tmp);
		break;

	case EVT_SYNC_CONN_COMPLETE:
		if (evt->plen != EVT_SYNC_CONN_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sync_conn_complete = FUTE_EVENT_PAYLOAD (data->payload);

		ba2str (&sync_conn_complete->bdaddr, ba);

		switch (sync_conn_complete->link_type) {
		case 0x00: tmp = "SCO"; break;
		case 0x02: tmp = "eSCO"; break;
		default: tmp = "Reserved"; break;
		}

		BLTS_DEBUG ("\tstatus: %s, handle: %d, address: %s\n"
			    "\tlink type (0x%.2x): %s, transmission interval: "
			    "%d, retransmission window: %d\n"
			    "\trx packet length: %d, tx packet length: %d\n"
			    "\tair mode (0x%.2x): %s\n",
			    FUTE_HCI_ERRORS[sync_conn_complete->status],
			    btohs (sync_conn_complete->handle), ba,
			    sync_conn_complete->link_type, tmp,
			    sync_conn_complete->trans_interval,
			    sync_conn_complete->retrans_window,
			    btohs (sync_conn_complete->rx_pkt_len),
			    btohs (sync_conn_complete->tx_pkt_len),
			    sync_conn_complete->air_mode,
			    bt_fute_agent_air_mode_to_str (
				sync_conn_complete->air_mode));
		break;

	case EVT_SYNC_CONN_CHANGED:
		if (evt->plen != EVT_SYNC_CONN_CHANGED_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sync_conn_changed = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n"
			    "\ttransmission interval: %d, retransmission "
			    "window: %d\n"
			    "\trx packet length: %d, tx packet length: %d\n",
			    FUTE_HCI_ERRORS[sync_conn_changed->status],
			    btohs (sync_conn_changed->handle),
			    sync_conn_changed->trans_interval,
			    sync_conn_changed->retrans_window,
			    btohs (sync_conn_changed->rx_pkt_len),
			    btohs (sync_conn_changed->tx_pkt_len));
		break;

	case EVT_SNIFF_SUBRATING:
		if (evt->plen != EVT_SNIFF_SUBRATING_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sniff = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n"
			    "\tmax tx latency: %d, max rx latency: %d\n"
			    "\tmin remote timeout: %d, min local timeout: "
			    "%d\n",
			    FUTE_HCI_ERRORS[sniff->status],
			    btohs (sniff->handle),
			    btohs (sniff->max_tx_latency),
			    btohs (sniff->max_rx_latency),
			    btohs (sniff->min_remote_timeout),
			    btohs (sniff->min_local_timeout));
		break;

	case EVT_EXTENDED_INQUIRY_RESULT:
		count = FUTE_EVENT_PAYLOAD (data->payload);
		if (!count) {
			BLTS_ERROR ("Error: Couldn't get result count\n");
			ret = -1;
			goto DONE;
		}

		BLTS_DEBUG ("\tnumber of results: %d\n", *count);

		/* According to specs, extended inquiry results always contain
		 * just one result...
		 */
		ext_inq_info = FUTE_EVENT_PAYLOAD (data->payload + 1);

		ba2str (&ext_inq_info->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, page scan repetition mode: %s\n"
			    "\tpage scan period mode: 0x%.2x\n"
			    "\tdevice class: 0x%.2x%.2x%.2x, clock offset: "
			    "0x%.4x\n"
			    "\trssi: %d\n", ba,
			    bt_fute_agent_pscan_rep_mode_to_str (
				ext_inq_info->pscan_rep_mode),
			    ext_inq_info->pscan_period_mode,
			    ext_inq_info->dev_class[2],
			    ext_inq_info->dev_class[1],
			    ext_inq_info->dev_class[0],
			    ext_inq_info->rssi);

		/* TODO: The data field */
		break;

	case EVT_ENCRYPTION_KEY_REFRESH_COMPLETE:
		if (evt->plen != EVT_ENCRYPTION_KEY_REFRESH_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		encrypt_key_ref = FUTE_EVENT_PAYLOAD (data->payload);
		BLTS_DEBUG ("\tstatus: %s, handle: %d\n",
			    FUTE_HCI_ERRORS[encrypt_key_ref->status],
			    btohs (encrypt_key_ref->handle));
		break;

	case EVT_IO_CAPABILITY_REQUEST:
		if (evt->plen != EVT_IO_CAPABILITY_REQUEST_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		io_cap_req = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&io_cap_req->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s\n", ba);
		break;

	case EVT_IO_CAPABILITY_RESPONSE:
		if (evt->plen != EVT_IO_CAPABILITY_RESPONSE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		io_caps = FUTE_EVENT_PAYLOAD (data->payload);
		ba2str (&io_caps->bdaddr, ba);
		BLTS_DEBUG ("\taddress: %s, capability (0x%.2x): %s\n"
			    "\toob data present: 0x%.2x\n"
			    "\tauthentication (0x%.2x): %s\n",
			    ba, io_caps->capability,
			    bt_fute_agent_caps_to_str (io_caps->capability),
			    io_caps->oob_data, io_caps->authentication,
			    bt_fute_agent_auth_to_str (
				io_caps->authentication));
		break;

	case EVT_USER_CONFIRM_REQUEST:
		if (evt->plen != EVT_USER_CONFIRM_REQUEST_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		user_req = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_USER_PASSKEY_REQUEST:
		if (evt->plen != EVT_USER_PASSKEY_REQUEST_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		passkey_req = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_REMOTE_OOB_DATA_REQUEST:
		if (evt->plen != EVT_REMOTE_OOB_DATA_REQUEST_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		oob_req = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_SIMPLE_PAIRING_COMPLETE:
		if (evt->plen != EVT_SIMPLE_PAIRING_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		simple_pairing = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_LINK_SUPERVISION_TIMEOUT_CHANGED:
		if (evt->plen != EVT_LINK_SUPERVISION_TIMEOUT_CHANGED_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		sup_timeout = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_ENHANCED_FLUSH_COMPLETE:
		if (evt->plen != EVT_ENHANCED_FLUSH_COMPLETE_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		enh_flush = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_USER_PASSKEY_NOTIFY:
		if (evt->plen != EVT_USER_PASSKEY_NOTIFY_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		passkey_notify = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_KEYPRESS_NOTIFY:
		if (evt->plen != EVT_KEYPRESS_NOTIFY_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		keypress_notify = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_REMOTE_HOST_FEATURES_NOTIFY:
		if (evt->plen != EVT_REMOTE_HOST_FEATURES_NOTIFY_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		remote_feats_notify = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_LE_META_EVENT:
		if (evt->plen != EVT_LE_META_EVENT_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		le_meta = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	case EVT_TESTING:
		break;

	case EVT_VENDOR:
		break;

	case EVT_STACK_INTERNAL:
		if (evt->plen != EVT_STACK_INTERNAL_SIZE) {
			BLTS_ERROR ("Error: Payload size doesn't "
				    "match expected\n");
			ret = -1;
			goto DONE;
		}
		bluez_internal = FUTE_EVENT_PAYLOAD (data->payload);
		break;

	default:
		break;
	}

DONE:
	if (tmp)
		free (tmp);
	tmp = NULL;

	return ret;
}

/* Handle the data, both incoming and outgoing. If we're trying to do pairing,
 * then reply to events accordingly.
 */
static int
bt_fute_agent_handle_data (BtFuteAgent *agent, bt_fute_hci_packet *data)
{
	int ret = 0;
	uint8_t hci_type = 0;

	if (!agent) {
		BLTS_DEBUG ("No agent given in\n");
		return -1;
	}

	pthread_mutex_lock (&agent->mutex);

	FUNC_ENTER();

	if (data->in) {
		BLTS_DEBUG ("(%d.%.6d) Incoming ",
			    data->time_sec, data->time_usec);
	} else {
		BLTS_DEBUG ("(%d.%.6d) Outgoing ",
			    data->time_sec, data->time_usec);
	}

	if (!data->payload) {
		BLTS_DEBUG ("(no payload)\n");
		return 0;
	}

	hci_type = data->payload[0];

	switch (hci_type) {
	case HCI_COMMAND_PKT:
		ret = bt_fute_agent_handle_hci_command (agent, data);
		break;

	case HCI_ACLDATA_PKT:
		BLTS_DEBUG ("ACL data: \n");
		break;

	case HCI_SCODATA_PKT:
		BLTS_DEBUG ("SCO data: \n");
		break;

	case HCI_EVENT_PKT:
		ret = bt_fute_agent_handle_hci_event (agent, data);
		break;

	case HCI_VENDOR_PKT:
		BLTS_DEBUG ("vendor packet: \n");
		break;

	default:
		BLTS_DEBUG ("something I don't recognize\n");
		break;
	}

	FUNC_LEAVE();

	pthread_mutex_unlock (&agent->mutex);

	return ret;
}

/* If we're logging, write the data */
static int
bt_fute_agent_write_data (BtFuteAgent *agent, bt_fute_hci_packet *data)
{
	return 0;
}

/* Thread run function */
static void *
bt_fute_run_thread (void *data)
{
	BtFuteAgent *agent;
	struct pollfd fds[1];
	int ret = 0;
	bt_fute_hci_packet packet;

	FUNC_ENTER();

	if (!data) {
		BLTS_ERROR ("No user data\n");
		return (void *)-1;
	}

	agent = data;

	fds[0].fd      = agent->hci_fd;
	fds[0].events  = POLLIN;
	fds[0].revents = 0;

	while (!bt_fute_get_stop_thread (agent)) {

		ret = poll (fds, 1, 1000);

		if (ret <= 0) {
			BLTS_TRACE ("No data for one second\n");
			continue;
		}

		if (fds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
			BLTS_DEBUG ("Socket disconnected\n");
			break;
		}

		/* Process incoming and outgoing data */
		memset (&packet, 0, sizeof (packet));
		ret = bt_fute_agent_read_data (agent, &packet);

		if (ret < 0) {
			BLTS_ERROR ("Socket data read error\n");
			break;
		}

		/* Handle the packet data */
		ret = bt_fute_agent_handle_data (agent, &packet);

		if (ret < 0) {
			BLTS_ERROR ("Failed to handle data\n");
			break;
		}

		/* Write the data to log if we have a log file */
		ret = bt_fute_agent_write_data (agent, &packet);

		if (ret < 0) {
			BLTS_ERROR ("Failed to write data\n");
			break;
		}

		/* Free the payload, it's the only portion dynamically
		 * allocated
		 */
		if (packet.payload) {
			free (packet.payload);
			packet.payload = NULL;
		}
	}

	if (ret) {
		BLTS_ERROR ("Thread run function has a failure\n");
		return (void *)-1;
	}

	FUNC_LEAVE();

	return NULL;
}

/* Public functions */

/* Create new instance, free with bt_fute_agent_unref() */
BtFuteAgent *
bt_fute_agent_new (int adapter)
{
	BtFuteAgent *agent;

	FUNC_ENTER();

	if (adapter < 0) {
		BLTS_DEBUG ("No adapter given, using first one available\n");
		adapter = hci_get_route (NULL);
	}

	if (adapter < 0) {
		BLTS_LOGGED_PERROR ("Failed to resolve adapter");
		return NULL;
	}

	agent = malloc (sizeof (BtFuteAgent));

	if (!agent) {
		BLTS_LOGGED_PERROR ("malloc failure");
		return NULL;
	}

	/* Adapter & dev info */
	agent->adapter = adapter;
	agent->hci_fd  = 0;
	agent->role    = BT_FUTE_ROLE_UNDEFINED;

	/* Thread data */
	agent->thread_id = 0;
	agent->stop      = 0;

	if (pthread_mutex_init (&agent->mutex, NULL) < 0) {
		BLTS_LOGGED_PERROR ("Failed to init mutex!");
		free (agent);
		return NULL;
	}

	/* Ref count is 1 */
	agent->ref_count = 1;

	FUNC_LEAVE();

	return agent;
}

/* Ref, aka copy the instance (if needed) */
BtFuteAgent *
bt_fute_agent_ref (BtFuteAgent *agent)
{
	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return NULL;
	}

	pthread_mutex_lock (&agent->mutex);

	agent->ref_count++;

	pthread_mutex_unlock (&agent->mutex);

	FUNC_LEAVE();

	return agent;
}

/* Unref the instance, NULL is returned if the agent was freed */
BtFuteAgent *
bt_fute_agent_unref (BtFuteAgent *agent)
{
	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return NULL;
	}

	pthread_mutex_lock (&agent->mutex);

	agent->ref_count--;

	if (agent->ref_count > 0) {
		pthread_mutex_unlock (&agent->mutex);

		FUNC_LEAVE();

		return agent;
	}

	BLTS_DEBUG ("Freeing agent\n");

	if (agent->hci_fd) {
		BLTS_DEBUG ("Closing adapter...");
		close (agent->hci_fd);
		agent->hci_fd = 0;
		BLTS_DEBUG ("done\n");
	}

	if (agent->thread_id) {
		pthread_mutex_unlock (&agent->mutex);
		bt_fute_agent_stop (agent);
	} else {
		pthread_mutex_unlock (&agent->mutex);
	}

	pthread_mutex_destroy (&agent->mutex);
	free (agent);

	BLTS_DEBUG ("Agent freed\n");

	FUNC_LEAVE();

	return NULL;
}

/* Run the agent (starts a thread if it's not having one already)
 *
 * Returns: Zero for success, negative for failure
 */
int
bt_fute_agent_run (BtFuteAgent *agent)
{
	int ret = 0;

	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return -1;
	}

	if (agent->thread_id) {
		BLTS_ERROR ("Thread is already running\n");
		ret = -1;
		goto DONE;
	}

	if (agent->adapter < 0) {
		BLTS_ERROR ("No valid adapter id has been given\n");
		ret = -1;
		goto DONE;
	}

	if (bt_fute_open_hci_dev (agent) < 0) {
		BLTS_ERROR ("Device open failed\n");
		ret = -1;
		goto DONE;
	}

	ret = pthread_create (
	    &agent->thread_id, NULL, bt_fute_run_thread, agent);

	if (ret) {
		BLTS_LOGGED_PERROR ("Failed to create thread");
		agent->thread_id = 0;
		ret = -1;
		goto DONE;
	}

DONE:
	FUNC_LEAVE();

	return ret;
}

/* Stop the agent */
int
bt_fute_agent_stop (BtFuteAgent *agent)
{
	int ret = 0;
	int thr_return = 0;

	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return -1;
	}

	pthread_mutex_lock (&agent->mutex);

	agent->stop = 1;

	pthread_mutex_unlock (&agent->mutex);

	ret = pthread_join (agent->thread_id, (void **) &thr_return);
	agent->thread_id = 0;

	if (ret) {
		BLTS_LOGGED_PERROR ("Thread join failed");
	}

	if (thr_return) {
		BLTS_DEBUG ("Thread returns with %d\n", thr_return);
		ret = thr_return;
	}

	FUNC_LEAVE();

	return ret;
}

/* Get the agent role at the moment */
BtFuteAgentRole
bt_fute_agent_get_role (BtFuteAgent *agent)
{
	BtFuteAgentRole role = BT_FUTE_ROLE_UNDEFINED;

	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return BT_FUTE_ROLE_UNDEFINED;
	}

	pthread_mutex_lock (&agent->mutex);

	role = agent->role;

	pthread_mutex_unlock (&agent->mutex);

	FUNC_LEAVE();

	return role;
}

/* Set the desired role, this might not happen though... */
int
bt_fute_agent_set_role (BtFuteAgent *agent, BtFuteAgentRole role)
{
	int ret = 0;

	FUNC_ENTER();

	if (!agent) {
		BLTS_ERROR ("Error: No agent given in\n");
		return -1;
	}

	pthread_mutex_lock (&agent->mutex);

	/* It's a role change... but agent only handles incoming commands,
	 * not really outgoing.
	 */
	if (role != agent->role) {
		agent->role = role;

		/* XXX: Or should this do a role change? */
	}

	pthread_mutex_unlock (&agent->mutex);

	FUNC_LEAVE();

	return ret;
}
