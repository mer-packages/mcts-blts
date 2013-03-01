/*  wlan-core-enumerate.c -- WLAN core scan functions

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
 * Scan functions adapted from hostapd implementation;
 * see driver_nl80211.c scan_helpers.c
 * License headers for hostapd code:
 *
 *
 * WPA Supplicant - driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2003-2008, Jouni Malinen <j@w1.fi>
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
   Link functions: link_bss_handler() and nl80211_scan_for_link_data()
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

#include "wlan-core-scan.h"
#include "wlan-core-eloop.h"
#include "wlan-core-utils.h"
#include "wlan-core-debug.h"

#include <net/if.h>
#include <sys/ioctl.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

struct scan_results* nl80211_get_scan_results(wlan_core_data *data);
int nl80211_scan(wlan_core_data* data, const u8 *ssid, size_t ssid_len);

static int no_seq_check(struct nl_msg *msg, void *arg)
{
        return NL_OK;
}

/**
 * Adapted from hostapd scan_helpers.c
 * Returns an Information Element from scan_res struct by given ID
 * e.g. WLAN_EID_SSID (0), or NULL if not found
 * */
const u8 * scan_get_ie(struct scan_res *res, u8 ie)
{
	const u8 *end, *pos;

	pos = (const u8 *) (res + 1);
	end = pos + res->ie_len;

	while (pos + 1 < end)
	{
		if (pos + 2 + pos[1] > end)
			break;
		if (pos[0] == ie)
			return pos;
		pos += 2 + pos[1];
	}

	return NULL;
}

/**
 * Converts a numeric ssid to a printable text string
 **/
const char* ssid_to_txt(const u8 *ssid, size_t ssid_len)
{
	static char ssid_txt[33];
	char *pos;

	if (ssid_len > 32)
		ssid_len = 32;
	memcpy(ssid_txt, ssid, ssid_len);
	ssid_txt[ssid_len] = '\0';

	for (pos = ssid_txt; *pos != '\0'; pos++)
	{
		if ((u8) *pos < 32 || (u8) *pos >= 127)
			*pos = '_';
	}
	return ssid_txt;
}

/**
 * Scan timeout to report scan completion
 *
 * This function can be used as registered timeout when starting a scan to
 * generate a scan completed event if the driver does not report this.
 */
void nl80211_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
    BLTS_DEBUG("Scan timeout - try to get results\n");
    wlan_core_data* data = eloop_ctx;
	scan_results_free(data->scan_res);
	data->scan_res = nl80211_get_scan_results(data);

	if (data->scan_res == NULL)
		BLTS_ERROR("Failed to get scan results");
}

static int process_scan_event(struct nl_msg *msg, void *arg)
{
	wlan_core_data* data =  arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int i;

	if(data->flags&CLI_FLAG_VERBOSE_LOG)
	{
		print_event(msg, arg);
	}

	switch (gnlh->cmd)
		{
		case NL80211_CMD_TRIGGER_SCAN:
			BLTS_DEBUG(">Scan trigger\n");
			break;
		case NL80211_CMD_NEW_SCAN_RESULTS:
			BLTS_DEBUG(">New scan results available\n");
			eloop_cancel_timeout(nl80211_scan_timeout, data, data->cmd);
			break;
		case NL80211_CMD_SCAN_ABORTED:
			BLTS_DEBUG(">Scan aborted\n");
			eloop_cancel_timeout(nl80211_scan_timeout, data, data->cmd);
			break;
		default:
			BLTS_DEBUG("Ignore unknown event [%d] for this handler\n", gnlh->cmd);
			break;
		}

	for (i = 0; i < data->cmd->n_waited_cmds; i++)
	{
		if (gnlh->cmd == data->cmd->waited_cmds[i])
		{
			eloop_unregister_read_sock(nl_socket_get_fd(data->nl_sock));
			eloop_terminate();

			if(data->scan_res)
			{
				scan_results_free(data->scan_res);
				data->scan_res = NULL;
			}
			data->scan_res = nl80211_get_scan_results(data);

			if (data->scan_res == NULL)
				BLTS_ERROR("Failed to get scan results\n");
		}
	}

	return NL_SKIP;
}

