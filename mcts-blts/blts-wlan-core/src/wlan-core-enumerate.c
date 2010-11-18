/*  wlan-core-enumerate.c -- WLAN core enum commands

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
 * Dump range functions adapted from Wireless Tools iwlist implementation;
 * see iwlist.c
 * License header for iwlist code:
 *
 *    Wireless Tools
 *
 *          Jean II - HPLB '99 - HPL 99->07
 *
 * This tool can access various piece of information on the card
 * not part of iwconfig...
 * You need to link this code against "iwlist.c" and "-lm".
 *
 * This file is released under the GPL license.
 *     Copyright (c) 1997-2007 Jean Tourrilhes <jt@hpl.hp.com>
 */

/* 'iw' is an userspace tool to use nl80211.
   dump_phy_handler(), dump_iface_handler() and dump_features_handler()
   functions contain adapted code from iw tool
   License header for iw tool:

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

#define _GNU_SOURCE
#include <stdio.h>
#include <blts_log.h>
#include <blts_timing.h>

#include "wlan-core-enumerate.h"
#include "wlan-core-utils.h"
#include "wlan-core-ioctl.h"

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <linux/if.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>

#define IW_ARRAY_LEN(x) (sizeof(x)/sizeof((x)[0]))
#define IW_AUTH_CAPA_NUM  IW_ARRAY_LEN(iw_auth_capa_name)

typedef struct iwmask_name
{
  unsigned int    mask;
  const char *    name;
} iwmask_name;

static const struct iwmask_name iw_auth_capa_name[] = {
	{ IW_ENC_CAPA_WPA,          "WPA" },
	{ IW_ENC_CAPA_WPA2,         "WPA2" },
	{ IW_ENC_CAPA_CIPHER_TKIP,  "CIPHER-TKIP" },
	{ IW_ENC_CAPA_CIPHER_CCMP,  "CIPHER-CCMP" },
};

static const char * event_capa_req[] = {
	[SIOCSIWNWID - SIOCIWFIRST] = "Set NWID (kernel generated)",
	[SIOCSIWFREQ - SIOCIWFIRST] = "Set Frequency/Channel (kernel generated)",
	[SIOCGIWFREQ - SIOCIWFIRST] = "New Frequency/Channel",
	[SIOCSIWMODE - SIOCIWFIRST] = "Set Mode (kernel generated)",
	[SIOCGIWTHRSPY - SIOCIWFIRST] = "Spy threshold crossed",
	[SIOCGIWAP - SIOCIWFIRST] = "New Access Point/Cell address - roaming",
	[SIOCGIWSCAN - SIOCIWFIRST] = "Scan request completed",
	[SIOCSIWESSID - SIOCIWFIRST] = "Set ESSID (kernel generated)",
	[SIOCGIWESSID - SIOCIWFIRST] = "New ESSID",
	[SIOCGIWRATE - SIOCIWFIRST] = "New bit-rate",
	[SIOCSIWENCODE - SIOCIWFIRST] = "Set Encoding (kernel generated)",
	[SIOCGIWPOWER - SIOCIWFIRST] = NULL,
};

static const char * event_capa_evt[] = {
	[IWEVTXDROP - IWEVFIRST] = "Tx packet dropped - retry exceeded",
	[IWEVCUSTOM - IWEVFIRST] = "Custom driver event",
	[IWEVREGISTERED - IWEVFIRST] = "Registered node",
	[IWEVEXPIRED - IWEVFIRST] = "Expired node",
};

////////////////////////////////////////////////////////////////////////////////////

static const char *ifmodes[NL80211_IFTYPE_MAX + 1] = {
	"unspecified",
	"IBSS",
	"managed",
	"AP",
	"AP/VLAN",
	"WDS",
	"monitor",
	"mesh point"
};
static const char *commands[NL80211_CMD_MAX + 1] = {
	[NL80211_CMD_GET_WIPHY] = "get_wiphy",
	[NL80211_CMD_SET_WIPHY] = "set_wiphy",
	[NL80211_CMD_NEW_WIPHY] = "new_wiphy",
	[NL80211_CMD_DEL_WIPHY] = "del_wiphy",
	[NL80211_CMD_GET_INTERFACE] = "get_interface",
	[NL80211_CMD_SET_INTERFACE] = "set_interface",
	[NL80211_CMD_NEW_INTERFACE] = "new_interface",
	[NL80211_CMD_DEL_INTERFACE] = "del_interface",
	[NL80211_CMD_GET_KEY] = "get_key",
	[NL80211_CMD_SET_KEY] = "set_key",
	[NL80211_CMD_NEW_KEY] = "new_key",
	[NL80211_CMD_DEL_KEY] = "del_key",
	[NL80211_CMD_GET_BEACON] = "get_beacon",
	[NL80211_CMD_SET_BEACON] = "set_beacon",
	[NL80211_CMD_NEW_BEACON] = "new_beacon",
	[NL80211_CMD_DEL_BEACON] = "del_beacon",
	[NL80211_CMD_GET_STATION] = "get_station",
	[NL80211_CMD_SET_STATION] = "set_station",
	[NL80211_CMD_NEW_STATION] = "new_station",
	[NL80211_CMD_DEL_STATION] = "del_station",
	[NL80211_CMD_GET_MPATH] = "get_mpath",
	[NL80211_CMD_SET_MPATH] = "set_mpath",
	[NL80211_CMD_NEW_MPATH] = "new_mpath",
	[NL80211_CMD_DEL_MPATH] = "del_mpath",
	[NL80211_CMD_SET_BSS] = "set_bss",
	[NL80211_CMD_SET_REG] = "set_reg",
	[NL80211_CMD_REQ_SET_REG] = "reg_set_reg",
	[NL80211_CMD_GET_MESH_PARAMS] = "get_mesh_params",
	[NL80211_CMD_SET_MESH_PARAMS] = "set_mesh_params",
	[NL80211_CMD_SET_MGMT_EXTRA_IE] = "set_mgmt_extra_ie",
	[NL80211_CMD_GET_REG] = "get_reg",
	[NL80211_CMD_GET_SCAN] = "get_scan",
	[NL80211_CMD_TRIGGER_SCAN] = "trigger_scan",
	[NL80211_CMD_NEW_SCAN_RESULTS] = "new_scan_results",
	[NL80211_CMD_SCAN_ABORTED] = "scan_aborted",
	[NL80211_CMD_REG_CHANGE] = "reg_change",
	[NL80211_CMD_AUTHENTICATE] = "authenticate",
	[NL80211_CMD_ASSOCIATE] = "associate",
	[NL80211_CMD_DEAUTHENTICATE] = "deauthenticate",
	[NL80211_CMD_DISASSOCIATE] = "disassociate",
	[NL80211_CMD_MICHAEL_MIC_FAILURE] = "michael_mic_failure",
	[NL80211_CMD_REG_BEACON_HINT] = "reg_beacon_hint",
	[NL80211_CMD_JOIN_IBSS] = "join_ibss",
	[NL80211_CMD_LEAVE_IBSS] = "leave_ibss",
	[NL80211_CMD_TESTMODE] = "testmode",
	[NL80211_CMD_CONNECT] = "connect",
	[NL80211_CMD_ROAM] = "roam",
	[NL80211_CMD_DISCONNECT] = "disconnect",
	[NL80211_CMD_SET_WIPHY_NETNS] = "set_wiphy_netns",
	[NL80211_CMD_GET_SURVEY] = "get_survey",
	[NL80211_CMD_SET_PMKSA] = "set_pmksa",
	[NL80211_CMD_DEL_PMKSA] = "del_pmksa",
	[NL80211_CMD_FLUSH_PMKSA] = "flush_pmksa",
	[NL80211_CMD_REMAIN_ON_CHANNEL] = "remain_on_channel",
	[NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL] = "cancel_remain_on_channel",
	[NL80211_CMD_SET_TX_BITRATE_MASK] = "set_tx_bitrate_mask",
	[NL80211_CMD_REGISTER_ACTION] = "register_action",
	[NL80211_CMD_ACTION] = "action",
	[NL80211_CMD_SET_CHANNEL] = "set_channel",
};


static unsigned int dumped_wiphy;

/* Adapted from iwtool info.c */
int dump_phy_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];

	static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
		[NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_RADAR] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
	};

	struct nlattr *tb_rate[NL80211_BITRATE_ATTR_MAX + 1];
	static struct nla_policy rate_policy[NL80211_BITRATE_ATTR_MAX + 1] = {
		[NL80211_BITRATE_ATTR_RATE] = { .type = NLA_U32 },
		[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE] = { .type = NLA_FLAG },
	};

	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	struct nlattr *nl_rate;
	int rem_band, rem_freq, rem_rate;
	int bandidx = 1;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb_msg[NL80211_ATTR_WIPHY_BANDS])
		return NL_SKIP;

	if (tb_msg[NL80211_ATTR_WIPHY_NAME])
		BLTS_DEBUG("Wiphy %s\n", nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]));

	nla_for_each_nested(nl_band, tb_msg[NL80211_ATTR_WIPHY_BANDS], rem_band)
	{
		BLTS_DEBUG("\tBand %d:\n", bandidx);
		bandidx++;

		nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
			  nla_len(nl_band), NULL);

		BLTS_DEBUG("\t\tFrequencies:\n");
		nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq)
		{
			uint32_t freq;
			nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq),
				  nla_len(nl_freq), freq_policy);
			if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
				continue;
			freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);
			BLTS_DEBUG("\t\t\t* %d MHz [%d]", freq, ieee80211_frequency_to_channel(freq));

			if (tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] &&
			    !tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
			{
				BLTS_DEBUG(" (%.1f dBm)", 0.01 * nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]));
			}

			if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
			{
				BLTS_DEBUG(" - disabled");
				goto next;
			}
			if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN])
				BLTS_DEBUG(" - passive scanning");
			if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IBSS])
				BLTS_DEBUG(" - no IBSS");
			if (tb_freq[NL80211_FREQUENCY_ATTR_RADAR])
				BLTS_DEBUG(" - radar detection");
 next:
			 BLTS_DEBUG("\n");
		}

		BLTS_DEBUG("\t\tBitrates (non-HT):\n");
		nla_for_each_nested(nl_rate, tb_band[NL80211_BAND_ATTR_RATES], rem_rate)
		{
			nla_parse(tb_rate, NL80211_BITRATE_ATTR_MAX, nla_data(nl_rate),
				  nla_len(nl_rate), rate_policy);
			if (!tb_rate[NL80211_BITRATE_ATTR_RATE])
				continue;

			BLTS_DEBUG("\t\t\t* %2.1f Mbps", 0.1 * nla_get_u32(tb_rate[NL80211_BITRATE_ATTR_RATE]));
			if (tb_rate[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE])
				BLTS_DEBUG(" - short preamble supported");
			BLTS_DEBUG("\n");
		}
	}

	return NL_SKIP;
}

