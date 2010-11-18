/*  wlan-core-connect.c -- WLAN core connection functions

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
 * Functions adapted from wpa_supplicant implementation;
 * see driver_nl80211.c
 * License headers for wpa_supplicant code:
 *
 *
 * Driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2002-2010, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
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

#define _GNU_SOURCE
#include <stdio.h>
#include <blts_log.h>
#include <blts_timing.h>

#include "wlan-core-connect.h"
#include "wlan-core-eloop.h"
#include "wlan-core-utils.h"
#include "wlan-core-scan.h"
#include "wlan-core-debug.h"
#include "wlan-core-dhcp.h"
#include "wlan-core-cmdqueue.h"
#include "wlan-core-adhoc.h"

#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>

#include <sys/ioctl.h>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <pthread.h>

static pthread_t event_reader_thread_id = 0;
static volatile int reader_thread_running;

static int no_seq_check(struct nl_msg *msg, void *arg);
static int process_connect_event(struct nl_msg *msg, void *arg);

static void *event_reader_thread_function(void *ptr)
{
	wlan_core_data* data = (wlan_core_data *)ptr;
	fd_set rfds;
	int res, sock;
	struct timeval tv;
	struct nl_handle *nl_handle;
	struct nl_cache *nl_cache;
	struct nl_cb *nl_cb;
	struct genl_family *nl80211;

	data->cmd->n_waited_cmds = 0;
	reader_thread_running = 1;

	nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!nl_cb) {
		BLTS_ERROR("Failed to allocate cb\n");
		return NULL;
	}

	nl_handle = nl_handle_alloc_cb(nl_cb);
	if (!nl_handle) {
		nl_cb_put(nl_cb);
		nl_cb = NULL;
		return NULL;
	}

	if (genl_connect(nl_handle)) {
		BLTS_ERROR("Failed to connect to generic netlink\n");
		goto out_handle_destroy;
	}

	nl_cache = genl_ctrl_alloc_cache(nl_handle);
	if (!nl_cache) {
		BLTS_ERROR("Failed to allocate generic netlink cache\n");
		goto out_handle_destroy;
	}

	nl80211 = genl_ctrl_search_by_name(nl_cache, "nl80211");
	if (!nl80211) {
		BLTS_ERROR("nl80211 not available on nl ifce\n");
		goto out_cache_free;
	}

	res = nl_get_multicast_id(data, "nl80211", "scan");
	if (res >= 0)
		res = nl_socket_add_membership(nl_handle, res);

	res = nl_get_multicast_id(data, "nl80211", "mlme");
	if (res >= 0)
		res = nl_socket_add_membership(nl_handle, res);

	nl_cb_set(nl_cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(nl_cb, NL_CB_VALID, NL_CB_CUSTOM, process_connect_event, data);

	sock = nl_socket_get_fd(nl_handle);

	while (reader_thread_running) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		memset(&tv, 0, sizeof(tv));
		tv.tv_sec = 1;

		res = select(sock + 1 , &rfds, NULL, NULL, &tv);
		if (res < 0 && errno != EINTR) {
			BLTS_LOGGED_PERROR("select");
			break;
		} else if (!res)
			continue;

		nl_recvmsgs(nl_handle, nl_cb);
	}

out_cache_free:
	nl_cache_free(nl_cache);
	nl_cache = NULL;
out_handle_destroy:
	nl_handle_destroy(nl_handle);
	nl_cb_put(nl_cb);
	return NULL;
}

static int setup_event_reader_thread(wlan_core_data *data)
{
	int ret;

	clear_cmd_queue();

	if (reader_thread_running)
		return -EFAULT;

	reader_thread_running = 0;
	ret = pthread_create(&event_reader_thread_id, NULL,
		event_reader_thread_function, data);
	if (ret)
		BLTS_ERROR("failed to create event reader thread (%d)!\n", ret);

	sleep(2);
	return ret;
}

static void stop_event_reader_thread()
{
	if (event_reader_thread_id) {
		reader_thread_running = 0;
		pthread_join(event_reader_thread_id, NULL);
	}
	clear_cmd_queue();
}

static int no_seq_check(struct nl_msg *msg, void *arg)
{
        return NL_OK;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
static int nl80211_set_conn_keys(struct associate_params *params, struct nl_msg *msg)
{
	int i, privacy = 0;
	struct nlattr *nl_keys, *nl_key;

	for (i = 0; i < 4; i++)
	{
		if (!params->wep_key[i])
			continue;
		privacy = 1;
		break;
	}
	if (!privacy)
		return 0;

	NLA_PUT_FLAG(msg, NL80211_ATTR_PRIVACY);

	nl_keys = nla_nest_start(msg, NL80211_ATTR_KEYS);
	if (!nl_keys)
		goto nla_put_failure;

	for (i = 0; i < 4; i++)
	{
		if (!params->wep_key[i])
			continue;

		nl_key = nla_nest_start(msg, i);
		if (!nl_key)
			goto nla_put_failure;

		NLA_PUT(msg, NL80211_KEY_DATA, params->wep_key_len[i],
				params->wep_key[i]);
		if (params->wep_key_len[i] == 5)
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER, WLAN_CIPHER_SUITE_WEP40);
		else
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER, WLAN_CIPHER_SUITE_WEP104);

		NLA_PUT_U8(msg, NL80211_KEY_IDX, i);

		if (i == params->wep_tx_keyidx)
			NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT);

		nla_nest_end(msg, nl_key);
	}
	nla_nest_end(msg, nl_keys);

	return 0;

nla_put_failure:
	return -ENOBUFS;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
static int nl_add_key(struct nl_msg *msg, wpa_alg alg,
                      int key_idx, int defkey,
                      const u8 *seq, size_t seq_len,
                      const u8 *key, size_t key_len)
{
        struct nlattr *key_attr = nla_nest_start(msg, NL80211_ATTR_KEY);
        if (!key_attr)
                return -1;

        if (defkey && alg == WPA_ALG_IGTK)
                NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT_MGMT);
        else if (defkey)
                NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT);

        NLA_PUT_U8(msg, NL80211_KEY_IDX, key_idx);

        switch (alg)
        {
        case WPA_ALG_WEP:
                if (key_len == 5)
                        NLA_PUT_U32(msg, NL80211_KEY_CIPHER,
                                    WLAN_CIPHER_SUITE_WEP40);
                else
                        NLA_PUT_U32(msg, NL80211_KEY_CIPHER,
                                    WLAN_CIPHER_SUITE_WEP104);
                break;
        case WPA_ALG_TKIP:
                NLA_PUT_U32(msg, NL80211_KEY_CIPHER, WLAN_CIPHER_SUITE_TKIP);
                break;
        case WPA_ALG_CCMP:
                NLA_PUT_U32(msg, NL80211_KEY_CIPHER, WLAN_CIPHER_SUITE_CCMP);
                break;
        case WPA_ALG_IGTK:
                NLA_PUT_U32(msg, NL80211_KEY_CIPHER,
                            WLAN_CIPHER_SUITE_AES_CMAC);
                break;
        default:
                BLTS_ERROR("%s: Unsupported encryption "
                           "algorithm %d\n", __func__, alg);
                return -1;
        }

        if (seq && seq_len)
                NLA_PUT(msg, NL80211_KEY_SEQ, seq_len, seq);

        NLA_PUT(msg, NL80211_KEY_DATA, key_len, key);

        nla_nest_end(msg, key_attr);

        return 0;
 nla_put_failure:
        return -1;
}

static void mlme_timeout_event(wlan_core_data *data, enum nl80211_commands cmd,
		struct nlattr *addr)
{
	if (nla_len(addr) != ETH_ALEN)
		return;

	BLTS_DEBUG("MLME event (%d) %s\n", cmd, nl80211_cmd_as_string(cmd));
	BLTS_DEBUG("timeout with " MACSTR "\n", MAC2STR((u8 *) nla_data(addr)));

	if (cmd == NL80211_CMD_AUTHENTICATE)
	{
		BLTS_DEBUG("NL80211_CMD_AUTHENTICATE timeout!\n");
		data->cmd->associated = 0;
		memset(data->cmd->auth_bssid, 0, ETH_ALEN);
	}
	else if (cmd == NL80211_CMD_ASSOCIATE)
	{
		BLTS_DEBUG("NL80211_CMD_ASSOCIATE timeout!\n");
	}
	else
	{
		BLTS_DEBUG("Timeout!\n");
		return;
	}
}

static void mlme_event(wlan_core_data *data, enum nl80211_commands cmd,
		struct nlattr *frame, struct nlattr *addr, struct nlattr *timed_out)
{

    u16 status;
	struct ieee80211_mgmt *mgmt;
	char macbuf[20];

	if (timed_out && addr)
	{
		mlme_timeout_event(data, cmd, addr);
		return;
	}

	if (frame == NULL)
	{
		BLTS_DEBUG("MLME event (%d) %s without frame data\n", cmd, nl80211_cmd_as_string(cmd));
		return;
	}

	BLTS_DEBUG("MLME event (%d) %s\n", cmd, nl80211_cmd_as_string(cmd));

	u8 *fra = nla_data(frame);
	size_t len = nla_len(frame);

	hexdump("MLME event frame", fra, len);

	switch (cmd)
	{
	case NL80211_CMD_AUTHENTICATE:
		mgmt = (struct ieee80211_mgmt *) fra;
		if (len < 24 + sizeof(mgmt->u.auth))
		{
			BLTS_ERROR("Too short authentication event frame\n");
			return;
		}
		memcpy(data->cmd->auth_bssid, mgmt->sa, ETH_ALEN);
		mac_addr_n2a(macbuf, data->cmd->auth_bssid);
		BLTS_DEBUG("NL80211_CMD_AUTHENTICATE -> auth_bssid:%s\n", macbuf);
		/*
		 mac_addr_n2a(macbuf, fra + 10);
		 BLTS_DEBUG("NL80211_CMD_AUTHENTICATE -> auth_bssid:%s\n", macbuf);
		 memcpy(data->cmd->auth_bssid, fra + 10, ETH_ALEN);
		 */
		break;
	case NL80211_CMD_ASSOCIATE:
        mgmt = (struct ieee80211_mgmt *) fra;
        if (len < 24 + sizeof(mgmt->u.assoc_resp))
        {
                BLTS_ERROR("Too short association event frame\n");
                return;
        }
        status = le_to_host16(mgmt->u.assoc_resp.status_code);
        if (status != WLAN_STATUS_SUCCESS)
        {
		 BLTS_ERROR("Status failed!\n");
             return;
        }
        memcpy(data->cmd->bssid, mgmt->sa, ETH_ALEN);
        mac_addr_n2a(macbuf, data->cmd->bssid);
        BLTS_DEBUG("NL80211_CMD_ASSOCIATE -> bssid:%s\n", macbuf);
        data->cmd->associated = 1;
		break;
	case NL80211_CMD_DEAUTHENTICATE:
		BLTS_DEBUG("NL80211_CMD_DEAUTHENTICATE\n");
		data->cmd->associated = 0;
		break;
	case NL80211_CMD_DISASSOCIATE:
		BLTS_DEBUG("NL80211_CMD_DEAUTHENTICATE\n");
		data->cmd->associated = 0;
		break;
	default:
		break;
	}
}

