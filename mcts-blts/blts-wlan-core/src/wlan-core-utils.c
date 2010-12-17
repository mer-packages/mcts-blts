/* wlan-core-utils.h -- WLAN core utility functions

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
 * Utility functions adapted from hostapd implementation;
 * see driver_nl80211.c and common.c
 * License headers for hostapd code:
 *
 * WPA Supplicant - driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2003-2008, Jouni Malinen <j@w1.fi>
 *
 * wpa_supplicant/hostapd / common helper functions, etc.
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
   The following functions are originally from iw tool (util.c)
   mac_addr_n2a()
   mac_addr_a2n()
   ieee80211_channel_to_frequency()
   ieee80211_frequency_to_channel()

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

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>

#include <blts_log.h>

#include "wlan-core-utils.h"
#include "wlan-core-eloop.h"

struct family_data {
        const char *group;
        int id;
};

static int nl80211_get_power_save(wlan_core_data* data);

static int ack_handler(struct nl_msg *msg, void *arg)
{
      int *err = arg;
      *err = 0;
      return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
      int *ret = arg;
      *ret = 0;
      return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
                   void *arg)
{
      int *ret = arg;
      *ret = err->error;
	  BLTS_ERROR("\n nl80211 error %d\n", err->error);
      return NL_SKIP;
}

/* from hostapd driver_nl80211.c */
static int family_handler(struct nl_msg *msg, void *arg)
{
	struct family_data *res = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int i;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i)
	{
		struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
		nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
			  nla_len(mcgrp), NULL);
		if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
		    strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
			       res->group,
			       nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
			continue;
		res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	};

	return NL_SKIP;
}

/* Adapted from hostapd driver_nl80211.c */
int nl_get_multicast_id(wlan_core_data* data, const char *family, const char *group)
{
	struct nl_msg *msg;
	int ret = -1;
	struct family_data res = { group, -ENOENT };

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;
	genlmsg_put(msg, 0, 0, genl_ctrl_resolve(data->nl_handle, "nlctrl"),
		    0, 0, CTRL_CMD_GETFAMILY, 0);
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = send_and_recv_msgs(data, msg, family_handler, &res);
	msg = NULL;
	if (ret == 0)
		ret = res.id;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

static void handle_term(int sig, void *eloop_ctx, void *signal_ctx)
{
        BLTS_DEBUG("Signal %d received - terminating\n", sig);
        eloop_terminate();
}

/* Adapted from hostapd driver_nl80211.c */
static int nl80211_get_ifflags_ifname(wlan_core_data* data,
		const char *ifname, int *flags)
{
	struct ifreq ifr;

	if(!ifname)
		return -EINVAL;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;

	if (ioctl(data->ioctl_sock, SIOCGIFFLAGS, (caddr_t) &ifr) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl[SIOCGIFFLAGS]");
		return -1;
	}
	*flags = ifr.ifr_flags & 0xffff;
	return 0;
}

static int fill_ifce_mac(wlan_core_data *data, const char *ifname)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;

	if (ioctl(data->ioctl_sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl[SIOCGIFHWADDR]");
		return -1;
	}
	memcpy(data->our_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	return 0;
}



/**
 * Get interface flags (SIOCGIFFLAGS)
 */
static int nl80211_get_ifflags(wlan_core_data* data, int *flags)
{
	return nl80211_get_ifflags_ifname(data, data->cmd->ifname, flags);
}

/* Adapted from hostapd driver_nl80211.c */
static int nl80211_set_ifflags_ifname(wlan_core_data* data,
	const char *ifname, int flags)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	ifr.ifr_flags = flags & 0xffff;

	if (ioctl(data->ioctl_sock, SIOCSIFFLAGS, (caddr_t) &ifr) < 0)
	{
		BLTS_LOGGED_PERROR("SIOCSIFFLAGS");
		return -1;
	}
	return 0;
}

/**
 * Set interface flags (SIOCSIFFLAGS)
 */
static int nl80211_set_ifflags(wlan_core_data* data, int flags)
{
	return nl80211_set_ifflags_ifname(data, data->cmd->ifname, flags);
}

/**
 * Set wireless mode (infra/adhoc), SIOCSIWMODE
 * mode: 0 = infra/BSS (associate with an AP), 1 = adhoc/IBSS
 */
int nl80211_set_mode(wlan_core_data* data, int mode)
{
	int ret = -1;
	int flags = 0;
	struct nl_msg *msg;

	int retry = 0;

	BLTS_DEBUG("Set wireless mode %d (0 = infra/BSS , 1 = adhoc/IBSS)\n", mode);

	do
	{
		msg = nlmsg_alloc();
		if (!msg)
			return -1;

		/* mac80211 doesn't allow mode changes while the device is up, so
		 * take the device down, try to set the mode again, and bring the
		 * device back up if the first attempt failed. */
		if(retry)
		{
			BLTS_DEBUG("Taking the device down and retrying mode change again...\n");
			if (nl80211_get_ifflags(data, &flags) == 0)
			{
				nl80211_set_ifflags(data, flags & ~IFF_UP);
			}
		}

		genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
				    0, NL80211_CMD_SET_INTERFACE, 0);
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
		NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE,
				    mode ? NL80211_IFTYPE_ADHOC : NL80211_IFTYPE_STATION);

		ret = send_and_recv_msgs(data, msg, NULL, NULL);

		/* set the device up again */
		if(retry)
		{
			nl80211_get_ifflags(data, &flags);
			nl80211_set_ifflags(data, flags | IFF_UP);
		}

		if (!ret && !retry)
			break;

		retry++;
	} while (retry <= 1);

	return ret;