/* Adapted from iwtool interface.c */
int dump_iface_handler(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];

	unsigned int *wiphy = arg;
	const char *indent = "";

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (wiphy && tb_msg[NL80211_ATTR_WIPHY])
	{
		unsigned int thiswiphy = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
		indent = "\t";

		if (*wiphy != thiswiphy)
			BLTS_DEBUG("Phy #%d\n", thiswiphy);

		*wiphy = thiswiphy;
	}

	if (tb_msg[NL80211_ATTR_IFNAME])
		BLTS_DEBUG("%sInterface %s\n", indent, nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	if (tb_msg[NL80211_ATTR_IFINDEX])
		BLTS_DEBUG("%s\tifindex %d\n", indent, nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	if (tb_msg[NL80211_ATTR_IFTYPE])
	{
		int index = nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE]);

		if (index <= NL80211_IFTYPE_MAX)
		{
			BLTS_DEBUG("%s\ttype %s\n", indent, (ifmodes[index]) );
		}
		else
			BLTS_DEBUG("%s\ttype Unknown mode - %d\n", indent, index);
	}
	return NL_SKIP;
}

/* Adapted from iwtool info.c */
static int dump_features_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	struct nlattr *nl_mode;
	struct nlattr *nl_cmd;
	int rem_mode, rem_cmd;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb_msg[NL80211_ATTR_WIPHY_BANDS])
		return NL_SKIP;

	if (tb_msg[NL80211_ATTR_WIPHY_NAME])
		BLTS_DEBUG("Wiphy %s\n", nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]));

	if (tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS])
		BLTS_DEBUG("\tmax # scan SSIDs: %d\n",
		       nla_get_u8(tb_msg[NL80211_ATTR_MAX_NUM_SCAN_SSIDS]));

	if (tb_msg[NL80211_ATTR_WIPHY_FRAG_THRESHOLD])
	{
		unsigned int frag;

		frag = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FRAG_THRESHOLD]);
		if (frag != (unsigned int)-1)
			BLTS_DEBUG("\tFragmentation threshold: %d\n", frag);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_RTS_THRESHOLD])
	{
		unsigned int rts;

		rts = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_RTS_THRESHOLD]);
		if (rts != (unsigned int)-1)
			BLTS_DEBUG("\tRTS threshold: %d\n", rts);
	}

	if (!tb_msg[NL80211_ATTR_SUPPORTED_IFTYPES])
		goto commands;

	BLTS_DEBUG("\tSupported interface modes:\n");
	nla_for_each_nested(nl_mode, tb_msg[NL80211_ATTR_SUPPORTED_IFTYPES], rem_mode)
	{
		enum nl80211_iftype index = nl_mode->nla_type;

		if (index <= NL80211_IFTYPE_MAX)
		{
			BLTS_DEBUG("\t\t * %s\n", ifmodes[index]);
		}
		else
		{
			BLTS_DEBUG("Unknown mode (%d)", index);
		}
	}

