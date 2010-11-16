/* wlan-core-dhcp.c -- WLAN core dhcp functions

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

   includes source code from udhcp client by Russ Dill <Russ.Dill@asu.edu>
*/

/* udhcp DHCP client
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <blts_log.h>
#include <blts_timing.h>

#include "wlan-core-dhcp.h"
#include "wlan-core-utils.h"

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>

#include <time.h>
#include <unistd.h>

#include <features.h>
#include <asm/types.h>

#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

struct config_t {
	char *interface;		/* The name of the interface to use */
	uint8_t *clientid;		/* Optional client id to use */
	uint8_t *hostname;		/* Optional hostname to use */
	int ifindex;			/* Index number of the interface to use */
	uint8_t arp[6];			/* Our arp address */
};

struct config_t config = {
	/* Default options. */
	interface: "wlan0",
	clientid: NULL,
	hostname: NULL,
	ifindex: 5,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
};

struct dhcp_option {
	char name[10];
	char flags;
	uint8_t code;
};

struct dhcp_message {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint32_t cookie;
	uint8_t options[308];
};

struct udp_dhcp_packet {
	struct iphdr ip;
	struct udphdr udp;
	struct dhcp_message data;
};

static int signal_pipe[2];

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT		67
#define CLIENT_PORT		68

#define DHCP_MAGIC		0x63825363

/* DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_REQUESTED_IP	0x32

#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_SERVER_ID		0x36
#define DHCP_PARAM_REQ		0x37
#define DHCP_MESSAGE		0x38
#define DHCP_MAX_SIZE		0x39
#define DHCP_VENDOR			0x3c
#define DHCP_CLIENT_ID		0x3d

#define DHCP_END		0xFF

/* miscellaneous defines */
#define MAC_BCAST_ADDR		(uint8_t *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2

#define OPTION_REQ	0x10 /* have the client request this option */
#define OPTION_LIST	0x20 /* There can be a list of 1 or more of these */

#define DHCP_MAGIC		0x63825363

#define BOOTREQUEST		1
#define BOOTREPLY		2

#define ETH_10MB		1
#define ETH_10MB_LEN	6

#define DHCPDISCOVER	1
#define DHCPOFFER		2

#define OPTION_FIELD	0
#define FILE_FIELD		1
#define SNAME_FIELD		2

#define VERSION "blts-wlan-core"
#define TYPE_MASK	0x0F

enum {
	OPTION_IP=1,
	OPTION_IP_PAIR,
	OPTION_STRING,
	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
	OPTION_S16,
	OPTION_U32,
	OPTION_S32
};

/* supported options are easily added here */
struct dhcp_option dhcp_options[] = {
	/* name[10]	flags					code */
	{"requestip",	OPTION_IP,			0x32},
	{"lease",		OPTION_U32,			0x33},
	{"dhcptype",	OPTION_U8,			0x35},
	{"serverid",	OPTION_IP,			0x36},
	{"message",		OPTION_STRING,		0x38},
	{"",			0x00,				0x00}
};

/* Lengths of the different option types */
int option_lengths[] = {
	[OPTION_IP] =		4,
	[OPTION_IP_PAIR] =	8,
	[OPTION_BOOLEAN] =	1,
	[OPTION_STRING] =	1,
	[OPTION_U8] =		1,
	[OPTION_U16] =		2,
	[OPTION_S16] =		2,
	[OPTION_U32] =		4,
	[OPTION_S32] =		4
};

/* Create a random xid */
unsigned long random_xid(void)
{
	static int initialized;
	if (!initialized) {
		int fd;
		unsigned long seed;

		fd = open("/dev/urandom", 0);
		if (fd < 0 || read(fd, &seed, sizeof(seed)) < 0) {
			BLTS_ERROR("Could not load seed from /dev/urandom\n");
			seed = time(0);
		}

		if (fd >= 0)
			close(fd);

		srand(seed);
		initialized++;
	}
	return rand();
}

