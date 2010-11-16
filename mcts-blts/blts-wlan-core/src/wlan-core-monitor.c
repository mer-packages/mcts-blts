/* wlan-core-monitor.c -- Monitor interface handling

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

#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <blts_log.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "wlan-core-monitor.h"
#include "wlan-core-utils.h"
#include "radiotap.h"
#include "radiotap_iter.h"

/* Helper for nl80211_create_monitor_iface: form monitor interface
 * name from current test state. */
static char *mk_monitor_ifname(wlan_core_data *data)
{
	int ret;
	char *newname = NULL;
	ret = asprintf(&newname, "mon.%s", data->cmd->ifname);
	if (ret < 0)
		newname = NULL;
	return newname;
}

/*
 * This creates a mon.wlanX interface corresponding to the WLAN ifce
 * currently tested. We'll still need to create socket(s) and bring
 * the ifce up to actually do anything with it. This is basically
 * nl80211_create_interface_once from hostapd.
 */
int nl80211_create_monitor_iface(wlan_core_data *data)
{
 	struct nl_msg *msg;
	char *mon_ifname;
	int ifidx, ret = 0;

	mon_ifname = mk_monitor_ifname(data);
	if(!mon_ifname)
		return -ENOMEM;

	msg = nlmsg_alloc();
	if(!msg) {
		free(mon_ifname);
		return -ENOMEM;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
		0, NL80211_CMD_NEW_INTERFACE, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
	NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, mon_ifname);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_MONITOR);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);
	if(ret) {

 nla_put_failure:
		if(ret == -ENFILE) {
			BLTS_DEBUG("Reusing existing monitor interface %s\n", mon_ifname);
			data->cmd->monitor_ifidx = if_nametoindex(mon_ifname);
			data->cmd->monitor_persist = 1;
			ret = 0;
		} else {
			BLTS_ERROR("Failed to create interface %s: %d (%s)\n",
				mon_ifname, ret, strerror(-ret));
		}
		free(mon_ifname);
		return ret;
	}

	ifidx = if_nametoindex(mon_ifname);
	BLTS_DEBUG("Monitor interface %s created: ifindex=%d\n",
		mon_ifname, ifidx);
	data->cmd->monitor_ifidx = ifidx;

	free(mon_ifname);
	if(ifidx <= 0)
		return -1;
	return ifidx;
}

void nl80211_remove_monitor_iface(wlan_core_data *data)
{
	struct nl_msg *msg;
	char ifname[IFNAMSIZ] = {};
	if(data->cmd->monitor_persist) {
		if_indextoname(data->cmd->monitor_ifidx, ifname);
		BLTS_DEBUG("Not removing previously existing monitor interface %s\n",
			ifname);
		return;
	}
	msg = nlmsg_alloc();
	if(!msg)
		goto nla_put_failure;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
		    0, NL80211_CMD_DEL_INTERFACE, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->monitor_ifidx);

	if(send_and_recv_msgs(data, msg, NULL, NULL) == 0) {
		data->cmd->monitor_ifidx = 0;
		return;
	}
 nla_put_failure:
	BLTS_ERROR("Failed to remove monitor interface (ifidx=%d)\n", data->cmd->monitor_ifidx);
}

/* Panic option: if we could not bring up a real monitor socket we may
   still be able to get our raw EAPOL frames over the real
   interface. If we can't get filtered packets here, we give up. */
static int monitor_sock_setup_alternate(wlan_core_data *data)
{
	int sock, ret = -EIO;
	struct sockaddr_ll ll;
	int optval;
	socklen_t optlen;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_EAPOL));
	if(sock < 0) {
		int err = errno;
		BLTS_ERROR("Error - Failed to open raw socket on wireless interface - %s (%d)\n",
			strerror(err), err);
		return -err;
	}

	memset(&ll, 0, sizeof ll);
	ll.sll_family = PF_PACKET;
	ll.sll_protocol = htons(ETH_P_EAPOL);
	ll.sll_ifindex = data->cmd->ifindex;
	ll.sll_hatype = ARPHRD_IEEE80211;
	ll.sll_pkttype = PACKET_HOST;
	/* memset(&ll, 0, sizeof ll); */
	/* ll.sll_family = AF_PACKET; */
	/* ll.sll_ifindex = data->cmd->ifindex; */

	ret = bind(sock, (struct sockaddr *) &ll, sizeof ll);
	if(ret < 0) {
		int err = errno;
		BLTS_ERROR("Error - Failed to bind socket on wireless interface - %s (%d)\n",
			strerror(err), err);
		return -err;
	}
	/* Bump priority. */
	optlen = sizeof(optval);
	optval = 20;
	ret = setsockopt(sock, SOL_SOCKET, SO_PRIORITY, &optval, optlen);
	if(ret) {
		int err = errno;
		BLTS_ERROR("Error - Failed to adjust raw socket priority - %s (%d)\n",
			strerror(err), err);
		close(sock);
		return -err;
	}

	data->monitor_sock = sock;
	data->monitor_alternate_in_use = 1;

	return ret;
}