commands:
	if (!tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS])
		return NL_SKIP;

	BLTS_DEBUG("\tSupported commands:\n");
	nla_for_each_nested(nl_cmd, tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS], rem_cmd)
	{
		enum nl80211_commands index = nla_get_u32(nl_cmd);
		if (index <= NL80211_CMD_MAX)
		{
			BLTS_DEBUG("\t\t * %s\n",(commands[index]));
		}
		else
		{
			BLTS_DEBUG("Unknown command (%d)", index);
		}
	}

	return NL_SKIP;
}

static int dump_wiphys(wlan_core_data* data)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();

	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
			NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);

	if (send_and_recv_msgs(data, msg, dump_phy_handler, NULL) == 0)
		return 0;

    return -1;
}

static int dump_ifaces(wlan_core_data* data)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();

	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
			NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);

	dumped_wiphy = -1;

	if (send_and_recv_msgs(data, msg, dump_iface_handler, &dumped_wiphy) == 0)
		return 0;

    return -1;
}

static int dump_features(wlan_core_data* data)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();

	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
			NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(data->cmd->ifname));

	if (send_and_recv_msgs(data, msg, dump_features_handler, NULL) == 0)
		return 0;

nla_put_failure:

    return -1;
}

static int test_expected_phy_values(wlan_command_data *cmd)
{
	char* saveptr = NULL;
	char* p = NULL;
	char* expected_values = NULL;

	expected_values = cmd->expected_phys;

	if(!expected_values)
	{
		BLTS_ERROR("expected phy values is NULL!\n");
		return -1;
	}

	if(!strlen(expected_values))
	{
		BLTS_DEBUG("No expected phy values given - skip check!\n");
		return 0;
	}

	p = strtok_r(expected_values, " ", &saveptr);

	while (p != NULL)
	{
		if(strlen(p))
		{
			BLTS_DEBUG("Checking phy: %s\n", p);
			if(phy_lookup(p) < 0)
			{
				BLTS_ERROR("%s not found!\n", p);
				return -1;
			}
			BLTS_DEBUG("OK\n", p);
		}
		p = strtok_r(NULL, " ", &saveptr);
	}

	return 0;
}