nla_put_failure:
	BLTS_ERROR("Failed to set interface mode: %d (%s)", ret, strerror(-ret));
	return -1;
}

static int nl80211_init_set_mode(wlan_core_data* data, int mode)
{
	int ret = 0;
	int flags = 0;
	struct nl_msg *msg;

	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	BLTS_DEBUG("Set wireless mode %d (0 = infra/BSS , 1 = adhoc/IBSS)\n", mode);
	BLTS_DEBUG("Taking the device down and do mode change ...\n");

	if (nl80211_get_ifflags(data, &flags))
	{
		BLTS_ERROR("Get interface flags failed!\n");
		return -1;
	}

	/* set the device down because mac80211 doesn't allow
	   mode changes while the device is up */
	if(nl80211_set_ifflags(data, flags & ~IFF_UP))
	{
		BLTS_ERROR("Set the device down failed!\n");
		return -1;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
			    0, NL80211_CMD_SET_INTERFACE, 0);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
	NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE,
			    mode ? NL80211_IFTYPE_ADHOC : NL80211_IFTYPE_STATION);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);

	return ret;

nla_put_failure:
	BLTS_ERROR("Failed to set interface mode: %d (%s)", ret, strerror(-ret));

	return -1;
}

int nl80211_finish_device_init(wlan_core_data* data, int test_num)
{
	int flags;
	int mode;
	struct ifreq ifr;

	/* hardware enumeration case does not need any further actions */
	if(test_num == CORE_HARDWARE_ENUMERATION)
		return 0;

	if(!data || !data->cmd->ifname)
	{
		BLTS_ERROR("interface name missing!\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, data->cmd->ifname, sizeof(ifr.ifr_name));

	/* test interface existence */
	if (ioctl(data->ioctl_sock, SIOCGIFINDEX, &ifr) == -1)
	{
		BLTS_LOGGED_PERROR("SIOCGIFINDEX");
	    	return -1;
	}

	BLTS_DEBUG("Finishing device:%s initialization before testing...\n", data->cmd->ifname);
	data->cmd->ifindex = if_nametoindex(data->cmd->ifname);

	if(test_num == CORE_DISCONNECT_FROM_ADHOC_NETWORK || 
		test_num == CORE_ESTABLISH_NEW_ADHOC_NETWORK ||
		test_num == CORE_JOIN_ESTABLISHED_ADHOC_NETWORK)
		mode = 1;
	else
		mode = 0;

	if (nl80211_init_set_mode(data, mode) < 0)
	{
		BLTS_DEBUG("Could not configure device to use mode:%d (0 = infra/BSS, 1 = adhoc/IBSS)\n", mode);
		return -1;
	}

	if (nl80211_get_ifflags(data, &flags) != 0)
	{
		BLTS_DEBUG("Could not get interface '%s' flags\n", data->cmd->ifname);
	}
	else if (!(flags & IFF_UP))
	{
		if (nl80211_set_ifflags(data, flags | IFF_UP) != 0)
		{
			BLTS_DEBUG("Could not set interface '%s' UP\n", data->cmd->ifname);
		}
	}

	BLTS_DEBUG("Power save mode...\n");
	if(data->flags&CLI_FLAG_POWER_SAVE)
		data->cmd->power_save = 1;
	else
		data->cmd->power_save = 0;

	/* ignore errors, because not necessarily supported by all drivers */
	nl80211_set_power_save(data);
	nl80211_get_power_save(data);

	if(fill_ifce_mac(data, data->cmd->ifname) < 0)
	{
		BLTS_ERROR("WARNING: Failed to fetch local hwaddr\n");
		return -1;
	}
	else
	{
		BLTS_DEBUG("Local device is at " MACSTR "\n", MAC2STR(data->our_addr));
	}

	return 0;
}

int nl80211_init(wlan_core_data* data)
{
	int err;
	int flags = 0;

	if (data == NULL)
		return -1;

	data->nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (data->nl_cb == NULL)
	{
		BLTS_ERROR("Failed to allocate cb\n");
		return -ENOMEM;
	}

	data->nl_handle = nl_handle_alloc_cb(data->nl_cb);
	if (data->nl_handle == NULL)
	{
		nl_cb_put(data->nl_cb);
		data->nl_cb = NULL;
		return -ENOMEM;
	}

	if (genl_connect(data->nl_handle))
	{
		BLTS_ERROR("Failed to connect to generic netlink\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	data->nl_cache = genl_ctrl_alloc_cache(data->nl_handle);
	if (!data->nl_cache)
	{
		BLTS_ERROR("Failed to allocate generic netlink cache\n");
		err = -ENOMEM;
		goto out_handle_destroy;
	}

	data->nl80211 = genl_ctrl_search_by_name(data->nl_cache, "nl80211");
	if (data->nl80211 == NULL)
	{
		BLTS_ERROR("nl80211 not available on nl ifce\n");
		err = -ENOENT;
		goto out_cache_free;
	}

	err = nl_get_multicast_id(data, "nl80211", "scan");
	if (err >= 0)
	{
		err = nl_socket_add_membership(data->nl_handle, err);
	}
	else
		BLTS_ERROR("Could not add multicast membership for scan events: %d (%s)\n",
			   err, strerror(-err));
	/*
	if (err < 0)
	{
		BLTS_ERROR("Could not add multicast membership for scan events: %d (%s)\n",
			   err, strerror(-err));
		goto out_cache_free;
	}
    */
	err = nl_get_multicast_id(data, "nl80211", "mlme");
	if (err >= 0)
	{
		err = nl_socket_add_membership(data->nl_handle, err);
	}
	else
		BLTS_ERROR("Could not add multicast membership for mlme events: %d (%s)\n",
			   err, strerror(-err));
	/*
	if (err < 0)
	{
		BLTS_ERROR("Could not add multicast membership for mlme events: %d (%s)\n",
			   err, strerror(-err));
		goto out_cache_free;
	}
	*/

	eloop_init(data);
	eloop_register_signal(SIGINT, handle_term, NULL);
	eloop_register_signal(SIGTERM, handle_term, NULL);

	data->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);

	if (data->ioctl_sock < 0)
	{
		genl_family_put(data->nl80211);
		BLTS_ERROR("socket(PF_INET,SOCK_DGRAM) failed\n");
		err = -ENOENT;
		goto out_cache_free;
	}

	if (nl80211_get_ifflags(data, &flags) == 0)
	{
		nl80211_set_ifflags(data, flags & ~IFF_UP);
	}

	return 0;

out_cache_free:
	nl_cache_free(data->nl_cache);
	data->nl_cache = NULL;
out_handle_destroy:
	nl_handle_destroy(data->nl_handle);
	nl_cb_put(data->nl_cb);

	return -1;
}

void nl80211_cleanup(wlan_core_data* data)
{
	if(data && data->cmd->ifname)
	{
		int flags = 0;
		if (nl80211_get_ifflags(data, &flags) == 0)
			nl80211_set_ifflags(data, flags & ~IFF_UP);
	}

	close(data->ioctl_sock);
	eloop_unregister_read_sock(nl_socket_get_fd(data->nl_handle));
	eloop_destroy();

	genl_family_put(data->nl80211);

	nl_cache_free(data->nl_cache);
	nl_handle_destroy(data->nl_handle);

	nl_cb_put(data->nl_cb);
}

char* find_parent_phy_device(const char *interface)
{
	DIR *dir;
	struct dirent *file;
	char path[FILENAME_MAX];
	char *name;

	snprintf(path, FILENAME_MAX,
	"/sys/class/net/%s/phy80211/device/ieee80211", interface);

	dir = opendir(path);

	if (!dir)
		return NULL;

	while ( (file = readdir(dir)) )
	{
		if (strncmp("phy", file->d_name, 3) == 0)
		{
			name = strdup(file->d_name);
			closedir(dir);
			return name;
		}
	}

	closedir(dir);

	return NULL;
}

int phy_lookup(char *name)
{
	char buf[200];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);

	return atoi(buf);
}


/* Adapted from hostapd driver_nl80211.c */
int send_and_recv_msgs(wlan_core_data* data,
                        struct nl_msg *msg,
                        int (*valid_handler)(struct nl_msg *, void *),
                        void *valid_data)
{
	int err = -ENOMEM;

	struct nl_cb *cb = nl_cb_clone(data->nl_cb);

	if (!cb)
		goto out;

	if(data->trace_netlink)
		nl_msg_dump(msg, stderr);

	err = nl_send_auto_complete(data->nl_handle, msg);

	if (err < 0)
	{
		BLTS_DEBUG("ERROR in sending message %d\n",err);
		goto out;
	}

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, valid_data);

	while (err > 0)
		nl_recvmsgs(data->nl_handle, cb);

out:
	nl_cb_put(cb);
	nlmsg_free(msg);

	return err;
}

/**
 * from iw util.c
 * Converts frequency to channel number
 */
int ieee80211_frequency_to_channel(int freq)
{
	if (freq == 2484)
		return 14;

	if (freq < 2484)
		return (freq - 2407) / 5;

	/* FIXME: dot11ChannelStartingFactor (802.11-2007 17.3.8.3.2) */
	return freq/5 - 1000;
}

/**
 * from iw util.c
 * Converts channel number to frequency
 */
int ieee80211_channel_to_frequency(int chan)
{
	if (chan < 14)
		return 2407 + chan * 5;

	if (chan == 14)
		return 2484;

	/* FIXME: 802.11j 17.3.8.3.2 */
	return (chan + 1000) * 5;
}

/* from iw util.c */
int mac_addr_a2n(unsigned char *mac_addr, char *arg)
{
        int i;

        for (i = 0; i < ETH_ALEN ; i++) {
                int temp;
                char *cp = strchr(arg, ':');
                if (cp)
                {
                        //*cp = 0;
                        cp++;
                }
                if (sscanf(arg, "%x", &temp) != 1)
                        return -1;
                if (temp < 0 || temp > 255)
                        return -1;

                mac_addr[i] = temp;
                if (!cp)
                        break;
                arg = cp;
        }
        if (i < ETH_ALEN - 1)
                return -1;

        return 0;
}

/* from iw util.c */
int mac_addr_n2a(char *mac_addr, unsigned char *arg)
{
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN; i++)
	{
		if (i == 0)
		{
			sprintf(mac_addr + l, "%02x", arg[i]);
			l += 2;
		}
		else
		{
			sprintf(mac_addr + l, ":%02x", arg[i]);
			l += 3;
		}
	}
	return 0;
}

