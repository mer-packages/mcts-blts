/* wlan-core-adhoc.c -- WLAN core adhoc functions

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
#include <stdio.h>
#include <blts_log.h>

#include "wlan-core-adhoc.h"
#include "wlan-core-connect.h"
#include "wlan-core-scan.h"
#include "wlan-core-utils.h"

#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <arpa/inet.h>


#include <sys/ioctl.h>

#define ETH_TYPE 	0x8000
#define RETRIES 	50
#define TESTDATA 	"ADHOC-TEST"

static int raw_socket_init(wlan_core_data* data, struct sockaddr_ll* socket_address, unsigned char* src_mac, unsigned char* bcast_mac)
{
	int i;
	int fd;
	struct ifreq ifr;

	if (!data || !data->cmd->ifname)
		return -1;

	if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		BLTS_ERROR("socket call failed\n");
		return -1;
	}

	BLTS_DEBUG("Successfully opened socket: %d\n", fd);

	strncpy(ifr.ifr_name, data->cmd->ifname, IFNAMSIZ);
	ifr.ifr_ifindex = data->cmd->ifindex;

	/* retrieve own MAC */
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
	{
		BLTS_LOGGED_PERROR("SIOCGIFINDEX");
		return -1;
	}
	for (i = 0; i < 6; i++)
	{
		src_mac[i] = ifr.ifr_hwaddr.sa_data[i];
	}

	socket_address->sll_family = PF_PACKET;
	socket_address->sll_protocol = htons(ETH_TYPE);
	socket_address->sll_ifindex = data->cmd->ifindex;
	socket_address->sll_pkttype = PACKET_BROADCAST;
	socket_address->sll_halen = ETH_ALEN;
	for (i = 0; i < 6; i++)
	{
		socket_address->sll_addr[i] = bcast_mac[i];
	}
	socket_address->sll_addr[6] = 0x00;
	socket_address->sll_addr[7] = 0x00;

	BLTS_DEBUG("raw_socket_init:Host MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4],
			src_mac[5]);

	BLTS_DEBUG("raw_socket_init:Dest MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			bcast_mac[0], bcast_mac[1], bcast_mac[2], bcast_mac[3],
			bcast_mac[4], bcast_mac[5]);

	return fd;
}

int send_test_data(wlan_core_data* data, struct associate_params *params, char* test_data, int count)
{
	int i;
	int raw = -1;
	int res = -1;
	int tries = count;
	void* buffer = NULL;
	unsigned char* eth_head;
	unsigned char* eth_data;
	struct ethhdr *eh;
	struct sockaddr_ll socket_address;
	memset(&socket_address, 0, sizeof(socket_address));

	unsigned char bcast_mac[6] =
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char src_mac[6] =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (!count)
		tries = RETRIES;

	raw = raw_socket_init(data, &socket_address, src_mac, bcast_mac);

	if (raw < 0)
	{
		BLTS_ERROR("Open raw socket failed!\n");
		return -1;
	}

	buffer = (void*) malloc(ETHER_MAX_LEN);
	eth_head = buffer;
	eth_data = buffer + ETH_HLEN;
	eh = (struct ethhdr *) eth_head;

	BLTS_DEBUG("Send test packets...\n");
	while (tries--)
	{
		memcpy((void *) eh->h_dest, (void*) bcast_mac, ETH_ALEN);
		memcpy((void *) eh->h_source, (void*) src_mac, ETH_ALEN);
		eh->h_proto = htons(ETH_TYPE);

		int len = strlen(test_data);

		for (i = 0; i < len; i++)
			eth_data[i] = test_data[i];

		/* send packet */
		i = sendto(raw, buffer, len + ETHER_HDR_LEN, 0,
				(struct sockaddr*) &socket_address, sizeof(socket_address));

		if (i == -1)
		{
			BLTS_LOGGED_PERROR("send_test_data sendto():");
			res = -1;
			goto out;
		}
		res = 0;
		sleep(1); /* sleep before next packet is sent */
	}

out:
	close(raw);
	free(buffer);
	return res;
}