/* This sets up a socket for currently used monitor interface. */
int monitor_sock_setup(wlan_core_data *data)
{
	int sock, ret = 0;
	char ifname[IFNAMSIZ] = {};
	struct sockaddr_ll ll;
	int optval;
	socklen_t optlen;

	if(data->cmd->monitor_ifidx <= 0) {
		BLTS_ERROR("Error - Monitor interface not available\n");
		return -ENOENT;
	}

	/* == ifconfig mon.wlanX up */
	if(!if_indextoname(data->cmd->monitor_ifidx, ifname)) {
		BLTS_ERROR("Error - Invalid monitor interface name\n");
		return -EBADF;
	}

	ret = set_iface_flags(data->ioctl_sock, ifname, 1);
	if(ret) {
		BLTS_ERROR("Error - Failed to bring up monitor interface. Using alternate RX (EAPOL only)\n");
		return monitor_sock_setup_alternate(data);
	}

	/* Now, open and bind the socket */

	memset(&ll, 0, sizeof ll);
	ll.sll_family = AF_PACKET;
	ll.sll_ifindex = data->cmd->monitor_ifidx;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sock < 0) {
		int err = errno;
		BLTS_ERROR("Error - Failed opening monitor socket - %s (%d)\n",
			strerror(err), err);
		return -err;
	}

	ret = bind(sock, (struct sockaddr *) &ll, sizeof ll);
	if(ret < 0) {
		int err = errno;
		BLTS_ERROR("Error - Failed bind() on monitor socket - %s (%d)\n",
			strerror(err), err);
		close(sock);
		return -err;
	}

	/* Bump priority. */
	optlen = sizeof(optval);
	optval = 20;
	ret = setsockopt(sock, SOL_SOCKET, SO_PRIORITY, &optval, optlen);
	if(ret) {
		int err = errno;
		BLTS_ERROR("Error - Failed to adjust monitor socket priority - %s (%d)\n",
			strerror(err), err);
		close(sock);
		return -err;
	}

	data->monitor_sock = sock;

	return ret;
}

void monitor_sock_cleanup(wlan_core_data *data)
{
	if (data->monitor_sock > 0)
		close(data->monitor_sock);
	data->monitor_sock = 0;
	data->monitor_alternate_in_use = 0;
}

/*
 * This keeps track of sequence numbering for raw frames we send, for
 * those cases where it matters.
 */
static void update_tx_seq(wlan_core_data *data, void *frame)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) frame;

	if(!((hdr->frame_control) & WLAN_FC_TODS)
		|| ((hdr->frame_control) & WLAN_FC_FROMDS))
		return; /* Not TX */
	if(memcmp(hdr->addr2, data->our_addr, ETH_ALEN))
		return; /* Not ours */

	/* wire: ffffssss ssssssss (f = frag, s = seq)*/
	data->tx_last_seq = le_to_host16(hdr->seq_ctrl) >> 4;
}

/*
 * Unpack raw radiotap frame for further processing. Adapted from
 * hostapd handle_monitor_read(). buf is the received radiotap frame
 * (sz == buf_sz), *frame is where the extracted frame starts,
 * frame_len gets length of that frame. Note that we don't copy
 * anything, the pointer is just the offset.
 */