static void nl80211_scan_oneshot_handler(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct nl_cb *cb;
	wlan_core_data* data = eloop_ctx;

	static const u32 cmds[] =
	{
		NL80211_CMD_NEW_SCAN_RESULTS,
		NL80211_CMD_SCAN_ABORTED,
	};

	data->cmd->waited_cmds = cmds;
	data->cmd->n_waited_cmds = ARRAY_SIZE(cmds);

	cb = nl_cb_clone(data->nl_cb);
	if (!cb)
		return;

	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, process_scan_event, data);
	nl_recvmsgs(data->nl_sock, cb);
	nl_cb_put(cb);
}

int nl80211_scan_oneshot(wlan_core_data* data, const u8 *ssid, size_t ssid_len)
{
	int err;

	eloop_register_read_sock(nl_socket_get_fd(data->nl_sock),
			nl80211_scan_oneshot_handler, data, data->cmd);

	err = nl80211_scan(data, ssid, ssid_len);

	if(!err)
		eloop_run();
	else
		return err;

	return 0;
}

/**
 * Adapted from hostapd driver_nl80211.c
 * Requests an active scan for a specific SSID if ssid && ssid_len is given
 * or an active scan for wildcard SSID if ssid is NULL.
 **/
int nl80211_scan(wlan_core_data* data, const u8 *ssid, size_t ssid_len)
{
    int ret = 0;
	int timeout;
	struct nl_msg *msg, *ssids;

	msg = nlmsg_alloc();
	ssids = nlmsg_alloc();
	if (!msg || !ssids)
	{
		nlmsg_free(msg);
		nlmsg_free(ssids);
		return -1;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
			NL80211_CMD_TRIGGER_SCAN, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	if (ssid && ssid_len)
	{
		/* Request an active scan for a specific SSID */
		NLA_PUT(ssids, 1, ssid_len, ssid);
	}
	else
	{
		/* Request an active scan for wildcard SSID */
		NLA_PUT(ssids, 1, 0, "");
	}
	nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);

	msg = NULL;
	if (ret)
	{
		BLTS_DEBUG("Scan trigger failed: ret=%d (%s)\n", ret, strerror(-ret));
		goto nla_put_failure;
	}

	/* Not all drivers generate "scan completed" wireless event, so try to
	 * read results after a timeout. */
	timeout = 10;

	if (data->scan_complete_events)
	{
		/*
		 * The driver seems to deliver SIOCGIWSCAN events to notify
		 * when scan is complete, so use longer timeout to avoid race
		 * conditions with scanning and following association request.
		 */
		timeout = 30;
	}

	BLTS_DEBUG("Scan requested (ret=%d) - scan timeout %d seconds\n", ret, timeout);
	eloop_cancel_timeout(nl80211_scan_timeout, data, data->cmd);
	eloop_register_timeout(timeout, 0, nl80211_scan_timeout, data, data->cmd);

nla_put_failure:
    nlmsg_free(ssids);
	nlmsg_free(msg);
	return ret;
}


int nl80211_check_bss_connection_status(wlan_core_data* data, struct scan_results *res)
{
	int connected = 0;
	size_t i;

	for (i = 0; i < res->num; i++)
	{
		struct scan_res *r = res->res[i];

		if (r->flags & SCAN_AUTHENTICATED)
		{
			BLTS_DEBUG("Scan results indicates BSS status with " MACSTR
					" as authenticated", MAC2STR(r->bssid));
			connected = 1;
		}

		if (r->flags & SCAN_ASSOCIATED)
		{
			BLTS_DEBUG("Scan results indicate BSS status with " MACSTR
					" as associated", MAC2STR(r->bssid));
			connected = 1;
		}

		if (r->flags & SCAN_JOINED)
		{
			BLTS_DEBUG("Scan results indicate iBSS status with " MACSTR
					" as joined", MAC2STR(r->bssid));
			connected = 1;
		}
	}
	return connected;
}

