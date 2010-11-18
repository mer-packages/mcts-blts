/* wlan-core-debug.h -- WLAN core debug and trace functions

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

/*
 * hexdump functions ( hexdump() and hexdump_ascii() ) adapted from
 * hostapd implementation ( wpa_hexdump() wpa_hexdump_ascii() );
 * see src/utils/wpa_debug.c
 * License headers for hostapd code:
 *
 * wpa_supplicant/hostapd / Debug prints
 * Copyright (c) 2002-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

/* 'iw' is an userspace tool to use nl80211.
   The print_event functions adapted from iw tool (event.c)

   License headers for iw tool:

   Copyright (c) 2007, 2008	Johannes Berg
   Copyright (c) 2007		Andy Lutomirski
   Copyright (c) 2007		Mike Kershaw
   Copyright (c) 2008-2009		Luis R. Rodriguez

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

#include <blts_log.h>

#include "wlan-core-debug.h"
#include "wlan-core-utils.h"

#define PARSE_BEACON_CHAN(_attr, _chan) do { \
	r = parse_beacon_hint_chan(tb[_attr], \
				   &_chan); \
	if (r) \
		return NL_SKIP; \
} while (0)

static const char *status_table[] = {
	[0] = "Successful",
	[1] = "Unspecified failure",
	[10] = "Cannot support all requested capabilities in the capability information field",
	[11] = "Reassociation denied due to inability to confirm that association exists",
	[12] = "Association denied due to reason outside the scope of this standard",
	[13] = "Responding station does not support the specified authentication algorithm",
	[14] = "Received an authentication frame with authentication transaction sequence number out of expected sequence",
	[15] = "Authentication rejected because of challenge failure",
	[16] = "Authentication rejected due to timeout waiting for next frame in sequence",
	[17] = "Association denied because AP is unable to handle additional associated STA",
	[18] = "Association denied due to requesting station not supporting all of the data rates in the BSSBasicRateSet parameter",
	[19] = "Association denied due to requesting station not supporting the short preamble option",
	[20] = "Association denied due to requesting station not supporting the PBCC modulation option",
	[21] = "Association denied due to requesting station not supporting the channel agility option",
	[22] = "Association request rejected because Spectrum Management capability is required",
	[23] = "Association request rejected because the information in the Power Capability element is unacceptable",
	[24] = "Association request rejected because the information in the Supported Channels element is unacceptable",
	[25] = "Association request rejected due to requesting station not supporting the short slot time option",
	[26] = "Association request rejected due to requesting station not supporting the ER-PBCC modulation option",
	[27] = "Association denied due to requesting STA not supporting HT features",
	[28] = "R0KH Unreachable",
	[29] = "Association denied because the requesting STA does not support the PCO transition required by the AP",
	[30] = "Association request rejected temporarily; try again later",
	[31] = "Robust Management frame policy violation",
	[32] = "Unspecified, QoS related failure",
	[33] = "Association denied due to QAP having insufficient bandwidth to handle another QSTA",
	[34] = "Association denied due to poor channel conditions",
	[35] = "Association (with QBSS) denied due to requesting station not supporting the QoS facility",
	[37] = "The request has been declined",
	[38] = "The request has not been successful as one or more parameters have invalid values",
	[39] = "The TS has not been created because the request cannot be honored. However, a suggested Tspec is provided so that the initiating QSTA may attempt to send another TS with the suggested changes to the TSpec",
	[40] = "Invalid Information Element",
	[41] = "Group Cipher is not valid",
	[42] = "Pairwise Cipher is not valid",
	[43] = "AKMP is not valid",
	[44] = "Unsupported RSN IE version",
	[45] = "Invalid RSN IE Capabilities",
	[46] = "Cipher suite is rejected per security policy",
	[47] = "The TS has not been created. However, the HC may be capable of creating a TS, in response to a request, after the time indicated in the TS Delay element",
	[48] = "Direct link is not allowed in the BSS by policy",
	[49] = "Destination STA is not present within this QBSS",
	[50] = "The destination STA is not a QSTA",
	[51] = "Association denied because Listen Interval is too large",
	[52] = "Invalid Fast BSS Transition Action Frame Count",
	[53] = "Invalid PMKID",
	[54] = "Invalid MDIE",
	[55] = "Invalid FTIE",
};

const char *get_status_str(uint16_t status)
{
	if (status < ARRAY_SIZE(status_table) && status_table[status])
		return status_table[status];
	return "<unknown>";
}

static const char *reason_table[] = {
	[1] = "Unspecified",
	[2] = "Previous authentication no longer valid",
	[3] = "Deauthenticated because sending station is leaving (or has left) the IBSS or ESS",
	[4] = "Disassociated due to inactivity",
	[5] = "Disassociated because AP is unable to handle all currently associated STA",
	[6] = "Class 2 frame received from non-authenticated station",
	[7] = "Class 3 frame received from non-authenticated station",
	[8] = "Disassociated because sending station is leaving (or has left) the BSS",
	[9] = "Station requesting (re)association is not authenticated with responding station",
	[10] = "Disassociated because the information in the Power Capability element is unacceptable",
	[11] = "Disassociated because the information in the Supported Channels element is unacceptable",
	[13] = "Invalid information element",
	[14] = "MIC failure",
	[15] = "4-way handshake timeout",
	[16] = "Group key update timeout",
	[17] = "Information element in 4-way handshake different from (Re-)associate request/Probe response/Beacon",
	[18] = "Multicast cipher is not valid",
	[19] = "Unicast cipher is not valid",
	[20] = "AKMP is not valid",
	[21] = "Unsupported RSNE version",
	[22] = "Invalid RSNE capabilities",
	[23] = "IEEE 802.1X authentication failed",
	[24] = "Cipher Suite rejected per security policy",
	[31] = "TS deleted because QoS AP lacks sufficient bandwidth for this QoS STA due to a change in BSS service characteristics or operational mode",
	[32] = "Disassociated for unspecified QoS-related reason",
	[33] = "Disassociated because QAP lacks sufficient bandwidth for this STA",
	[34] = "Disassociated because of excessive frame losses and/or poor channel conditions",
	[35] = "Disassociated because QSTA is transmitting outside the limits of its polled TXOPs",
	[36] = "Requested from peer QSTA as the QSTA is leaving the QBSS (or resetting)",
	[37] = "Requested from peer QSTA as it does not want to use Traffic Stream",
	[38] = "Requested from peer QSTA as the QSTA received frames indicated Traffic Stream for which it has not set up",
	[39] = "Requested from peer QSTA due to time out",
	[40] = "Requested from peer QSTA as the QSTA is leaving the QBSS (or resetting)",
	[41] = "Requested from peer QSTA as it does not want to receive frames directly from the QSTA",
	[42] = "Requested from peer QSTA as the QSTA received DLP frames for which it has not set up",
	[43] = "Requested from peer QSTA as it does not want to use Block Ack",
	[44] = "Requested from peer QSTA as the QSTA received frames indicated Block Acknowledgement policy for which it has not set up",
	[45] = "Peer QSTA does not support the requested cipher suite",
};

const char *get_reason_str(uint16_t reason)
{
	if (reason < ARRAY_SIZE(reason_table) && reason_table[reason])
		return reason_table[reason];
	return "<unknown>";
}

char *reg_initiator_to_string(u8 initiator)
{
	switch (initiator) {
	case NL80211_REGDOM_SET_BY_CORE:
		return "the wireless core upon initialization";
	case NL80211_REGDOM_SET_BY_USER:
		return "a user";
	case NL80211_REGDOM_SET_BY_DRIVER:
		return "a driver";
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		return "a country IE";
	default:
		return "BUG";
	}
}

struct ieee80211_beacon_channel {
	u16 center_freq;
	bool passive_scan;
	bool no_ibss;
};

static int parse_beacon_hint_chan(struct nlattr *tb,
		struct ieee80211_beacon_channel *chan)
{
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	static struct nla_policy beacon_freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] =
	{
		[NL80211_FREQUENCY_ATTR_FREQ] =
		{	.type = NLA_U32},
		[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] =
		{	.type = NLA_FLAG},
		[NL80211_FREQUENCY_ATTR_NO_IBSS] =
		{	.type = NLA_FLAG},
	};

	if (nla_parse_nested(tb_freq, NL80211_FREQUENCY_ATTR_MAX, tb,
			beacon_freq_policy))
		return -EINVAL;

	chan->center_freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);

	if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN])
		chan->passive_scan = true;
	if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IBSS])
		chan->no_ibss = true;

	return 0;
}

static void print_frame(struct nlattr *attr)
{
	uint8_t *frame;
	size_t len;
	int i;
	char macbuf[6* 3 ];
	uint16_t tmp;

	if (!attr)
	BLTS_DEBUG(" [no frame]");

	frame = nla_data(attr);
	len = nla_len(attr);

	if (len < 26)
	{
		BLTS_DEBUG(" [invalid frame: ");
		goto print_frame;
	}

	mac_addr_n2a(macbuf, frame + 10);
	BLTS_DEBUG(" %s -> ", macbuf);
	mac_addr_n2a(macbuf, frame + 4);
	BLTS_DEBUG("%s", macbuf);

	switch (frame[0] & 0xfc)
	{
		case 0x10: /* assoc resp */
		case 0x30: /* reassoc resp */
		/* status */
		tmp = (frame[27] << 8) + frame[26];
		BLTS_DEBUG(" status: %d: %s", tmp, get_status_str(tmp));
		break;
		case 0x00: /* assoc req */
		case 0x20: /* reassoc req */
		break;
		case 0xb0: /* auth */
		/* status */
		tmp = (frame[29] << 8) + frame[28];
		BLTS_DEBUG(" status: %d: %s", tmp, get_status_str(tmp));
		break;
		break;
		case 0xa0: /* disassoc */
		case 0xc0: /* deauth */
		/* reason */
		tmp = (frame[25] << 8) + frame[24];
		BLTS_DEBUG(" reason %d: %s", tmp, get_reason_str(tmp));
		break;
	}

	BLTS_DEBUG(" [frame:");