static void mlme_event_connect(wlan_core_data *data, enum nl80211_commands cmd,
		struct nlattr *status, struct nlattr *addr, struct nlattr *req_ie,
		struct nlattr *resp_ie)
{
	if (cmd == NL80211_CMD_CONNECT && nla_get_u16(status)
			!= WLAN_STATUS_SUCCESS)
	{
		BLTS_ERROR("Association rejected!\n");
		return;
	}

	data->cmd->associated = 1;
	if (addr)
	{
		memcpy(data->cmd->bssid, nla_data(addr), ETH_ALEN);
	}
}

static void mlme_event_join_ibss(wlan_core_data *data, struct nlattr *tb[])
{
        if (tb[NL80211_ATTR_MAC] == NULL)
        {
                BLTS_ERROR("No address in IBSS joined event\n");
                return;
        }
        memcpy(data->cmd->bssid, nla_data(tb[NL80211_ATTR_MAC]), ETH_ALEN);
        data->cmd->associated = 1;
}

static int process_connect_event(struct nl_msg *msg, void *arg)
{
	int i;
	wlan_core_data* data =  arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *tb[NL80211_ATTR_MAX + 1];

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
                  genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_IFINDEX])
	{
		int ifindex = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
		if (ifindex != data->cmd->ifindex)
		{
			BLTS_DEBUG("Ignored event (cmd=%d (%s)) for foreign interface (ifindex %d)\n",
				gnlh->cmd, nl80211_cmd_as_string(gnlh->cmd), ifindex);
			return NL_SKIP;
		}
	}

	if(data->flags&CLI_FLAG_VERBOSE_LOG)
	{
		print_event(msg, arg);
	}

	switch (gnlh->cmd)
		{
		 case NL80211_CMD_AUTHENTICATE:
	        case NL80211_CMD_ASSOCIATE:
	        case NL80211_CMD_DEAUTHENTICATE:
	        case NL80211_CMD_DISASSOCIATE:
	        	BLTS_TRACE(">mlme_event\n");
	            mlme_event(data, gnlh->cmd, tb[NL80211_ATTR_FRAME],
	                       tb[NL80211_ATTR_MAC], tb[NL80211_ATTR_TIMED_OUT]);
	                break;
	        case NL80211_CMD_CONNECT:
	        case NL80211_CMD_ROAM:
	        	BLTS_TRACE(">mlme_event_connect\n");
	            mlme_event_connect(data, gnlh->cmd,
	                               tb[NL80211_ATTR_STATUS_CODE],
	                               tb[NL80211_ATTR_MAC],
	                               tb[NL80211_ATTR_REQ_IE],
	                               tb[NL80211_ATTR_RESP_IE]);
	                break;
	        case NL80211_CMD_DISCONNECT:
	        	BLTS_TRACE(">NL80211_CMD_DISCONNECT\n");
	            data->cmd->associated = 0;
	            break;
	        case NL80211_CMD_JOIN_IBSS:
	        	BLTS_TRACE(">NL80211_CMD_JOIN_IBSS\n");
	        	mlme_event_join_ibss(data, tb);
	            break;
	        case NL80211_CMD_NEW_STATION:
	        	BLTS_TRACE(">NL80211_CMD_NEW_STATION\n");
	        	break;
		default:
			BLTS_DEBUG("Ignore unknown event [%d (%s)] for this handler\n", gnlh->cmd,
				nl80211_cmd_as_string(gnlh->cmd));
			break;
		}

	if(event_reader_thread_id)
		add_to_cmd_queue(gnlh->cmd);

	for (i = 0; i < data->cmd->n_waited_cmds; i++)
	{
		if (gnlh->cmd == data->cmd->waited_cmds[i])
		{
			BLTS_DEBUG(">>eloop_terminate\n");
			eloop_unregister_read_sock(nl_socket_get_fd(data->nl_handle));
			eloop_terminate();
		}
	}

	return NL_SKIP;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