/* Adapted from Wireless Tools iwlist.c */
static int print_freq_info(wlan_core_data *data, struct iw_range range)
{
	int index;

	if (!data)
	{
		BLTS_ERROR("wlan_core_data pointer = NULL\n");
		return -1;
	}

	if (!range.num_frequency && !range.num_channels)
	{
		BLTS_ERROR("\tUnknown channel/frequency information.\n");
		return -1;
	}

	BLTS_DEBUG("\tChannels and available frequencies :\n");
	if (range.num_frequency > 0)
	{
		BLTS_DEBUG("\t%d channels in total - available frequencies :\n",
				range.num_channels);

		for (index = 0; index < range.num_frequency; index++)
		{

			int f;
			struct iw_freq freq = (range.freq[index]);

			if (freq.e == 0)
			{
				if (freq.m < 0)
				{
					BLTS_ERROR("cannot get valid channel/frequency data\n");
					return -1;
				}

				f = ieee80211_channel_to_frequency(freq.m);
				BLTS_DEBUG("\t\t* %d MHz [%d]\n", f, ieee80211_frequency_to_channel(f));
			}
			else
			{
				int i, div = 1000000;
				for (i = 0; i < freq.e; i++)
					div /= 10;
				if (div <= 0)
				{
					BLTS_ERROR("cannot get valid channel/frequency data\n");
					return -1;
				}
				f = freq.m / div;
				BLTS_DEBUG("\t\t* %d MHz [%d]\n", f, ieee80211_frequency_to_channel(f));
			}
		}
	}
	else
	{
		BLTS_DEBUG("\t%d channels\n", range.num_channels);
	}

	return 0;
}