int monitor_extract_frame(wlan_core_data *data, u8 *buf, unsigned buf_sz, u8 **frame, unsigned *frame_len)
{
	struct ieee80211_radiotap_iterator iter;
	int len = buf_sz, ret = 0;
	int datarate = 0, ssi_signal = 0;
	int injected = 0, failed = 0, rxflags = 0;

	if(!buf || !buf_sz || !frame || !frame_len) {
		BLTS_ERROR("BUG: Bad args to %s\n", __FUNCTION__);
		return -EINVAL;
	}

	if(ieee80211_radiotap_iterator_init(&iter, (void*)buf, buf_sz)) {
		BLTS_ERROR("Monitor: Received invalid radiotap frame\n");
		return -1;
	}

	for(;;) {
		ret = ieee80211_radiotap_iterator_next(&iter);
		if (ret == -ENOENT)
			break;
		if (ret) {
			BLTS_ERROR("Monitor: Received invalid radiotap frame (%d)\n", ret);
			return -1;
		}
		switch (iter.this_arg_index) {
		case IEEE80211_RADIOTAP_FLAGS:
			if (*iter.this_arg & IEEE80211_RADIOTAP_F_FCS)
				len -= 4;
			break;
		case IEEE80211_RADIOTAP_RX_FLAGS:
			rxflags = 1;
			break;
		case IEEE80211_RADIOTAP_TX_FLAGS:
			injected = 1;
			failed = le_to_host16((*(uint16_t *) iter.this_arg)) &
					IEEE80211_RADIOTAP_F_TX_FAIL;
			break;
		case IEEE80211_RADIOTAP_DATA_RETRIES:
			break;
		case IEEE80211_RADIOTAP_CHANNEL:
			break;
		case IEEE80211_RADIOTAP_RATE:
			datarate = *iter.this_arg * 5;
			break;
		case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
			ssi_signal = *iter.this_arg;
			break;
		}
	}
	if (rxflags && injected) {
		BLTS_TRACE("Monitor: RX injected frame\n");
		return 0;
	}

	update_tx_seq(data, buf + iter.max_length);

	if (injected) {
		BLTS_TRACE("Monitor: TX injected frame\n");
		*frame = NULL;
		*frame_len = 0;
		return 0;
	}

	*frame_len = len - iter.max_length;
	*frame = buf + iter.max_length;

	return 0;
}


/*
 * Send raw frame using radiotap on the monitor interface. Note that
 * you need to include all ethernet headers for data sent with this
 * function.
 */
/* Adapted from hostapd driver_nl80211.c */
int monitor_send_raw_frame(wlan_core_data *drv, void *data, size_t len, int encrypt)
{
	__u8 rtap_hdr[] = {
		0x00, 0x00, /* radiotap version */
		0x0e, 0x00, /* radiotap length */
		0x02, 0xc0, 0x00, 0x00, /* bmap: flags, tx and rx flags */
		IEEE80211_RADIOTAP_F_FRAG, /* F_FRAG (fragment if required) */
		0x00,       /* padding */
		0x00, 0x00, /* RX and TX flags to indicate that */
		0x00, 0x00, /* this is the injected frame directly */
	};
	struct iovec iov[2] = {
		{
			.iov_base = &rtap_hdr,
			.iov_len = sizeof(rtap_hdr),
		},
		{
			.iov_base = (void *) data,
			.iov_len = len,
		}
	};
	struct msghdr msg = {
		.msg_name = NULL,
		.msg_namelen = 0,
		.msg_iov = iov,
		.msg_iovlen = 2,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};

	if (encrypt)
		rtap_hdr[8] |= IEEE80211_RADIOTAP_F_WEP;

	return sendmsg(drv->monitor_sock, &msg, 0);
}


/*
 * Send layer 2 packet on connected wlanX ifce. Note that this already
 * gets all ethernet headers needed.
 */
int monitor_send_l2_packet(wlan_core_data *data, u16 proto, void *buf, size_t len)
{
	int sock, ret = 0;
	struct sockaddr_ll ll;

	if(data->cmd->ifindex <= 0) {
		BLTS_ERROR("Error - WLAN interface not available\n");
		return -ENOENT;
	}

	memset(&ll, 0, sizeof ll);
	ll.sll_family = PF_PACKET;
	ll.sll_protocol = htons(proto);
	ll.sll_ifindex = data->cmd->ifindex;
	ll.sll_hatype = ARPHRD_IEEE80211;
	ll.sll_pkttype = PACKET_HOST;
	ll.sll_halen = ETH_ALEN;
	memcpy(&ll.sll_addr, data->their_addr, ETH_ALEN);

	sock = socket(AF_PACKET, SOCK_DGRAM, htons(proto));
	if(sock < 0) {
		int err = errno;
		BLTS_ERROR("Error - Failed opening socket - %s (%d)\n",
			strerror(err), err);
		return -err;
	}

	ret = sendto(sock, buf, len, MSG_DONTROUTE, (struct sockaddr *) &ll, sizeof(ll));
	close(sock);
	return ret;
}