/* from hostapd common.c */
static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/* from hostapd common.c */
static int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * from hostapd common.c
 * Converts ASCII hex string into binary data
 */
int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++)
	{
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

/* from hostapd */
int set_iface_flags(int sock, const char *ifname, int dev_up)
{
	struct ifreq ifr;

	if (sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
		BLTS_ERROR("Could not read interface %s flags: %s\n",
			   ifname, strerror(errno));
		return -1;
	}

	if (dev_up) {
		if (ifr.ifr_flags & IFF_UP)
			return 0;
		ifr.ifr_flags |= IFF_UP;
	} else {
		if (!(ifr.ifr_flags & IFF_UP))
			return 0;
		ifr.ifr_flags &= ~IFF_UP;
	}

	if (ioctl(sock, SIOCSIFFLAGS, &ifr) != 0) {
		BLTS_ERROR("Could not set interface %s flags: %s\n",
			   ifname, strerror(errno));
		return -1;
	}

	return 0;
}

static int power_save_handler(struct nl_msg *msg, void *arg)
{
	wlan_core_data* data =  arg;
	struct nlattr *attrs[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	const char *s = "undefined";

	nla_parse(attrs, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[NL80211_ATTR_PS_STATE])
		return NL_SKIP;

	switch (nla_get_u32(attrs[NL80211_ATTR_PS_STATE]))
	{
	case NL80211_PS_ENABLED:
		/* unexpect power save mode */
		if(data->cmd->power_save != 1) data->cmd->power_save = -1;
		s = "on";
		break;
	case NL80211_PS_DISABLED:
		/* unexpect power save mode */
		if(data->cmd->power_save != 0) data->cmd->power_save = -1;
		s = "off";
		break;
	default:
		data->cmd->power_save = -1;
	}

	BLTS_DEBUG("Power save: %s\n", s);

	return NL_SKIP;
}

int nl80211_set_power_save(wlan_core_data* data)
{
    int ret = 0;
	struct nl_msg *msg;
	enum nl80211_ps_state ps_state;

	msg = nlmsg_alloc();
	if (!msg)
	{
		nlmsg_free(msg);
		return -1;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
			NL80211_CMD_SET_POWER_SAVE, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	if (data->cmd->power_save)
	{
		ps_state = NL80211_PS_ENABLED;
	}
	else
	{
		ps_state = NL80211_PS_DISABLED;
	}

	NLA_PUT_U32(msg, NL80211_ATTR_PS_STATE, ps_state);

	ret = send_and_recv_msgs(data, msg, NULL/*TODO: verify event with power_save_handler*/, NULL);

	msg = NULL;
	if (ret)
	{
		BLTS_DEBUG("Set power save failed: ret=%d (%s)\n", ret, strerror(-ret));
		goto nla_put_failure;
	}

	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

static int nl80211_get_power_save(wlan_core_data* data)
{
    int ret = 0;
	struct nl_msg *msg;

	msg = nlmsg_alloc();
	if (!msg)
	{
		nlmsg_free(msg);
		return -1;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0, 0,
			NL80211_CMD_GET_POWER_SAVE, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);


	ret = send_and_recv_msgs(data, msg, power_save_handler, data);

	msg = NULL;
	if (ret)
	{
		BLTS_DEBUG("Get power save failed: ret=%d (%s)\n", ret, strerror(-ret));
		goto nla_put_failure;
	}

	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

int nl80211_set_key_ccmp(wlan_core_data *data, const u8 *addr,
				int key_idx, int set_tx,
				const u8 *seq, size_t seq_len,
				const u8 *key, size_t key_len)
{
	struct nl_msg *msg;
	int ret;

	msg = nlmsg_alloc();
	if(!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
		0, NL80211_CMD_NEW_KEY, 0);

	NLA_PUT(msg, NL80211_ATTR_KEY_DATA, key_len, key);
	NLA_PUT_U32(msg, NL80211_ATTR_KEY_CIPHER,
		WLAN_CIPHER_SUITE_CCMP);

	if(seq && seq_len)
		NLA_PUT(msg, NL80211_ATTR_KEY_SEQ, seq_len, seq);

	if (addr && memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN))
		NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, addr);

	NLA_PUT_U8(msg, NL80211_ATTR_KEY_IDX, key_idx);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);

	if(ret) {
		BLTS_ERROR("ERROR - NL80211_CMD_NEW_KEY failed (%d) - %s",
			   ret, strerror(-ret));
		return ret;
	}

	/* if (data->nlmode == NL80211_IFTYPE_AP && addr) */
	/* 	return ret; */ /* <- uncomment once we test as ap, too */

	msg = nlmsg_alloc();
	if(!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
		0, NL80211_CMD_SET_KEY, 0);
	NLA_PUT_U8(msg, NL80211_ATTR_KEY_IDX, key_idx);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, data->cmd->ifindex);
	NLA_PUT_FLAG(msg, NL80211_ATTR_KEY_DEFAULT);

	ret = send_and_recv_msgs(data, msg, NULL, NULL);
	if (ret == -ENOENT)
		ret = 0;
	if (ret)
		BLTS_ERROR("ERROR - NL80211_CMD_SET_KEY (default) failed (%d)"
			   " - %s)", ret, strerror(-ret));
	return ret;

nla_put_failure:
	return -1;
}

int nl80211_set_supp_port(wlan_core_data *data, int authorized)
{
	struct nl_msg *msg;
	struct nl80211_sta_flag_update upd;

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, 0, 0, genl_family_get_id(data->nl80211), 0,
		    0, NL80211_CMD_SET_STATION, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX,
		    if_nametoindex(data->cmd->ifname));
	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, data->cmd->bssid);

	memset(&upd, 0, sizeof(upd));
	upd.mask = BIT(NL80211_STA_FLAG_AUTHORIZED);
	if (authorized)
		upd.set = BIT(NL80211_STA_FLAG_AUTHORIZED);
	NLA_PUT(msg, NL80211_ATTR_STA_FLAGS2, sizeof(upd), &upd);

	return send_and_recv_msgs(data, msg, NULL, NULL);
 nla_put_failure:
	return -ENOBUFS;
}