/* Open a raw socket on given ifindex */
int raw_socket(int ifindex)
{
	int fd;
	struct sockaddr_ll sock = {};

    BLTS_DEBUG("Opening raw socket on ifindex %d\n", ifindex);

    if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
	BLTS_ERROR("socket call failed\n");
        return -1;
    }

    sock.sll_family = AF_PACKET;
    sock.sll_protocol = htons(ETH_P_IP);
    sock.sll_ifindex = ifindex;

    if (bind(fd, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
	BLTS_ERROR("bind call failed!\n");
        close(fd);
        return -1;
    }

    return fd;
}

long uptime(void)
{
	struct sysinfo info;
	sysinfo(&info);
	return info.uptime;
}

uint16_t checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;
	uint16_t *source = (uint16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		uint16_t tmp = 0;
		*(uint8_t *) (&tmp) = * (uint8_t *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int read_interface(char *interface, int *ifindex, uint32_t *addr, uint8_t *arp)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *our_ip;

	memset(&ifr, 0, sizeof(struct ifreq));
	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, interface);

		if (addr) {
			if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
				our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
				*addr = our_ip->sin_addr.s_addr;
				BLTS_DEBUG("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
			} else {
				BLTS_ERROR("SIOCGIFADDR failed, is the interface up and configured?\n");
				return -1;
			}
		}

		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			BLTS_DEBUG( "adapter index %d\n", ifr.ifr_ifindex);
			*ifindex = ifr.ifr_ifindex;
		} else {
			BLTS_ERROR("SIOCGIFINDEX failed!\n");
			return -1;
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
						BLTS_DEBUG("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
							arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
		} else {
			BLTS_ERROR("SIOCGIFHWADDR failed!\n");
			return -1;
		}
	} else {
		BLTS_ERROR("socket failed!\n");
		return -1;
	}
	close(fd);
	return 0;
}

/* Construct a ip/udp header for a packet, and specify the source and dest hardware address */
int raw_packet(struct dhcp_message *payload, uint32_t source_ip, int source_port,
		   uint32_t dest_ip, int dest_port, uint8_t *dest_arp, int ifindex)
{

	int fd;
	int result;
	struct sockaddr_ll dest;
	struct udp_dhcp_packet packet;

	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		BLTS_ERROR("socket call failed!\n");
		return -1;
	}

	memset(&dest, 0, sizeof(dest));
	memset(&packet, 0, sizeof(packet));

	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, dest_arp, 6);
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) {
		BLTS_ERROR("bind call failed!\n");
		close(fd);
		return -1;
	}

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source_ip;
	packet.ip.daddr = dest_ip;
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
	packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcp_message)); /* cheat on the psuedo-header */
	packet.ip.tot_len = packet.udp.len;
	memcpy(&(packet.data), payload, sizeof(struct dhcp_message));
	packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));

	packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));

	result = sendto(fd, &packet, sizeof(struct udp_dhcp_packet), 0, (struct sockaddr *) &dest, sizeof(dest));
	if (result <= 0) {
		BLTS_ERROR("write on socket failed!\n");
	}

	close(fd);
	return result;
}

/* return the position of the 'end' option (no bounds checking) */
int end_option(uint8_t *optionptr)
{
	int i = 0;

	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) i++;
		else i += optionptr[i + OPT_LEN] + 2;
	}
	return i;
}

int add_option_string(uint8_t *optionptr, uint8_t *string)
{
	int end = end_option(optionptr);

	/* end position + string length + option code/length + end option */
	if (end + string[OPT_LEN] + 2 + 1 >= 308) {
		BLTS_ERROR("Option 0x%02x did not fit into the packet!\n", string[OPT_CODE]);
		return 0;
	}
	BLTS_DEBUG("adding option 0x%02x\n", string[OPT_CODE]);
	memcpy(optionptr + end, string, string[OPT_LEN] + 2);
	optionptr[end + string[OPT_LEN] + 2] = DHCP_END;
	return string[OPT_LEN] + 2;
}