static int nl80211_set_key(wlan_core_data *data,
                                      wpa_alg alg, const u8 *addr,
                                      int key_idx, int set_tx,
                                      const u8 *seq, size_t seq_len,
                                      const u8 *key, size_t key_len)
{
        struct nl_msg *msg;
        int ret;

        BLTS_DEBUG("%s: ifindex=%d alg=%d addr=%p key_idx=%d "
                   "set_tx=%d seq_len=%lu key_len=%lu\n",
                   __func__, data->cmd->ifindex, alg, addr, key_idx, set_tx,
                   (unsigned long) seq_len, (unsigned long) key_len);

        msg = nlmsg_alloc();
        if (!msg)
                return -ENOMEM;

        if (alg == WPA_ALG_NONE)
        {
                genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
                            0, NL80211_CMD_DEL_KEY, 0);
        }
        else
        {
                genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
                            0, NL80211_CMD_NEW_KEY, 0);
                NLA_PUT(msg, NL80211_ATTR_KEY_DATA, key_len, key);
                switch (alg) {
                case WPA_ALG_WEP:
                        if (key_len == 5)
                                NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
                                            WLAN_CIPHER_SUITE_WEP40);
                        else
                                NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
                                            WLAN_CIPHER_SUITE_WEP104);
                        break;
                case WPA_ALG_TKIP:
                        NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
                                    WLAN_CIPHER_SUITE_TKIP);
                        break;
                case WPA_ALG_CCMP:
                        NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
                                    WLAN_CIPHER_SUITE_CCMP);
                        break;
                case WPA_ALG_IGTK:
                        NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
                                    WLAN_CIPHER_SUITE_AES_CMAC);
                        break;
                default:
                        BLTS_ERROR("%s: Unsupported encryption "
                                   "algorithm %d\n", __func__, alg);
                        nlmsg_free(msg);
                        return -1;
                }
        }

        if (seq && seq_len)
                NLA_PUT(msg, NL80211_ATTR_KEY_SEQ, seq_len, seq);

        if (addr && memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) != 0)
        {
                BLTS_DEBUG("   addr=" MACSTR "\n", MAC2STR(addr));
                NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, addr);
        }
        NLA_PUT_U8(msg, NL80211_ATTR_KEY_IDX, key_idx);
        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

        ret = send_and_recv_msgs(data, msg, NULL, NULL);
        if (ret == -ENOENT && alg == WPA_ALG_NONE)
                ret = 0;
        if (ret)
                BLTS_DEBUG("set_key failed; err=%d %s\n", ret, strerror(-ret));

        /*
         * If we failed or don't need to set the default TX key (below),
         * we're done here.
         */
        if (ret || !set_tx || alg == WPA_ALG_NONE)
                return ret;

        msg = nlmsg_alloc();
        if (!msg)
                return -ENOMEM;

        genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
                    0, NL80211_CMD_SET_KEY, 0);
        NLA_PUT_U8(msg, NL80211_ATTR_KEY_IDX, key_idx);
        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
        if (alg == WPA_ALG_IGTK)
                NLA_PUT_FLAG(msg, NL80211_ATTR_KEY_DEFAULT_MGMT);
        else
                NLA_PUT_FLAG(msg, NL80211_ATTR_KEY_DEFAULT);

        ret = send_and_recv_msgs(data, msg, NULL, NULL);
        if (ret == -ENOENT)
                ret = 0;
        if (ret)
                BLTS_DEBUG("Set_key default failed err=%d %s\n", ret, strerror(-ret));
        return ret;

