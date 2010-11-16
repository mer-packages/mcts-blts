/*  wlan-core-channel.c -- WLAN core channel functions

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

/* 'iw' is an userspace tool to use nl80211.
   Channel list functions contain adapted code from iw tool
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

#include "wlan-core-channel.h"
#include "wlan-core-eloop.h"
#include "wlan-core-utils.h"
#include "wlan-core-scan.h"
#include "wlan-core-debug.h"
#include "wlan-core-dhcp.h"

#include <linux/if.h>
#include <sys/ioctl.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

static int nl80211_channel_list_handler(struct nl_msg *msg, void *arg)
{
	uint32_t freq;
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = (struct genlmsghdr *) nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *nl_band, *nl_freq, *nl_cmd;
	int rem_band, rem_freq, rem_cmd, num_freq = 0;

	struct nl80211_channel *channel_data = (struct nl80211_channel *) arg;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb_msg[NL80211_ATTR_WIPHY_BANDS])
	{
		return NL_SKIP;
	}

	if (tb_msg[NL80211_ATTR_WIPHY_NAME])
	{
		if (strcmp(nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]),
				channel_data->phyname) != 0)
		{
			return NL_SKIP;
		}
	}

	for (nl_band = (struct nlattr *) nla_data(tb_msg[NL80211_ATTR_WIPHY_BANDS]), rem_band
			= nla_len(tb_msg[NL80211_ATTR_WIPHY_BANDS]); nla_ok(nl_band,
			rem_band); nl_band = (struct nlattr *) nla_next(nl_band, &rem_band))
	{

		nla_parse(tb_band, NL80211_BAND_ATTR_MAX,
			(struct nlattr *) nla_data(nl_band), nla_len(nl_band), NULL);

		for (nl_freq = (struct nlattr *) nla_data(
				tb_band[NL80211_BAND_ATTR_FREQS]), rem_freq = nla_len(
				tb_band[NL80211_BAND_ATTR_FREQS]); nla_ok(nl_freq, rem_freq); nl_freq
				= (struct nlattr *) nla_next(nl_freq, &rem_freq))
		{

			nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX,
					(struct nlattr *) nla_data(nl_freq), nla_len(nl_freq), NULL);

			if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
				continue;

			if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;

			num_freq++;
		}
	}
	channel_data->nfreqs = num_freq;
	channel_data->channel_list = malloc(sizeof(int) * num_freq);
	channel_data->set_ch_support = 0;
	num_freq = 0;

	for (nl_band = (struct nlattr *) nla_data(tb_msg[NL80211_ATTR_WIPHY_BANDS]), rem_band
			= nla_len(tb_msg[NL80211_ATTR_WIPHY_BANDS]); nla_ok(nl_band,
			rem_band); nl_band = (struct nlattr *) nla_next(nl_band, &rem_band))
	{
		nla_parse(tb_band, NL80211_BAND_ATTR_MAX, (struct nlattr *) nla_data(
				nl_band), nla_len(nl_band), NULL);

		for (nl_freq = (struct nlattr *) nla_data(
				tb_band[NL80211_BAND_ATTR_FREQS]), rem_freq = nla_len(
				tb_band[NL80211_BAND_ATTR_FREQS]); nla_ok(nl_freq, rem_freq); nl_freq
				= (struct nlattr *) nla_next(nl_freq, &rem_freq))
		{
			nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX,
					(struct nlattr *) nla_data(nl_freq), nla_len(nl_freq), NULL);

			if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
				continue;

			if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;

			freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);

			channel_data->channel_list[num_freq++] =
			ieee80211_frequency_to_channel(freq);
		}
	}

	if (!tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS])
		return NL_SKIP;

	nla_for_each_nested(nl_cmd, tb_msg[NL80211_ATTR_SUPPORTED_COMMANDS], rem_cmd)
	{
		enum nl80211_commands cmd = nla_get_u32(nl_cmd);
		if (cmd <= NL80211_CMD_MAX && cmd == NL80211_CMD_SET_CHANNEL)
			channel_data->set_ch_support = 1;
	}

	return NL_SKIP;
}

/*
 * get channel list and channel count using nl80211 command NL80211_CMD_GET_WIPHY,
 * parameter set_ch_support value indicates is new command NL80211_CMD_SET_CHANNEL supported by device
 * NOTE: user must free allocated channel list
 * example usage:
 * 		// wlan_core_data* data; //filled and defined somewhere else
 *		int i, num, sup;
 *		int *list = NULL;
 *		nl80211_get_channel_list(data, &num, &list, &sup);
 *		for(i=0; i < num; i++)  //prints all channels
 *			printf("%d\n", list[i]);
 *		nl80211_set_channel(data, i, NL80211_CHAN_NO_HT, sup); //sets last channel
 */