static int receive_test_data(wlan_core_data* data, struct associate_params *params, char* exp_data, int count)
{
	int i;
	int res = -1;
	int raw = -1;
	int tries = count;
	void* buffer = NULL;
	unsigned char* eth_head;
	unsigned char* eth_data;
	struct ethhdr *eh;
	struct sockaddr_ll socket_address;
	memset(&socket_address, 0, sizeof(socket_address));

	unsigned char bcast_mac[6] =
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	unsigned char src_mac[6] =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (!count)
		tries = RETRIES;

	raw = raw_socket_init(data, &socket_address, src_mac, bcast_mac);

	if (raw < 0)
	{
		BLTS_ERROR("Open raw socket failed!\n");
		return -1;
	}

	buffer = (void*)malloc(ETHER_MAX_LEN);
	eth_head = buffer;
	eth_data = buffer + ETH_HLEN;
	eh = (struct ethhdr *)eth_head;

	BLTS_DEBUG("Receive test packets...\n");
	while(tries--)
	{
		i = recvfrom(raw, buffer, ETHER_MAX_LEN, 0, NULL, NULL);

		if (i == -1)
		{
			BLTS_LOGGED_PERROR("receive_test_data recvfrom():");
			res = -1;
			goto out;
		}

		if(eh->h_proto == ntohs(ETH_TYPE))
		{
			BLTS_DEBUG("Received packet from: %02X:%02X:%02X:%02X:%02X:%02X\n",
					eh->h_source[0], eh->h_source[1], eh->h_source[2], eh->h_source[3],
					eh->h_source[4], eh->h_source[5]);

			int len = strlen(exp_data);

			/* compare payload with expected data */
			for(i=0; i<len; i++)
			{
			  BLTS_DEBUG("%c", eth_data[i]);
			  if(eth_data[i] != exp_data[i])
			  {
				  BLTS_DEBUG("\nMismatch %c <> %c...continue\n",
						  eth_data[i] , exp_data[i]);
				  continue;
			  }
			  if(i+1 == len)
			  {
				  BLTS_DEBUG("\nRight package received!\n");
				  res = 0;
				  goto out;
			  }
			}
		}
		sleep(1); /* sleep before next packet is handled */
	}
out:
	close(raw);
	free(buffer);
	return res;
}


int do_test_data_sending(wlan_core_data* data, struct associate_params *params)
{
	return send_test_data(data, params, "ADHOC-PING", 10);
}


int join_existing_open_adhoc_network(wlan_core_data* data)
{
	u8 *ie;
	int res = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct associate_params as_params;
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

	as_params.bssid = bss->bssid;
	as_params.ssid = ie ? ie + 2 : (u8 *) "";
	as_params.ssid_len = ie ? ie[1] : 0;
	as_params.freq = bss->freq;
	as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	if(nl80211_join_ibss(data, &as_params))
	{
		BLTS_ERROR("Join ibss failed!\n");
		return -1;
	}

	if(send_test_data(data, &as_params, TESTDATA, RETRIES))
	{
		BLTS_ERROR("Send test data failed!\n");
		res = -1;
	}

	if (nl80211_leave_ibss(data))
	{
		BLTS_ERROR("Leave from %s failed!\n", as_params.ssid);
		res = -1;
	}

	return res;
}

int create_open_adhoc_network(wlan_core_data* data)
{
	int res = 0;
	const u8 *ssid = (const u8 *) data->cmd->ssid;

	struct associate_params as_params;
	memset(&as_params, 0, sizeof(as_params));

	if(!ssid)
		return -1;

	BLTS_DEBUG("\nSearch for SSID: %s...\n", ssid);
	if(nl80211_scan_oneshot(data, ssid, strlen((const char *)ssid)))
	{
		BLTS_ERROR("\nERROR wlan scanning failed!\n");
		return -1;
	}

	struct scan_res* bss = get_bss_by_ssid(data, (u8*)ssid, strlen((const char *)ssid));

	if (bss)
	{
		BLTS_ERROR("Adhoc network with SSID: %s already exists!\n", ssid);
		return -1;
	}

	as_params.bssid = "";
	as_params.ssid = ssid;
	as_params.ssid_len = strlen((const char *)ssid);
	as_params.freq = ieee80211_channel_to_frequency(1); //TODO make configurable?
	as_params.auth_alg = WPA_AUTH_ALG_OPEN;

	if(nl80211_join_ibss(data, &as_params))
	{
		BLTS_ERROR("Join failed!\n");
		return -1;
	}

	if(receive_test_data(data, &as_params, TESTDATA, RETRIES))
	{
		BLTS_ERROR("Send test data failed!\n");
		res = -1;
	}

	if (nl80211_leave_ibss(data))
	{
		BLTS_ERROR("Leave from %s failed!\n", as_params.ssid);
		res = -1;
	}
	return res;
}