nla_put_failure:
        return -ENOBUFS;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
int nl80211_connect(wlan_core_data *data, struct associate_params *params)
{
        struct nl_msg *msg;
        enum nl80211_auth_type type;
        int ret = 0;

        msg = nlmsg_alloc();
        if (!msg)
                return -1;

        BLTS_DEBUG("Connect (ifindex=%d)\n", data->cmd->ifindex);
        genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
                    NL80211_CMD_CONNECT, 0);

        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
        if (params->bssid)
        {
                BLTS_DEBUG("  * bssid=" MACSTR "\n", MAC2STR(params->bssid));
                NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, params->bssid);
        }
        if (params->freq)
        {
                BLTS_DEBUG("  * freq=%d\n", params->freq);
                NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, params->freq);
        }
        if (params->ssid)
        {
                hexdump_ascii("  * SSID", params->ssid, params->ssid_len);
                NLA_PUT(msg, NL80211_ATTR_SSID, params->ssid_len, params->ssid);
        }
        hexdump("  * IEs", params->wpa_ie, params->wpa_ie_len);
        if (params->wpa_ie)
                NLA_PUT(msg, NL80211_ATTR_IE, params->wpa_ie_len, params->wpa_ie);

        if (params->auth_alg & WPA_AUTH_ALG_OPEN)
                type = NL80211_AUTHTYPE_OPEN_SYSTEM;
        else if (params->auth_alg & WPA_AUTH_ALG_SHARED)
                type = NL80211_AUTHTYPE_SHARED_KEY;
        else if (params->auth_alg & WPA_AUTH_ALG_LEAP)
                type = NL80211_AUTHTYPE_NETWORK_EAP;
        else if (params->auth_alg & WPA_AUTH_ALG_FT)
                type = NL80211_AUTHTYPE_FT;
        else
                goto nla_put_failure;

        BLTS_DEBUG("  * Auth Type %d\n", type);
        NLA_PUT_U32(msg, NL80211_ATTR_AUTH_TYPE, type);

        if (params->wpa_ie && params->wpa_ie_len)
        {
                enum nl80211_wpa_versions ver;

                if (params->wpa_ie[0] == WLAN_EID_RSN)
                        ver = NL80211_WPA_VERSION_2;
                else
                        ver = NL80211_WPA_VERSION_1;

                BLTS_DEBUG("  * WPA Version %d\n", ver);
                NLA_PUT_U32(msg, NL80211_ATTR_WPA_VERSIONS, ver);
        }

        if (params->pairwise_suite != CIPHER_NONE)
        {
                int cipher;

                switch (params->pairwise_suite)
                {
                case CIPHER_WEP40:
                        cipher = WLAN_CIPHER_SUITE_WEP40;
                        break;
                case CIPHER_WEP104:
                        cipher = WLAN_CIPHER_SUITE_WEP104;
                        break;
                case CIPHER_CCMP:
                        cipher = WLAN_CIPHER_SUITE_CCMP;
                        break;
                case CIPHER_TKIP:
                default:
                        cipher = WLAN_CIPHER_SUITE_TKIP;
                        break;
                }
                NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITES_PAIRWISE, cipher);
        }

        if (params->group_suite != CIPHER_NONE)
        {
                int cipher;

                switch (params->group_suite)
                {
                case CIPHER_WEP40:
                        cipher = WLAN_CIPHER_SUITE_WEP40;
                        break;
                case CIPHER_WEP104:
                        cipher = WLAN_CIPHER_SUITE_WEP104;
                        break;
                case CIPHER_CCMP:
                        cipher = WLAN_CIPHER_SUITE_CCMP;
                        break;
                case CIPHER_TKIP:
                default:
                        cipher = WLAN_CIPHER_SUITE_TKIP;
                        break;
                }
                NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITE_GROUP, cipher);
        }

        if (params->key_mgmt_suite == KEY_MGMT_802_1X ||
            params->key_mgmt_suite == KEY_MGMT_PSK)
        {
                int mgmt = WLAN_AKM_SUITE_PSK;

                switch (params->key_mgmt_suite) {
                case KEY_MGMT_802_1X:
                        mgmt = WLAN_AKM_SUITE_8021X;
                        break;
                case KEY_MGMT_PSK:
                default:
                        mgmt = WLAN_AKM_SUITE_PSK;
                        break;
                }
                NLA_PUT_U32(msg, NL80211_ATTR_AKM_SUITES, mgmt);
        }

        ret = nl80211_set_conn_keys(params, msg);
        if (ret)
                goto nla_put_failure;

        ret = send_and_recv_msgs(data, msg, NULL, NULL);
        msg = NULL;
        if (ret)
        {
                BLTS_DEBUG("MLME connect failed: ret=%d (%s)\n", ret, strerror(-ret));
                goto nla_put_failure;
        }
        ret = 0;
        BLTS_DEBUG("Connect request send successfully\n");