/* Adapted from hostapd driver_nl80211.c */
static int bss_info_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *bss[NL80211_BSS_MAX + 1];
    static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] =
    {
    		[NL80211_BSS_BSSID] = { .type = NLA_UNSPEC },
    		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
    		[NL80211_BSS_TSF] = { .type = NLA_U64 },
    		[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
    		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
    		[NL80211_BSS_INFORMATION_ELEMENTS] = { .type = NLA_UNSPEC },
    		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
    		[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
	};
	struct scan_results *res = arg;
	struct scan_res **tmp;
	struct scan_res *r;
	const u8 *ie;
	size_t ie_len;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(
			gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS])
	{
		BLTS_ERROR("bss info missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy))
	{
		BLTS_ERROR("failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
	{
		ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
		ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
	}
	else
	{
		ie = NULL;
		ie_len = 0;
	}

	r = calloc(1, sizeof(*r) + ie_len);

	if (r == NULL)
		return NL_SKIP;
	if (bss[NL80211_BSS_BSSID])
	{
		char mac_addr[20], dev[20];
		memcpy(r->bssid, nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);
		mac_addr_n2a(mac_addr, r->bssid);
		if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
		BLTS_DEBUG("BSS %s (on %s)\n", mac_addr, dev);
	}
	if (bss[NL80211_BSS_FREQUENCY])
	{
		r->freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
		BLTS_DEBUG("\tfreq: %d\n", r->freq);
	}
	if (bss[NL80211_BSS_BEACON_INTERVAL])
	{
		r->beacon_int = nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]);
		BLTS_DEBUG("\tbeacon interval: %d\n", r->beacon_int);
	}
	if (bss[NL80211_BSS_CAPABILITY])
	{
		r->caps = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
		BLTS_DEBUG("\tcapability:");
		if (r->caps & WLAN_CAPABILITY_ESS)
			BLTS_DEBUG(" ESS");
		if (r->caps & WLAN_CAPABILITY_IBSS)
			BLTS_DEBUG(" IBSS");
		if (r->caps & WLAN_CAPABILITY_PRIVACY)
			BLTS_DEBUG(" Privacy");
		if (r->caps & WLAN_CAPABILITY_SHORT_PREAMBLE)
			BLTS_DEBUG(" ShortPreamble");
		if (r->caps & WLAN_CAPABILITY_PBCC)
			BLTS_DEBUG(" PBCC");
		if (r->caps & WLAN_CAPABILITY_CHANNEL_AGILITY)
			BLTS_DEBUG(" ChannelAgility");
		if (r->caps & WLAN_CAPABILITY_SPECTRUM_MGMT)
			BLTS_DEBUG(" SpectrumMgmt");
//		if (r->caps & WLAN_CAPABILITY_QOS)  //TODO header?
//			BLTS_DEBUG(" QoS");
		if (r->caps & WLAN_CAPABILITY_SHORT_SLOT_TIME)
			BLTS_DEBUG(" ShortSlotTime");
//		if (r->caps & WLAN_CAPABILITY_APSD) //TODO header?
//			BLTS_DEBUG(" APSD");
		if (r->caps & WLAN_CAPABILITY_DSSS_OFDM)
			BLTS_DEBUG(" DSSS-OFDM");
		BLTS_DEBUG(" (0x%.4x)\n", r->caps);
	}
	if (bss[NL80211_BSS_SIGNAL_UNSPEC])
	{
		r->qual = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);
		BLTS_DEBUG("\tsignal: %d/100\n", r->qual);
	}
	if (bss[NL80211_BSS_SIGNAL_MBM])
	{
		r->level = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
		BLTS_DEBUG("\tsignal: %d.%.2d dBm\n", r->level / 100, r->level % 100);
	}
	if (bss[NL80211_BSS_TSF])
	{
		r->tsf = nla_get_u64(bss[NL80211_BSS_TSF]);
		BLTS_DEBUG("\tTSF: %llu usec (%llud, %.2lld:%.2llu:%.2llu)\n", r->tsf, r->tsf
				/ 1000 / 1000 / 60 / 60 / 24, (r->tsf / 1000 / 1000 / 60 / 60)
				% 24, (r->tsf / 1000 / 1000 / 60) % 60, (r->tsf / 1000 / 1000)
				% 60);
	}
	r->ie_len = ie_len;
	if (ie)
		memcpy(r + 1, ie, ie_len);

	if (bss[NL80211_BSS_STATUS])
	switch (nla_get_u32(bss[NL80211_BSS_STATUS]))
	{
		case NL80211_BSS_STATUS_ASSOCIATED:
			BLTS_TRACE("\t-associated-\n");
			r->flags |= SCAN_ASSOCIATED;
			break;
		case NL80211_BSS_STATUS_AUTHENTICATED:
			BLTS_TRACE("\t-authenticated-\n");
			r->flags |= SCAN_AUTHENTICATED;
			break;
		case NL80211_BSS_STATUS_IBSS_JOINED:
			BLTS_TRACE("\t-joined-\n");
			r->flags |= SCAN_JOINED;
			break;
		default:
			BLTS_TRACE("\n-unknown status-\n");
	}

	tmp = realloc(res->res, (res->num + 1) * sizeof(struct scan_res *));
	if (tmp == NULL)
	{
		free(r);
		return NL_SKIP;
	}
	tmp[res->num++] = r;
	res->res = tmp;

	return NL_SKIP;
}

/**
* Adapted from hostapd driver_nl80211.c
* Fetch the latest scan results or NULL on failure
**/
struct scan_results* nl80211_get_scan_results(wlan_core_data *data)
{
	struct nl_msg *msg;
	struct scan_results *res;
	int ret;

	res = calloc(1, sizeof(*res));

	if (res == NULL)
		return NULL;
	msg = nlmsg_alloc();
	if (!msg)
		goto nla_put_failure;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, NLM_F_DUMP,
			NL80211_CMD_GET_SCAN, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	ret = send_and_recv_msgs(data, msg, bss_info_handler, res);
	msg = NULL;
	if (ret == 0)
	{
		BLTS_DEBUG("Received scan results (%lu BSSes)\n", (unsigned long) res->num);
		return res;
	}

	BLTS_ERROR("Scan result fetch failed: ret=%d (%s)\n", ret, strerror(-ret));

nla_put_failure: nlmsg_free(msg);
	scan_results_free(res);

	return NULL;
}

/**
 * Frees allocated scan results
 **/
void scan_results_free(struct scan_results *res)
{
	size_t i;

	if (res == NULL)
		return;

	for (i = 0; i < res->num; i++)
	{
		BLTS_DEBUG("free scan_result from index [%d]\n", i);
		free(res->res[i]);
	}

	free(res->res);
	free(res);
}

/**
* Search a specific BSS from scan results by a given ssid,
* returns filled scan_res struct or NULL if not found
**/
struct scan_res* get_bss_by_ssid(wlan_core_data *data, u8 *ssid,
		size_t ssid_len)
{
	int i;
	struct scan_res *bss = NULL;
	struct scan_res *found = NULL;
	struct scan_results *results = data->scan_res;

	if (!results)
	{
		BLTS_ERROR("No scan results available!\n");
		return NULL;
	}

	BLTS_DEBUG("Searching bss...\n");
	for (i = 0; i < results->num && !found; i++)
	{
		u8 *ie;
		u8 *id;
		u8 len;

		bss = results->res[i];
		ie = (u8 *) scan_get_ie(bss, WLAN_EID_SSID);

		if (ie == NULL)
		{
			BLTS_DEBUG("No SSID IE included for " MACSTR "\n", MAC2STR(bss->bssid));
			continue;
		}
		if (ie[1] > 32)
		{
			BLTS_DEBUG("Too long SSID IE included for" MACSTR "\n", MAC2STR(bss->bssid));
			continue;
		}

		id = ie ? ie + 2 : (u8 *) "";
		len = ie ? ie[1] : 0;

		BLTS_DEBUG("{%s}\n", ssid_to_txt(id, len));

		if (len != ssid_len || memcmp(id, ssid, len) != 0)
		{
			BLTS_DEBUG("skip - SSID mismatch\n");
			continue;
		}

		found = bss;
		BLTS_DEBUG("BSS found\n");
		break;
	}

	return found;
}

/* Adapted from iw tool link.c */
static int link_bss_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_TSF] = { .type = NLA_U64 },
		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_BSS_BSSID] = { },
		[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = { },
		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
		[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
		[NL80211_BSS_STATUS] = { .type = NLA_U32 },
	};
	wlan_core_data* data =  arg;
	char mac_addr[20], dev[20];

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS]) {
		BLTS_ERROR("bss info missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(bss, NL80211_BSS_MAX,
			     tb[NL80211_ATTR_BSS],
			     bss_policy)) {
		BLTS_ERROR("failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;

	if (!bss[NL80211_BSS_STATUS])
		return NL_SKIP;

	if (nla_get_u32(tb[NL80211_ATTR_IFINDEX]) != data->cmd->ifindex)
	{
			BLTS_DEBUG("Ignored data for foreign interface (ifindex %d)\n", dev);
			return NL_SKIP;
	}
	mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);

	switch (nla_get_u32(bss[NL80211_BSS_STATUS])) {
	case NL80211_BSS_STATUS_IBSS_JOINED:
		BLTS_DEBUG("Joined IBSS %s (on %s)\n", mac_addr, dev);
		break;
	default:
		return NL_SKIP;
	}

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
	{
		u8 *ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
		u8 *id;
		u8 len;

		if (ie == NULL )
			return NL_SKIP;
		if (ie[1] > 32)
			return NL_SKIP;

		id = ie ? ie + 2 : (u8 *) "";
		len = ie ? ie[1] : 0;

		BLTS_DEBUG("\t%s\n", ssid_to_txt(id, len));

		if (bss[NL80211_BSS_FREQUENCY])
			printf("\tfreq: %d\n",
				nla_get_u32(bss[NL80211_BSS_FREQUENCY]));

		BLTS_DEBUG("link_found...\n");
		data->cmd->adhoc_joined = 1;
		memcpy(data->cmd->adhoc_bssid, nla_data(bss[NL80211_BSS_BSSID]), 6);
	}
	return NL_SKIP;
}