/* add a one to four byte option to a packet */
int add_simple_option(uint8_t *optionptr, uint8_t code, uint32_t data)
{
	char length = 0;
	int i;
	uint8_t option[2 + 4];
	uint8_t *u8;
	uint16_t *u16;
	uint32_t *u32;
	uint32_t aligned;
	u8 = (uint8_t *) &aligned;
	u16 = (uint16_t *) &aligned;
	u32 = &aligned;

	for (i = 0; dhcp_options[i].code; i++)
		if (dhcp_options[i].code == code) {
			length = option_lengths[dhcp_options[i].flags & TYPE_MASK];
		}

	if (!length) {
		BLTS_ERROR("Could not add option 0x%02x\n", code);
		return 0;
	}

	option[OPT_CODE] = code;
	option[OPT_LEN] = length;

	switch (length) {
		case 1: *u8 =  data; break;
		case 2: *u16 = data; break;
		case 4: *u32 = data; break;
	}
	memcpy(option + 2, &aligned, length);
	return add_option_string(optionptr, option);
}

void init_header(struct dhcp_message *packet, char type)
{
	memset(packet, 0, sizeof(struct dhcp_message));
	switch (type) {
	case DHCPDISCOVER:
		packet->op = BOOTREQUEST;
		break;
	case DHCPOFFER:
		packet->op = BOOTREPLY;
	}
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
    add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
}

/* initialize a packet with the proper defaults */
static void init_packet(struct dhcp_message *packet, char type)
{
	struct vendor  {
		char vendor, length;
		char str[sizeof("udhcp "VERSION)];
	} vendor_id = { DHCP_VENDOR,  sizeof("udhcp "VERSION) - 1, "udhcp "VERSION};

	init_header(packet, type);
	memcpy(packet->chaddr, config.arp, 6);
	if (config.clientid)
	    add_option_string(packet->options, config.clientid);
	if (config.hostname) add_option_string(packet->options, config.hostname);
	add_option_string(packet->options, (uint8_t *) &vendor_id);
}

/* Add a parameter request list for stubborn DHCP servers. Pull the data
 * from the struct in options.c. Don't do bounds checking here because it
 * goes towards the head of the packet. */
static void add_requests(struct dhcp_message *packet)
{
	int end = end_option(packet->options);
	int i, len = 0;

	packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
	for (i = 0; dhcp_options[i].code; i++)
		if (dhcp_options[i].flags & OPTION_REQ)
			packet->options[end + OPT_DATA + len++] = dhcp_options[i].code;
	packet->options[end + OPT_LEN] = len;
	packet->options[end + OPT_DATA + len] = DHCP_END;

}
/* Broadcast a DHCP discover packet to the network */
int send_discover(unsigned long xid, unsigned long requested)
{
	struct dhcp_message packet;

	init_packet(&packet, DHCPDISCOVER);
	packet.xid = xid;
	if (requested)
		add_simple_option(packet.options, DHCP_REQUESTED_IP, requested);

	add_requests(&packet);
	BLTS_DEBUG("***** Sending discover...\n");
	return raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
				SERVER_PORT, MAC_BCAST_ADDR, config.ifindex);
}

/* Setup the rfds - will return the max_fd for use with select */
int udhcp_sp_fd_set(fd_set *rfds, int extra_fd)
{
	FD_ZERO(rfds);
	FD_SET(signal_pipe[0], rfds);
	if (extra_fd >= 0) FD_SET(extra_fd, rfds);
	return signal_pipe[0] > extra_fd ? signal_pipe[0] : extra_fd;
}

/* Handle raw packets
 * return -1 on errors that are fatal for the socket
 * return -2 for those that aren't */