nla_put_failure:

        nlmsg_free(msg);
        return ret;

}

/* Adapted from wpa_supplicant driver_nl80211.c */
static int nl80211_mlme(wlan_core_data *data, const u8 *addr, int cmd, u16 reason_code)
{
        int ret = -1;
        struct nl_msg *msg;

        msg = nlmsg_alloc();
        if (!msg)
                return -1;

        genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0, cmd, 0);

        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
        NLA_PUT_U16(msg, NL80211_ATTR_REASON_CODE, reason_code);
        NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, addr);

        ret = send_and_recv_msgs(data, msg, NULL, NULL);
        msg = NULL;
        if (ret)
        {
                BLTS_DEBUG("MLME command failed: ret=%d (%s)\n", ret, strerror(-ret));
                goto nla_put_failure;
        }
        ret = 0;

nla_put_failure:
        nlmsg_free(msg);
        return ret;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
int nl80211_authenticate(wlan_core_data *data, struct auth_params *params)
{
        int ret = -1;
        int i;
        struct nl_msg *msg;
        enum nl80211_auth_type type;
        int count = 0;

        data->cmd->associated = 0;
        memset(data->cmd->auth_bssid, 0, ETH_ALEN);

retry:
        msg = nlmsg_alloc();
        if (!msg)
                return -1;

        BLTS_DEBUG("Authenticate (ifindex=%d)\n", data->cmd->ifindex);

        genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
                    NL80211_CMD_AUTHENTICATE, 0);

        for (i = 0; i < 4; i++)
        {
                if (!params->wep_key[i])
                        continue;
                nl80211_set_key(data, WPA_ALG_WEP, NULL,
                                           i,
                                           i == params->wep_tx_keyidx, NULL, 0,
                                           params->wep_key[i],
                                           params->wep_key_len[i]);
                if (params->wep_tx_keyidx != i)
                        continue;
                if (nl_add_key(msg, WPA_ALG_WEP, i, 1, NULL, 0,
                               params->wep_key[i], params->wep_key_len[i]))
                {
                        nlmsg_free(msg);
                        return -1;
                }
        }

        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
        if (params->bssid)
        {
                BLTS_DEBUG("  * bssid=" MACSTR "\n", MAC2STR(params->bssid));
                NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, params->bssid);
        }
        if (params->freq)
        {
                BLTS_DEBUG("  * freq=%d\n", params->freq);
                NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, params->freq);
        }
        if (params->ssid)
        {
                hexdump_ascii("  * SSID", params->ssid, params->ssid_len);
                NLA_PUT(msg, NL80211_ATTR_SSID, params->ssid_len, params->ssid);
        }
        hexdump("  * IEs", params->ie, params->ie_len);

        if (params->ie)
                NLA_PUT(msg, NL80211_ATTR_IE, params->ie_len, params->ie);
        if (params->auth_alg & WPA_AUTH_ALG_OPEN)
                type = NL80211_AUTHTYPE_OPEN_SYSTEM;
        else if (params->auth_alg & WPA_AUTH_ALG_SHARED)
                type = NL80211_AUTHTYPE_SHARED_KEY;
        else if (params->auth_alg & WPA_AUTH_ALG_LEAP)
                type = NL80211_AUTHTYPE_NETWORK_EAP;
        else if (params->auth_alg & WPA_AUTH_ALG_FT)
                type = NL80211_AUTHTYPE_FT;
        else
                goto nla_put_failure;
        BLTS_DEBUG("  * Auth Type %d\n", type);
        NLA_PUT_U32(msg, NL80211_ATTR_AUTH_TYPE, type);


       ret = send_and_recv_msgs(data, msg, NULL, NULL);
        msg = NULL;
        if (ret)
        {
                BLTS_DEBUG("MLME command failed: ret=%d (%s)\n", ret, strerror(-ret));
                count++;

                if (ret == -EALREADY && count == 1 && params->bssid)
                {
                        /*
                         * mac80211 does not currently accept new
                         * authentication if we are already authenticated. As a
                         * workaround, force deauthentication and try again.
                         */
                        BLTS_DEBUG("Retry authentication after forced deauthentication\n");
                        nl80211_deauthenticate(
                                data, params->bssid,
                                WLAN_REASON_PREV_AUTH_NOT_VALID);
                        nlmsg_free(msg);
                        goto retry;
                }
                goto nla_put_failure;
        }
        ret = 0;
        BLTS_DEBUG("Authentication request send successfully\n");

nla_put_failure:
        nlmsg_free(msg);
        return ret;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