/* Adapted from Wireless Tools iwlist.c */
static int print_auth_info(wlan_core_data *data, struct iw_range range)
{
	int index;
	unsigned int mask;

	if (!data)
	{
		BLTS_ERROR("wlan_core_data pointer = NULL\n");
		return -1;
	}

	if (!range.enc_capa)
	{
		BLTS_ERROR("\tUnknown authentication information.\n");
		return -1;
	}

	/* Display advanced encryption capabilities */
	BLTS_DEBUG("\tAuthentication capabilities :\n");
	mask = range.enc_capa;

	for (index = 0; index < IW_AUTH_CAPA_NUM; index++)
	{
		if (mask & iw_auth_capa_name[index].mask)
		{
			BLTS_DEBUG("\t\t* %s\n", iw_auth_capa_name[index].name);
			mask &= ~iw_auth_capa_name[index].mask;
		}
	}

	if (mask != 0)
		BLTS_DEBUG("Unknown capabilities detected\n");

	BLTS_DEBUG("\n");

	return 0;
}

/* Adapted from Wireless Tools iwlist.c */
static int print_event_capa_info(wlan_core_data *data, struct iw_range range)
{
	int cmd;

	if (!data)
	{
		BLTS_ERROR("wlan_core_data pointer = NULL\n");
		return -1;
	}

	BLTS_DEBUG("\tWireless Events supported :\n");

	for (cmd = SIOCIWFIRST; cmd <= SIOCGIWPOWER; cmd++)
	{
		int idx = IW_EVENT_CAPA_INDEX(cmd);
		int mask = IW_EVENT_CAPA_MASK(cmd);
		if (range.event_capa[idx] & mask)
			BLTS_DEBUG("\t\t* 0x%04X : %s\n", cmd, event_capa_req[cmd
					- SIOCIWFIRST]);
	}
	for (cmd = IWEVFIRST; cmd <= IWEVEXPIRED; cmd++)
	{
		int idx = IW_EVENT_CAPA_INDEX(cmd);
		int mask = IW_EVENT_CAPA_MASK(cmd);
		if (range.event_capa[idx] & mask)
			BLTS_DEBUG("\t\t* 0x%04X : %s\n", cmd, event_capa_evt[cmd - IWEVFIRST]);
	}
	BLTS_DEBUG("\n");

	return 0;
}