/* Adapted from iw tool link.c */
int nl80211_scan_for_link_data(wlan_core_data *data)
{
	struct nl_msg *msg;
	int ret;

	msg = nlmsg_alloc();
	if (!msg)
		goto nla_put_failure;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, NLM_F_DUMP,
			NL80211_CMD_GET_SCAN, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	data->cmd->adhoc_joined = 0;
	ret = send_and_recv_msgs(data, msg, link_bss_handler, data);
	msg = NULL;
	if (ret == 0)
	{
		if(data->cmd->adhoc_joined)
		{
			 BLTS_DEBUG("Connected to " MACSTR "\n", MAC2STR(data->cmd->adhoc_bssid));
			 BLTS_DEBUG("\n");
		}
		return data->cmd->adhoc_joined;
	}

	BLTS_ERROR("Link data fetch failed: ret=%d (%s)\n", ret, strerror(-ret));

nla_put_failure:
	nlmsg_free(msg);

	return -1;
}

// test cases --->

int scan_for_specific_ap(wlan_core_data* data)
{
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	if(!ssid)
		return -1;

	if(nl80211_scan_oneshot(data, ssid, strlen((const char *)ssid)))
	{
		BLTS_ERROR("\nERROR wlan scanning failed!\n");
		return -1;
	}

	struct scan_res* bss = get_bss_by_ssid(data, (u8*)ssid, strlen((const char *)ssid));

	if (!bss)
	{
		BLTS_ERROR("ERROR cannot find SSID: %s\n", ssid);
		return -1;
	}

	return 0;
}