int nl80211_associate(wlan_core_data *data, struct associate_params *params)
{
        int ret = -1;
        struct nl_msg *msg;

        data->cmd->associated = 0;

        msg = nlmsg_alloc();
        if (!msg)
                return -1;

        BLTS_DEBUG("Associate (ifindex=%d)\n", data->cmd->ifindex);
        genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
                    NL80211_CMD_ASSOCIATE, 0);

        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
        if (params->bssid)
        {
                BLTS_DEBUG("  * bssid=" MACSTR "\n", MAC2STR(params->bssid));
                NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, params->bssid);
        }
        if (params->freq)
        {
                BLTS_DEBUG("  * freq=%d\n", params->freq);
                NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, params->freq);
        }

        if (params->ssid)
        {
                hexdump_ascii("  * SSID", params->ssid, params->ssid_len);
                NLA_PUT(msg, NL80211_ATTR_SSID, params->ssid_len, params->ssid);
        }

        hexdump("  * IEs", params->wpa_ie, params->wpa_ie_len);
        if (params->wpa_ie)
                NLA_PUT(msg, NL80211_ATTR_IE, params->wpa_ie_len,
                        params->wpa_ie);

        NLA_PUT_FLAG(msg, NL80211_ATTR_CONTROL_PORT);

        ret = send_and_recv_msgs(data, msg, NULL, NULL);
        msg = NULL;
        if (ret)
        {
                BLTS_DEBUG("MLME command failed: ret=%d (%s)\n", ret, strerror(-ret));
                goto nla_put_failure;
        }
        ret = 0;
        BLTS_DEBUG("Association request send successfully\n");

nla_put_failure:
        nlmsg_free(msg);
        return ret;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
int nl80211_leave_ibss(wlan_core_data *data)
{
	struct nl_msg *msg;
	int ret = -1;

	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
			NL80211_CMD_LEAVE_IBSS, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);
	msg = NULL;

	if (ret)
	{
		BLTS_ERROR("Leave IBSS failed: ret=%d (%s)\n", ret, strerror(-ret));
		goto nla_put_failure;
	}

	ret = 0;
	BLTS_DEBUG("Leave IBSS request sent successfully\n");

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

/* Adapted from wpa_supplicant driver_nl80211.c */
int nl80211_join_ibss(wlan_core_data *data, struct associate_params *params)
{
    struct nl_msg *msg;
	int ret = -1;
	int count = 0;

	BLTS_DEBUG("Join IBSS (ifindex=%d)\n", data->cmd->ifindex);
retry:
	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
			NL80211_CMD_JOIN_IBSS, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	if (params->ssid == NULL)
		goto nla_put_failure;

	hexdump_ascii("  * SSID\n", params->ssid, params->ssid_len);
	NLA_PUT(msg, NL80211_ATTR_SSID, params->ssid_len, params->ssid);
	memcpy(data->cmd->ssid, params->ssid, params->ssid_len);

	if(params->freq)
	{
		BLTS_DEBUG("  * freq=%d\n", params->freq);
		NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, params->freq);
	}

	ret = nl80211_set_conn_keys(params, msg);
	if (ret)
		goto nla_put_failure;

	if (params->wpa_ie)
	{
		hexdump("  * Extra IEs for Beacon/Probe Response frames\n",
				params->wpa_ie, params->wpa_ie_len);
		NLA_PUT(msg, NL80211_ATTR_IE, params->wpa_ie_len, params->wpa_ie);
	}

	ret = send_and_recv_msgs(data, msg, NULL, NULL);
	msg = NULL;
	if (ret)
	{
		BLTS_DEBUG("Join IBSS failed: ret=%d (%s)\n", ret, strerror(-ret));
		count++;
		if (ret == -EALREADY && count == 1)
		{
			BLTS_DEBUG("Retry IBSS join after forced leave\n");
			nl80211_leave_ibss(data);
			nlmsg_free(msg);
			goto retry;
		}
		ret = -1;
		goto nla_put_failure;
	}
	ret = 0;
	BLTS_DEBUG("Join IBSS request sent successfully\n");

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

int nl80211_disconnect(wlan_core_data *data, const u8 *addr, int reason_code)
{
        data->cmd->associated = 0;
        return nl80211_mlme(data, addr, NL80211_CMD_DISCONNECT, reason_code);
}

int nl80211_deauthenticate(wlan_core_data *data, const u8 *addr, int reason_code)
{
        data->cmd->associated = 0;
        return nl80211_mlme(data, addr, NL80211_CMD_DEAUTHENTICATE, reason_code);
}

int nl80211_disassociate(wlan_core_data *data, const u8 *addr, int reason_code)
{
        data->cmd->associated = 0;
        return nl80211_mlme(data, addr, NL80211_CMD_DISASSOCIATE, reason_code);
}


static void nl80211_wait_cmd_handler(int sock, void *eloop_ctx, void *sock_ctx)
{
	int i;
	struct nl_cb *cb;
	wlan_core_data* data = eloop_ctx;

	data->cmd->waited_cmds = sock_ctx;

	for (i = 0; i < data->cmd->waited_cmds[i]; i++)
		if(data->cmd->waited_cmds[i] <= NL80211_CMD_UNSPEC ||
				data->cmd->waited_cmds[i] >= __NL80211_CMD_AFTER_LAST)
		{
			BLTS_ERROR("unknown command [%d] for wait handler!\n",
					data->cmd->waited_cmds[i]);

			eloop_unregister_read_sock(nl_socket_get_fd(data->nl_handle));
			eloop_terminate();
			return;
		}
	data->cmd->n_waited_cmds = i;

	cb = nl_cb_clone(data->nl_cb);
	if (!cb)
		return;

	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, process_connect_event, data);
	nl_recvmsgs(data->nl_handle, cb);
	nl_cb_put(cb);
}

int nl80211_associate_oneshot(wlan_core_data *data, struct associate_params* params)
{
	int err;

	static const u32 cmds[] =
	{
		NL80211_CMD_CONNECT,
		0
	};

	eloop_register_read_sock(nl_socket_get_fd(data->nl_handle),
			nl80211_wait_cmd_handler, data, &cmds);

	err = nl80211_associate(data, params);

	if(!err)
		eloop_run();
	else
		return err;

	return 0;
}