static int dump_range(wlan_core_data* data)
{
	struct iw_range *range;
	struct iwreq iwr;
	int minlen;
	size_t buflen;

	buflen = (sizeof(struct iw_range) * 2);
	range = calloc(1, buflen);

	if (range == NULL)
		return -1;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, data->cmd->ifname, IFNAMSIZ);
	iwr.ifr_name[IFNAMSIZ-1] = 0;

	iwr.u.data.pointer = (caddr_t) range;
	iwr.u.data.length = buflen;

	minlen = ((char *) &range->enc_capa) - (char *) range + sizeof(range->enc_capa);

	if(IOCTL_CALL(data, SIOCGIWRANGE, &iwr))

	{
		BLTS_ERROR("ioctl call [SIOCGIWRANGE] failed!\n");
		goto error;
	}

	if (iwr.u.data.length >= minlen && range->we_version_compiled >= 18)
	{
			BLTS_DEBUG("\tioctl call [SIOCGIWRANGE]: WE(compiled)=%d "
				   "WE(source)=%d enc_capa=0x%x\n\n",
				   range->we_version_compiled,
				   range->we_version_source,
				   range->enc_capa);
	}
	else
	{
		BLTS_DEBUG("ioctl call [SIOCGIWRANGE]: too old (short) data - cannot get valid data\n");
		goto error;
	}

    if (print_freq_info(data, *range) ||
    	print_auth_info(data, *range) ||
    	print_event_capa_info(data, *range) )
    	goto error;

	free(range);
	return 0;

error:
	free(range);
	return -1;
}

// --------------Test cases ------------

/**
 * Enumerate through hardware
 */

int enum_hardware(wlan_core_data* data)
{
	int check;
	int err;

	BLTS_DEBUG("\n-- Lookup expected phys: --\n");
	check = test_expected_phy_values(data->cmd);
	if(check)
	{
		BLTS_ERROR("\nExpected phy values check failed: %d\n", check);
		/*NOTE:  test is already failed, but do enumerate whiphys and ifases anyway */
	}

	BLTS_DEBUG("\n-- Enumerated wiphys: --\n");
	err = dump_wiphys(data);
	if (err)
	{
		BLTS_ERROR("\nERROR dump wiphys: %d\n", err);
		return err;
	}

	BLTS_DEBUG("\n-- Enumerated ifaces: --\n");
	err = dump_ifaces(data);
	if (err)
	{
		BLTS_ERROR("\nERROR dump ifaces: %d\n", err);
		return err;
	}

	return check;
}

/**
 * Enumerate through supported features
 */

int enum_features(wlan_core_data* data)
{
	int err;

	BLTS_DEBUG("\n-- Enumerated features: --\n");
	err = dump_features(data);
	if(err)
	{
		BLTS_ERROR("\nERROR dump features: %d\n", err);
		return err;
	}

	BLTS_DEBUG("\n-- Enumerated range of parameters: --\n");
	err = dump_range(data);
	if(err)
	{
		BLTS_ERROR("\nERROR dump range: %d\n", err);
		return err;
	}

	return 0;
}