int get_raw_packet(struct dhcp_message *payload, int fd)
{
	int bytes;
	struct udp_dhcp_packet packet;
	uint32_t source, dest;
	uint16_t check;

	memset(&packet, 0, sizeof(struct udp_dhcp_packet));
	bytes = read(fd, &packet, sizeof(struct udp_dhcp_packet));
	if (bytes < 0) {
		BLTS_DEBUG("couldn't read on raw listening socket -- ignoring\n");
		usleep(500000); /* possible down interface, looping condition */
		return -1;
	}

	if (bytes < (int) (sizeof(struct iphdr) + sizeof(struct udphdr))) {
		BLTS_DEBUG("message too short, ignoring\n");
		return -2;
	}

	if (bytes < ntohs(packet.ip.tot_len)) {
		BLTS_DEBUG("Truncated packet\n");
		return -2;
	}

	/* ignore any extra garbage bytes */
	bytes = ntohs(packet.ip.tot_len);

	/* Make sure its the right packet for us, and that it passes sanity checks */
	if (packet.ip.protocol != IPPROTO_UDP || packet.ip.version != IPVERSION ||
	    packet.ip.ihl != sizeof(packet.ip) >> 2 || packet.udp.dest != htons(CLIENT_PORT) ||
	    bytes > (int) sizeof(struct udp_dhcp_packet) ||
	    ntohs(packet.udp.len) != (uint16_t) (bytes - sizeof(packet.ip))) {
		BLTS_DEBUG( "unrelated/bogus packet\n");
	    	return -2;
	}

	/* check IP checksum */
	check = packet.ip.check;
	packet.ip.check = 0;

	if (check != checksum(&(packet.ip), sizeof(packet.ip))) {
		BLTS_DEBUG("bad IP header checksum, ignoring\n");
		return -1;
	}

	/* verify the UDP checksum by replacing the header with a psuedo header */
	source = packet.ip.saddr;
	dest = packet.ip.daddr;
	check = packet.udp.check;
	packet.udp.check = 0;
	memset(&packet.ip, 0, sizeof(packet.ip));

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = source;
	packet.ip.daddr = dest;
	packet.ip.tot_len = packet.udp.len; /* cheat on the psuedo-header */

	if (check && check != checksum(&packet, bytes)) {
		BLTS_DEBUG("packet with bad UDP checksum received, ignoring\n");
		return -2;
	}

	memcpy(payload, &(packet.data), bytes - (sizeof(packet.ip) + sizeof(packet.udp)));

	if (ntohl(payload->cookie) != DHCP_MAGIC) {
		BLTS_DEBUG("received bogus message (bad magic) -- ignoring\n");
		return -2;
	}

	BLTS_DEBUG( "***** Get raw packet OK\n");
	return bytes - (sizeof(packet.ip) + sizeof(packet.udp));
}

/* get an option with bounds checking (warning, not aligned). */
uint8_t *get_option(struct dhcp_message *packet, int code)
{
	int i, length;
	uint8_t *optionptr;
	int over = 0, done = 0, curr = OPTION_FIELD;

	optionptr = packet->options;
	i = 0;
	length = 308;
	while (!done) {
		if (i >= length) {
			BLTS_DEBUG( "bogus packet, option fields too long.\n");
			return NULL;
		}
		if (optionptr[i + OPT_CODE] == code) {
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				BLTS_DEBUG("bogus packet, option fields too long.\n");
				return NULL;
			}
			return optionptr + i + 2;
		}
		switch (optionptr[i + OPT_CODE]) {
		case DHCP_PADDING:
			i++;
			break;
		case DHCP_OPTION_OVER:
			if (i + 1 + optionptr[i + OPT_LEN] >= length) {
				BLTS_DEBUG("bogus packet, option fields too long.\n");
				return NULL;
			}
			over = optionptr[i + 3];
			i += optionptr[OPT_LEN] + 2;
			break;
		case DHCP_END:
			if (curr == OPTION_FIELD && over & FILE_FIELD) {
				optionptr = packet->file;
				i = 0;
				length = 128;
				curr = FILE_FIELD;
			} else if (curr == FILE_FIELD && over & SNAME_FIELD) {
				optionptr = packet->sname;
				i = 0;
				length = 64;
				curr = SNAME_FIELD;
			} else done = 1;
			break;
		default:
			i += optionptr[OPT_LEN + i] + 2;
		}
	}
	return NULL;
}