int nl80211_authenticate_oneshot(wlan_core_data *data, struct auth_params* params)
{
	int err;

	static const u32 cmds[] =
	{
		NL80211_CMD_AUTHENTICATE,
		0
	};

	eloop_register_read_sock(nl_socket_get_fd(data->nl_handle),
			nl80211_wait_cmd_handler, data, &cmds);

	err = nl80211_authenticate(data, params);

	if(!err)
		eloop_run();
	else
		return err;

	return 0;
}

int nl80211_connect_oneshot(wlan_core_data *data, struct associate_params* params)
{
	int err;

	static const u32 cmds[] =
	{
		NL80211_CMD_CONNECT,
		0
	};

	eloop_register_read_sock(nl_socket_get_fd(data->nl_handle),
			nl80211_wait_cmd_handler, data, &cmds);

	err = nl80211_connect(data, params);

	if(!err)
		eloop_run();
	else
		return err;

	return 0;
}

// test cases --->

int associate_with_open_ap(wlan_core_data* data)
{
	u8 *ie;
	int connected = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct auth_params au_params;
	struct associate_params as_params;

	memset(&au_params, 0, sizeof(au_params));
	memset(&as_params, 0, sizeof(as_params));

	memset(&data->cmd->bssid, 0, sizeof(data->cmd->bssid));
	memset(&data->cmd->auth_bssid, 0, sizeof(data->cmd->auth_bssid));

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

	ie = (u8*)scan_get_ie(bss, WLAN_EID_SSID);

	au_params.bssid = as_params.bssid = bss->bssid;
	au_params.ssid = as_params.ssid = ie ? ie + 2 : (u8 *) "";
	au_params.ssid_len = as_params.ssid_len = ie ? ie[1] : 0;
	au_params.freq = as_params.freq = bss->freq;
	au_params.auth_alg = as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	if(nl80211_authenticate_oneshot(data, &au_params))
	{
		BLTS_ERROR("Authentication failed!\n");
		return -1;
	}

	if(nl80211_associate_oneshot(data, &as_params))
	{
		BLTS_ERROR("Association failed!\n");
		return -1;
	}


	if(nl80211_set_supp_port(data, 1)) {
		BLTS_ERROR("Cannot authorize port - connect failed.\n");
		return -1;
	}
	/* if(nl80211_connect(data, &as_params)) */
	/* { */
	/* 	BLTS_ERROR("Connection failed!\n"); */
	/* 	goto error; */
	/* } */
	if(!data->cmd->associated) {
		BLTS_ERROR("Not associated, annot test.\n");
		goto error;
	}

	connected = 1;

	if(send_dhcp_discover(data))
	{
		BLTS_ERROR("Verification with DHCP failed!\n");
		goto error;
	}

	if (nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING))
	{
		BLTS_ERROR("Failed to disconnect from " MACSTR "\n", MAC2STR(bss->bssid));
		goto error;
	}

	connected = 0;

	/* check that mlme events AUTHENTICATE and ASSOCIATE really filled addresses */
	if (!data->cmd->bssid || !data->cmd->auth_bssid)
		goto error;

	return 0;
error:
	if(connected)
		nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING);
	return -1;
}

int associate_with_wep_ap(wlan_core_data* data)
{
	u8 *ie;
	u8 *hex_key_str[4] = {NULL, NULL, NULL, NULL};
	size_t hex_key_len;
	int connected = 0;
	int index = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct associate_params as_params;
	memset(&as_params, 0, sizeof(as_params));

	memset(&data->cmd->bssid, 0, sizeof(data->cmd->bssid));
	memset(&data->cmd->auth_bssid, 0, sizeof(data->cmd->auth_bssid));

	if(!ssid)
		return -1;

	BLTS_DEBUG("wep keys [%s]\n" , data->cmd->wep_keys);
	char* keys = data->cmd->wep_keys;
	char* saveptr = NULL;
	char* token;
	token = strtok_r(keys, " ", &saveptr); // ->  "i:t:pass" token

	while (token != NULL)
	{
		char* key = 0;
		char* part;
		char* chk;
		char* index_str = strtok_r(token, ":", &part);

		/* get wep key index (0 - 3) */
		index = strtol(index_str, &chk, 10);
		if (*chk != 0 || index < 0 || index > 4)
		{
			BLTS_ERROR("\tInvalid key index %s\n", index_str);
			goto error;
		}

		/* get wep key type (ascii / hex) */
		char* type_str = strtok_r(part, ":", &key);
		if(type_str[0] == 'a')
		{
			/* handle ASCII key */
			as_params.wep_key[index] = (u8*)key;
			as_params.wep_key_len[index] = strlen(key);
		}
		else if(type_str[0] == 'h')
		{
			/* handle HEX key */
			size_t hlen = strlen(key);
			if (hlen % 1)
			{
				BLTS_ERROR("\tInvalid hex key value %s\n", key);
				goto error;
			}

			hex_key_len = hlen / 2;
			hex_key_str[index] = malloc(hex_key_len);
			if (hex_key_str[index] == NULL)
			{
				BLTS_ERROR("hex_key_str malloc failed!\n");
			   	goto error;
			}
			if (hexstr2bin(key, hex_key_str[index], hex_key_len))
			{
				BLTS_ERROR("hexstr2bin conversion failed!\n");
			    goto error;
			}

		    as_params.wep_key[index] = hex_key_str[index];
		    as_params.wep_key_len[index] = hex_key_len;
		}
		else
		{
			BLTS_ERROR("\tInvalid key type %s\n", type_str);
			goto error;
		}

		BLTS_DEBUG("wep key index:%d type:%s key:%s\n", index, type_str, key);
		token = strtok_r(NULL, " ", &saveptr); // ->  "i:t:pass" token
	}
	as_params.wep_tx_keyidx = data->cmd->wep_keyidx;
	BLTS_DEBUG("wep_tx_keyidx:%d\n", data->cmd->wep_keyidx);

	if(nl80211_scan_oneshot(data, (u8*)ssid, strlen((const char *)ssid)))
	{
		BLTS_ERROR("\nERROR wlan scanning failed!\n");
		goto error;
	}

	struct scan_res* bss = get_bss_by_ssid(data, (u8*)ssid, strlen((const char *)ssid));

	if (!bss)
	{
		BLTS_ERROR("ERROR cannot find SSID: %s\n", ssid);
		goto error;
	}

	ie = (u8*)scan_get_ie(bss, WLAN_EID_SSID);

	as_params.bssid = bss->bssid;
	as_params.ssid = ie ? ie + 2 : (u8 *) "";
	as_params.ssid_len = ie ? ie[1] : 0;
	as_params.freq = bss->freq;
	as_params.auth_alg = WPA_AUTH_ALG_SHARED;

	if(nl80211_connect_oneshot(data, &as_params))
	{
		BLTS_ERROR("Connection failed!\n");
		goto error;
	}

	if(!data->cmd->associated) {
		BLTS_ERROR("Not associated, cannot test.\n");
		goto error;
	}

	connected = 1;

	if(send_dhcp_discover(data))
	{
		BLTS_ERROR("Authentication failed!\n");
		goto error;
	}

	if (nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING))
	{
		BLTS_ERROR("Failed to disconnect from " MACSTR "\n", MAC2STR(bss->bssid));
		goto error;
	}

	/* check that mlme events AUTHENTICATE and ASSOCIATE really filled addresses */
	if (!data->cmd->bssid || !data->cmd->auth_bssid)
		goto error;

	/* free memory allocated for hex keys */
	for (index = 0; index < 4 ; index++)
		if( hex_key_str[index] != NULL)
			free(hex_key_str[index]);
	return 0;