int nl80211_get_channel_list(wlan_core_data* data, int *ret_num_channels, int **ret_channel_list, int* set_ch_support)
{
	int ret;
	struct nl_msg *msg;
	nl80211_channel_t channel_data;

	if(!data || !data->cmd->ifname)
		return -1;

	channel_data.phyname = find_parent_phy_device(data->cmd->ifname);
	if (strlen(channel_data.phyname) == 0)
	{
		BLTS_ERROR("Could not find a parent phy device for '%s'\n", data->cmd->ifname);
		return -1;
	}

	msg = nlmsg_alloc();

	if(!msg)
	{
		BLTS_ERROR("Failed to allocate message\n");
		return -1;
	}
	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211),
			0, NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);

	ret = send_and_recv_msgs(data, msg, nl80211_channel_list_handler, &channel_data);
	msg = NULL;

	if(ret)
	{
		BLTS_ERROR("Could not get channel list on interface '%s'\n",  data->cmd->ifname);
		(*ret_num_channels) =  0;
		(*ret_channel_list) = NULL;
		goto nla_put_failure;
	}

	ret = 0;
	BLTS_DEBUG("Get channel list ok:%d\n", channel_data.nfreqs);

	(*set_ch_support) = channel_data.set_ch_support;
	(*ret_num_channels) = channel_data.nfreqs;
	(*ret_channel_list) = (int*) malloc(sizeof(int) * channel_data.nfreqs);
	memcpy(*ret_channel_list, channel_data.channel_list, sizeof(int) * channel_data.nfreqs);

nla_put_failure:

	nlmsg_free(msg);
	free(channel_data.channel_list);
	free(channel_data.phyname);

	return ret;
}

/* set channel using nl80211 command NL80211_CMD_SET_CHANNEL if parameter set_ch_support is not 0
 * or otherwise NL80211_CMD_SET_WIPHY command is used */
int nl80211_set_channel(wlan_core_data* data, int channel, unsigned int chmode, int set_ch_support)
{
	int ret = 0;
	struct nl_msg *msg;

	int channel_mode[] = {
		NL80211_CHAN_NO_HT,
		NL80211_CHAN_HT20,
		NL80211_CHAN_HT40PLUS,
		NL80211_CHAN_HT40MINUS
	};

	if(!data || !data->cmd->ifname)
		return -1;

	if (chmode > 4)
	{
		BLTS_ERROR("Invalid channel mode\n");
		return -1;
	}

	msg = nlmsg_alloc();

	if (!msg)
	{
		BLTS_ERROR("Failed to allocate message\n");
		return -1;
	}

	if(set_ch_support)
	{
		BLTS_DEBUG("using command NL80211_CMD_SET_CHANNEL...\n");
		genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
				NL80211_CMD_SET_CHANNEL, 0);
	}
	else
	{
		BLTS_DEBUG("using command NL80211_CMD_SET_WIPHY...\n");
		genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
				NL80211_CMD_SET_WIPHY, 0);
	}
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, ieee80211_channel_to_frequency(channel));
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, channel_mode[chmode]);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);
	msg = NULL;

	if(ret)
	{
		BLTS_ERROR("Set channel failed: ret=%d (%s)\n", ret, strerror(-ret));
		BLTS_ERROR("Could not set channel [%d] %d MHz on interface '%s'\n", channel,
			ieee80211_channel_to_frequency(channel), data->cmd->ifname);
		goto nla_put_failure;
	}
	ret = 0;
	BLTS_DEBUG("Set channel [%d] %d MHz ok\n", channel,
			ieee80211_channel_to_frequency(channel));

nla_put_failure:

	nlmsg_free(msg);
	return ret;
}