print_frame:
	for (i = 0; i < len; i++)
	BLTS_DEBUG(" %.02x", frame[i]);
	BLTS_DEBUG("]");
}

void print_ssid_escaped(const uint8_t len, const uint8_t *data)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if (isprint(data[i]))
		{
			BLTS_DEBUG("%c", data[i]);
		}
		else
		{
			BLTS_DEBUG("\\x%.2x", data[i]);
		}
	}
}
/* Adapted from iw event.c */
int print_event(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1], *nst;

	char ifname[100];
	char macbuf[6* 3 ];
	u8 reg_type;
	struct ieee80211_beacon_channel chan_before_beacon, chan_after_beacon;
	u32 wiphy_idx = 0;
	int r;
	int rem_nst;
	u16 status;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (tb[NL80211_ATTR_IFINDEX] && tb[NL80211_ATTR_WIPHY])
	{
		if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), ifname);
		BLTS_DEBUG("%s (phy #%d): ", ifname, nla_get_u32(tb[NL80211_ATTR_WIPHY]));
	}
	else if (tb[NL80211_ATTR_IFINDEX])
	{
		if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), ifname);
		BLTS_DEBUG("%s: ", ifname);
	}
	else if (tb[NL80211_ATTR_WIPHY])
	{
		BLTS_DEBUG("phy #%d: ", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
	}

	switch (gnlh->cmd)
	{
		case NL80211_CMD_NEW_WIPHY:
		BLTS_DEBUG("renamed to %s\n", nla_get_string(tb[NL80211_ATTR_WIPHY_NAME]));
		break;
	case NL80211_CMD_TRIGGER_SCAN:
		BLTS_DEBUG("scan started\n");
		break;
	case NL80211_CMD_NEW_SCAN_RESULTS:
		BLTS_DEBUG("scan finished:");
	case NL80211_CMD_SCAN_ABORTED:
		if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED)
			BLTS_DEBUG("scan aborted:");
		if (tb[NL80211_ATTR_SCAN_FREQUENCIES]) {
			nla_for_each_nested(nst, tb[NL80211_ATTR_SCAN_FREQUENCIES], rem_nst)
				BLTS_DEBUG(" %d", nla_get_u32(nst));
			BLTS_DEBUG(",");
		}
		if (tb[NL80211_ATTR_SCAN_SSIDS]) {
			nla_for_each_nested(nst, tb[NL80211_ATTR_SCAN_SSIDS], rem_nst) {
				BLTS_DEBUG(" \"");
				print_ssid_escaped(nla_len(nst), nla_data(nst));
				BLTS_DEBUG("\"");
			}
		}
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_REG_CHANGE:
		BLTS_DEBUG("regulatory domain change: ");

		reg_type = nla_get_u8(tb[NL80211_ATTR_REG_TYPE]);

		switch (reg_type) {
		case NL80211_REGDOM_TYPE_COUNTRY:
			BLTS_DEBUG("set to %s by %s request",
			       nla_get_string(tb[NL80211_ATTR_REG_ALPHA2]),
			       reg_initiator_to_string(nla_get_u8(tb[NL80211_ATTR_REG_INITIATOR])));
			if (tb[NL80211_ATTR_WIPHY])
				BLTS_DEBUG(" on phy%d", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
			break;
		case NL80211_REGDOM_TYPE_WORLD:
			BLTS_DEBUG("set to world roaming by %s request",
			       reg_initiator_to_string(nla_get_u8(tb[NL80211_ATTR_REG_INITIATOR])));
			break;
		case NL80211_REGDOM_TYPE_CUSTOM_WORLD:
			BLTS_DEBUG("custom world roaming rules in place on phy%d by %s request",
			       nla_get_u32(tb[NL80211_ATTR_WIPHY]),
			       reg_initiator_to_string(nla_get_u32(tb[NL80211_ATTR_REG_INITIATOR])));
			break;
		case NL80211_REGDOM_TYPE_INTERSECTION:
			BLTS_DEBUG("intersection used due to a request made by %s",
			       reg_initiator_to_string(nla_get_u32(tb[NL80211_ATTR_REG_INITIATOR])));
			if (tb[NL80211_ATTR_WIPHY])
				BLTS_DEBUG(" on phy%d", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
			break;
		default:
			BLTS_DEBUG("unknown source (upgrade this utility)");
			break;
		}

		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_REG_BEACON_HINT:

		wiphy_idx = nla_get_u32(tb[NL80211_ATTR_WIPHY]);

		memset(&chan_before_beacon, 0, sizeof(chan_before_beacon));
		memset(&chan_after_beacon, 0, sizeof(chan_after_beacon));

		PARSE_BEACON_CHAN(NL80211_ATTR_FREQ_BEFORE, chan_before_beacon);
		PARSE_BEACON_CHAN(NL80211_ATTR_FREQ_AFTER, chan_after_beacon);

		if (chan_before_beacon.center_freq != chan_after_beacon.center_freq)
			break;

		/* A beacon hint is sent _only_ if something _did_ change */
		BLTS_DEBUG("beacon hint:\n");

		BLTS_DEBUG("phy%d %d MHz [%d]:\n",
		       wiphy_idx,
		       chan_before_beacon.center_freq,
		       ieee80211_frequency_to_channel(chan_before_beacon.center_freq));

		if (chan_before_beacon.passive_scan && !chan_after_beacon.passive_scan)
			BLTS_DEBUG("\to active scanning enabled\n");
		if (chan_before_beacon.no_ibss && !chan_after_beacon.no_ibss)
			BLTS_DEBUG("\to beaconing enabled\n");

		break;
	case NL80211_CMD_NEW_STATION:
		mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
		BLTS_DEBUG("new station %s\n", macbuf);
		break;
	case NL80211_CMD_JOIN_IBSS:
		mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
		BLTS_DEBUG("IBSS %s joined\n", macbuf);
		break;
	case NL80211_CMD_AUTHENTICATE:
		BLTS_DEBUG("auth");
		if (tb[NL80211_ATTR_FRAME])
			print_frame(tb[NL80211_ATTR_FRAME]);
		else if (tb[NL80211_ATTR_TIMED_OUT])
		{
			BLTS_DEBUG(": timed out");
		}
		else
		{
			BLTS_DEBUG(": unknown event");
		}
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_ASSOCIATE:
		BLTS_DEBUG("assoc");
		if (tb[NL80211_ATTR_FRAME])
		{
			print_frame(tb[NL80211_ATTR_FRAME]);
		}
		else if (tb[NL80211_ATTR_TIMED_OUT])
		{
			BLTS_DEBUG(": timed out");
		}
		else
		{
			BLTS_DEBUG(": unknown event");
		}
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_DEAUTHENTICATE:
		BLTS_DEBUG("deauth");
		print_frame(tb[NL80211_ATTR_FRAME]);
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_DISASSOCIATE:
		BLTS_DEBUG("disassoc");
		print_frame(tb[NL80211_ATTR_FRAME]);
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_CONNECT:
		status = 0;
		if (!tb[NL80211_ATTR_STATUS_CODE])
		{
			BLTS_DEBUG("unknown connect status");
		}
		else if (nla_get_u16(tb[NL80211_ATTR_STATUS_CODE]) == 0)
		{
			BLTS_DEBUG("connected");
		}
		else
		{
			status = nla_get_u16(tb[NL80211_ATTR_STATUS_CODE]);
			BLTS_DEBUG("failed to connect");
		}
		if (tb[NL80211_ATTR_MAC]) {
			mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
			BLTS_DEBUG(" to %s", macbuf);
		}
		if (status)
			BLTS_DEBUG(", status: %d: %s", status, get_status_str(status));
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_ROAM:
		BLTS_DEBUG("roamed");
		if (tb[NL80211_ATTR_MAC]) {
			mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
			BLTS_DEBUG(" to %s", macbuf);
		}
		BLTS_DEBUG("\n");
		break;
	case NL80211_CMD_DISCONNECT:
		BLTS_DEBUG("disconnected");
		if (tb[NL80211_ATTR_DISCONNECTED_BY_AP])
		{
			BLTS_DEBUG(" (by AP)");
		}
		else
		{
			BLTS_DEBUG(" (local request)");
		}
		if (tb[NL80211_ATTR_REASON_CODE])
		{
			BLTS_DEBUG(" reason: %d: %s", nla_get_u16(tb[NL80211_ATTR_REASON_CODE]),
				get_reason_str(nla_get_u16(tb[NL80211_ATTR_REASON_CODE])));
		}
		BLTS_DEBUG("\n");
		break;
	default:
		BLTS_DEBUG("unknown event %d\n", gnlh->cmd);
		break;
	}

	return NL_SKIP;
}

/* Adapted from hostapd wpa_debug.c */
void hexdump(const char *title, const u8 *buf,	size_t len)
{
	size_t i;
	char *dump = NULL;
	static char hex[] = "0123456789abcdef";

	if(buf == NULL) {
		BLTS_DEBUG("%s - hexdump(len=%lu): [NULL]\n", title, (unsigned long) len);
		return;
	}

	/* Collect the dump on one line since we're may be competing other
	   threads for logging */

	dump = malloc(len * 3 + 1); /* " xx" * len */
	if(!dump)
		return;
	for (i = 0; i < len; i++) {
		dump[3 * i] = ' ';
		dump[3 * i + 1] = hex[(buf[i] & 0xf0) >> 4];
		dump[3 * i + 2] = hex[buf[i] & 0x0f];
	}
	dump[len * 3] = '\0';

	BLTS_DEBUG("%s - hexdump(len=%lu): %s\n", title, (unsigned long) len, dump);
	free(dump);
}

/* Adapted from hostapd wpa_debug.c */
void hexdump_ascii(const char *title, const u8 *buf, size_t len)
{
	size_t i, llen;
	const u8 *pos = buf;
	const size_t line_len = 16;

	if (buf == NULL)
	{
		BLTS_DEBUG("%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		return;
	}
	BLTS_DEBUG("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len)
	{
		llen = len > line_len ? line_len : len;
		BLTS_DEBUG("    ");
		for (i = 0; i < llen; i++)
			BLTS_DEBUG(" %02x", pos[i]);
		for (i = llen; i < line_len; i++)
			BLTS_DEBUG("   ");
		BLTS_DEBUG("   ");
		for (i = 0; i < llen; i++)
		{
			if (isprint(pos[i]))
			{
				BLTS_DEBUG("%c", pos[i]);
			}
			else
			{
				BLTS_DEBUG("_");
			}
		}
		for (i = llen; i < line_len; i++)
			BLTS_DEBUG(" ");
		BLTS_DEBUG("\n");
		pos += llen;
		len -= llen;
	}
}


static const char *nl80211_cmd_strings[] = {
	"NL80211_CMD_UNSPEC",
	"NL80211_CMD_GET_WIPHY",
	"NL80211_CMD_SET_WIPHY",
	"NL80211_CMD_NEW_WIPHY",
	"NL80211_CMD_DEL_WIPHY",
	"NL80211_CMD_GET_INTERFACE",
	"NL80211_CMD_SET_INTERFACE",
	"NL80211_CMD_NEW_INTERFACE",
	"NL80211_CMD_DEL_INTERFACE",
	"NL80211_CMD_GET_KEY",
	"NL80211_CMD_SET_KEY",
	"NL80211_CMD_NEW_KEY",
	"NL80211_CMD_DEL_KEY",
	"NL80211_CMD_GET_BEACON",
	"NL80211_CMD_SET_BEACON",
	"NL80211_CMD_NEW_BEACON",
	"NL80211_CMD_DEL_BEACON",
	"NL80211_CMD_GET_STATION",
	"NL80211_CMD_SET_STATION",
	"NL80211_CMD_NEW_STATION",
	"NL80211_CMD_DEL_STATION",
	"NL80211_CMD_GET_MPATH",
	"NL80211_CMD_SET_MPATH",
	"NL80211_CMD_NEW_MPATH",
	"NL80211_CMD_DEL_MPATH",
	"NL80211_CMD_SET_BSS",
	"NL80211_CMD_SET_REG",
	"NL80211_CMD_REQ_SET_REG",
	"NL80211_CMD_GET_MESH_PARAMS",
	"NL80211_CMD_SET_MESH_PARAMS",
	"NL80211_CMD_SET_MGMT_EXTRA_IE",
	"NL80211_CMD_GET_REG",
	"NL80211_CMD_GET_SCAN",
	"NL80211_CMD_TRIGGER_SCAN",
	"NL80211_CMD_NEW_SCAN_RESULTS",
	"NL80211_CMD_SCAN_ABORTED",
	"NL80211_CMD_REG_CHANGE",
	"NL80211_CMD_AUTHENTICATE",
	"NL80211_CMD_ASSOCIATE",
	"NL80211_CMD_DEAUTHENTICATE",
	"NL80211_CMD_DISASSOCIATE",
	"NL80211_CMD_MICHAEL_MIC_FAILURE",
	"NL80211_CMD_REG_BEACON_HINT",
	"NL80211_CMD_JOIN_IBSS",
	"NL80211_CMD_LEAVE_IBSS",
	"NL80211_CMD_TESTMODE",
	"NL80211_CMD_CONNECT",
	"NL80211_CMD_ROAM",
	"NL80211_CMD_DISCONNECT",
	"NL80211_CMD_SET_WIPHY_NETNS",
	"NL80211_CMD_GET_SURVEY",
	"NL80211_CMD_NEW_SURVEY_RESULTS",
	"NL80211_CMD_SET_PMKSA",
	"NL80211_CMD_DEL_PMKSA",
	"NL80211_CMD_FLUSH_PMKSA",
	"NL80211_CMD_REMAIN_ON_CHANNEL",
	"NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL",
	"NL80211_CMD_SET_TX_BITRATE_MASK",
	"NL80211_CMD_REGISTER_ACTION",
	"NL80211_CMD_ACTION",
	"NL80211_CMD_ACTION_TX_STATUS",
	"NL80211_CMD_SET_POWER_SAVE",
	"NL80211_CMD_GET_POWER_SAVE",
	"NL80211_CMD_SET_CQM",
	"NL80211_CMD_NOTIFY_CQM",
	"NL80211_CMD_SET_CHANNEL"
};
static const char nl80211_cmd_string_unknown[] = "!! Unknown NL80211_CMD !!";

const char *nl80211_cmd_as_string(unsigned cmd)
{
	if (cmd > NL80211_CMD_SET_CHANNEL)
		return nl80211_cmd_string_unknown;
	return nl80211_cmd_strings[cmd];
}