error:
	if(connected)
		nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING);
	for (index = 0; index < 4 ; index++)
		if( hex_key_str[index] != NULL)
			free(hex_key_str[index]);
	return -1;
}

int disconnect_from_open_ap(wlan_core_data* data, int disconnect_type)
{
	u8 *ie;
	int connected = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct associate_params as_params;

	memset(&as_params, 0, sizeof(as_params));

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

	ie = (u8*)scan_get_ie(bss, WLAN_EID_SSID);

	as_params.bssid = bss->bssid;
	as_params.ssid = ie ? ie + 2 : (u8 *) "";
	as_params.ssid_len = ie ? ie[1] : 0;
	as_params.freq = bss->freq;
	as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	if (setup_event_reader_thread(data))
	{
		BLTS_ERROR("Setup disconnect reader thread failed!\n");
		return -1;
	}

	if(nl80211_connect_oneshot(data, &as_params))
	{
		BLTS_ERROR("Connection failed!\n");
		goto error;
	}

	if(!data->cmd->associated) {
		BLTS_ERROR("Not associated, cannot test.\n");
		goto error;
	}
	connected = 1;

	if(send_dhcp_discover(data))
	{
		BLTS_ERROR("Authentication failed!\n");
		goto error;
	}

	switch (disconnect_type)
	{
		case DEAUTHENTICATE:
			if(nl80211_deauthenticate(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING))
				goto error;
			break;
		case DISASSOCIATE:
			if(nl80211_disassociate(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING))
				goto error;
			break;
		case AP_LOSS:
			BLTS_DEBUG("\n\nTurn off AP to pass this test\n\n");
			break;
		default:
			BLTS_ERROR("Unknown disconnect function type - test failed!\n");
			goto error;
	}

	if(wait_for_cmd(NL80211_CMD_DISCONNECT, 60.0))
	{
		BLTS_ERROR("Waiting for disconnect command failed!\n");
		goto error;
	}

	stop_event_reader_thread();
	return 0;

error:
	if(connected)
		nl80211_disconnect(data, bss->bssid, WLAN_REASON_DEAUTH_LEAVING);
	stop_event_reader_thread();
	return -1;
}

int disconnect_from_open_adhoc_network(wlan_core_data* data)
{
	u8 *ie;
	int res = 0;
	int joined = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct associate_params as_params;
	memset(&as_params, 0, sizeof(as_params));

	memset(&data->cmd->adhoc_bssid, 0, sizeof(data->cmd->adhoc_bssid));

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

	ie = (u8*)scan_get_ie(bss, WLAN_EID_SSID);

	as_params.bssid = bss->bssid;
	as_params.ssid = ie ? ie + 2 : (u8 *) "";
	as_params.ssid_len = ie ? ie[1] : 0;
	as_params.freq = bss->freq;
	as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	if(nl80211_join_ibss(data, &as_params))
	{
		BLTS_ERROR("Join ibss failed!\n");
		goto error;
	}
	joined = 1;

	if(do_test_data_sending(data, &as_params))
	{
		BLTS_ERROR("Send test data failed!\n");
		goto error;
	}

	res = nl80211_scan_for_link_data(data);
	if(res != 1)
	{
		BLTS_DEBUG("Connected link not found in scan results!\n");
		goto error;
	}

	if (nl80211_leave_ibss(data))
	{
		BLTS_ERROR("Leave from %s failed!\n", as_params.ssid);
		goto error;
	}

	res = nl80211_scan_for_link_data(data);

	if(res != 0)
	{
		BLTS_DEBUG("Link not disconnected!\n");
		goto error;
	}

	return 0;

error:
	if(joined)
		nl80211_leave_ibss(data);
	return -1;
}