int send_dhcp_discover(wlan_core_data* data)
{
	int len;
	int retval;
	int max_fd;
	int fd = -1;
	int packet_num = 0;

	long now;
	fd_set rfds;
	unsigned long xid = 0;
	unsigned long timeout = 0;

	char *message;

	struct dhcp_message packet;
	struct timeval tv;

	if(!data || !data->cmd->ifname)
		return -1;

	config.interface = data->cmd->ifname;
	config.ifindex = data->cmd->ifindex;

	if (read_interface(config.interface, &config.ifindex, NULL,
			config.arp) < 0)

	if (!config.clientid)
	{
		config.clientid = malloc(6 + 3);
		config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
		config.clientid[OPT_LEN] = 7;
		config.clientid[OPT_DATA] = 1;
		memcpy(config.clientid + 3, config.arp, 6);
	}

	for (;;)
	{
		tv.tv_sec = timeout - uptime();
		tv.tv_usec = 0;

		if (fd < 0)
		{
			fd = raw_socket(config.ifindex);
			if (fd < 0)
			{
				BLTS_ERROR("FATAL: couldn't listen on socket\n");
				return -1;
			}
		}

		max_fd = udhcp_sp_fd_set(&rfds, fd);

		if (tv.tv_sec > 0)
		{
			BLTS_DEBUG("***** Waiting on select...\n");
			retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		}
		else
			retval = 0; /* If we already timed out, fall through */

		now = uptime();
		if (retval == 0)
		{
			/* timeout dropped to zero */
			if (packet_num < 5)
			{
				if (packet_num == 0)
					xid = random_xid();

				/* send discover packet */
				send_discover(xid, 0);

				timeout = now + ((packet_num == 2) ? 4 : 2);
				packet_num++;
			}
			else
			{
				BLTS_DEBUG("send discover failed\n");
				close(fd);
				fd = -1;
				return -1;
			}
		}
		else if (retval > 0 && FD_ISSET(fd, &rfds))
		{
			BLTS_DEBUG("***** Packet is ready - read it...\n");
			len = get_raw_packet(&packet, fd);

			if (len == -1 && errno != EINTR)
			{
				BLTS_DEBUG("error on read, reopening socket");
				if (fd >= 0)
					close(fd);
				fd = -1;
			}
			if (len < 0)
				continue;

			if (packet.xid != xid)
			{
				BLTS_DEBUG("Ignoring XID %lx (our xid is %lx)",
						(unsigned long) packet.xid, xid);
				continue;
			}
			/* Ignore packets that aren't for us */
			if (memcmp(packet.chaddr, config.arp, 6))
			{
				BLTS_DEBUG("packet does not have our chaddr -- ignoring");
				continue;
			}

			if ((message = (char*)get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL)
			{
				BLTS_DEBUG("couldn't get option from packet -- ignoring");
				continue;
			}

			if (*message == DHCPOFFER)
			{
				if ((get_option(&packet, DHCP_SERVER_ID)))
				{
					BLTS_DEBUG("***** Discover OK\n");
					close(fd);
					return 0;
				}
				else
				{
					BLTS_DEBUG("***** No server ID in message!\n");
					close(fd);
					return -1;
				}
			}
		}
		else if (retval == -1 && errno == EINTR)
		{
			BLTS_ERROR("a signal was caught\n");
		}
		else
		{
			BLTS_ERROR("error on select\n");
		}
	}
	return -1;
}

