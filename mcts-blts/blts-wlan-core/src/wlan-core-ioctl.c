/* wlan-core-ioctl.c -- wlan core ioctls

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <blts_log.h>

#include "wlan-core-ioctl.h"
#include "wlan-core-profiler.h"

#define CALL_IOCTL(a, b)						\
	ioctl_profile(#b);						\
	if(ioctl(a, b) < 0)						\
	{								\
		err = errno;						\
		BLTS_TRACE(#b" failed - %s (%d)\n", strerror(err), err); \
		err = -1;						\
	}

#define CALL_IOCTL2(a, b, c)						\
	ioctl_profile(#b);						\
	if(ioctl(a, b, c) < 0)						\
	{								\
		err = errno;						\
		BLTS_TRACE(#b" failed - %s (%d)\n", strerror(err), err); \
		err = -1;						\
	}

#define CALL_IOCTL3(a, b, c, d)						\
	ioctl_profile(d);						\
	if(ioctl(a, b, c) < 0)						\
	{								\
		err = errno;						\
		BLTS_TRACE(#b" failed - %s (%d)\n", strerror(err), err); \
		err = -1;						\
	}

static void get_ioctl_name(int request, char* name)
{
	switch (request)
	{
/* Wireless Identification */
	case SIOCSIWCOMMIT:   /* 0x8B00-Commit pending changes to driver */
		strcpy(name, "SIOCSIWCOMMIT"); break;
	case SIOCGIWNAME:     /* 0x8B01-get name == wireless protocol */
		strcpy(name, "SIOCGIWNAME"); break;
/* Basic operations */
	case SIOCSIWNWID:     /* 0x8B02-set network id (pre-802.11) */
		strcpy(name, "SIOCSIWNWID"); break;
	case SIOCGIWNWID:     /* 0x8B03-get network id (the cell) */
		strcpy(name, "SIOCGIWNWID"); break;
	case SIOCSIWFREQ:     /* 0x8B04-set channel/frequency (Hz) */
		strcpy(name, "SIOCSIWFREQ"); break;
	case SIOCGIWFREQ:     /* 0x8B05-get channel/frequency (Hz) */
		strcpy(name, "SIOCGIWFREQ"); break;
	case SIOCSIWMODE:     /* 0x8B06-set operation mode */
		strcpy(name, "SIOCSIWMODE"); break;
	case SIOCGIWMODE:     /* 0x8B07-get operation mode */
		strcpy(name, "SIOCGIWMODE"); break;
	case SIOCSIWSENS:     /* 0x8B08-set sensitivity (dBm) */
		strcpy(name, "SIOCSIWSENS"); break;
	case SIOCGIWSENS:     /* 0x8B09-get sensitivity (dBm) */
		strcpy(name, "SIOCGIWSENS"); break;

/* Informative stuff */
	case SIOCSIWRANGE:    /* 0x8B0A-Unused */
		strcpy(name, "SIOCSIWRANGE"); break;
	case SIOCGIWRANGE:    /* 0x8B0B-Get range of parameters */
		strcpy(name, "SIOCGIWRANGE"); break;
	case SIOCSIWPRIV:     /* 0x8B0C-Unused */
		strcpy(name, "SIOCSIWPRIV"); break;
	case SIOCGIWPRIV:     /* 0x8B0D-get private ioctl interface info */
		strcpy(name, "SIOCGIWPRIV"); break;
	case SIOCSIWSTATS:    /* 0x8B0E-Unused */
		strcpy(name, "SIOCSIWSTATS"); break;
	case SIOCGIWSTATS:    /* 0x8B0F-Get /proc/net/wireless stats */
		strcpy(name, "SIOCGIWSTATS"); break;

/* Spy support (statistics per MAC address - used for Mobile IP support) */
	case SIOCSIWSPY:      /* 0x8B10-set spy addresses */
		strcpy(name, "SIOCSIWSPY"); break;
	case SIOCGIWSPY:      /* 0x8B11-get spy info (quality of link) */
		strcpy(name, "SIOCGIWSPY"); break;
	case SIOCSIWTHRSPY:   /* 0x8B12-set spy threshold (spy event) */
		strcpy(name, "SIOCSIWTHRSPY"); break;
	case SIOCGIWTHRSPY:   /* 0x8B13-get spy threshold */
		strcpy(name, "SIOCGIWTHRSPY"); break;

/* Access Point manipulation */
	case SIOCSIWAP:       /* 0x8B14-set access point MAC addresses */
		strcpy(name, "SIOCSIWAP"); break;
	case SIOCGIWAP:       /* 0x8B15-get access point MAC addresses */
		strcpy(name, "SIOCGIWAP"); break;
	case SIOCGIWAPLIST:   /* 0x8B17-Deprecated in favor of scanning */
		strcpy(name, "SIOCGIWAPLIST"); break;
	case SIOCSIWSCAN:     /* 0x8B18-trigger scanning (list cells) */
		strcpy(name, "SIOCSIWSCAN"); break;
	case SIOCGIWSCAN:     /* 0x8B19-get scanning results */
		strcpy(name, "SIOCGIWSCAN"); break;

/* 802.11 specific support */
	case SIOCSIWESSID:    /* 0x8B1A-set ESSID (network name) */
		strcpy(name, "SIOCSIWESSID"); break;
	case SIOCGIWESSID:    /* 0x8B1B-get ESSID */
		strcpy(name, "SIOCGIWESSID"); break;
	case SIOCSIWNICKN:    /* 0x8B1C-set node name/nickname */
		strcpy(name, "SIOCSIWNICKN"); break;
	case SIOCGIWNICKN:    /* 0x8B1D-get node name/nickname */
		strcpy(name, "SIOCGIWNICKN"); break;

/* Other parameters useful in 802.11 and some other devices */
	case SIOCSIWRATE:     /* 0x8B20-set default bit rate (bps) */
		strcpy(name, "SIOCSIWRATE"); break;
	case SIOCGIWRATE:     /* 0x8B21-get default bit rate (bps) */
		strcpy(name, "SIOCGIWRATE"); break;
	case SIOCSIWRTS:      /* 0x8B22-set RTS/CTS threshold (bytes) */
		strcpy(name, "SIOCSIWRTS"); break;
	case SIOCGIWRTS:      /* 0x8B23-get RTS/CTS threshold (bytes) */
		strcpy(name, "SIOCGIWRTS"); break;
	case SIOCSIWFRAG:     /* 0x8B24-set fragmentation thr (bytes) */
		strcpy(name, "SIOCSIWFRAG"); break;
	case SIOCGIWFRAG:     /* 0x8B25-get fragmentation thr (bytes) */
		strcpy(name, "SIOCGIWFRAG"); break;
	case SIOCSIWTXPOW:    /* 0x8B26-set transmit power (dBm) */
		strcpy(name, "SIOCSIWTXPOW"); break;
	case SIOCGIWTXPOW:    /* 0x8B27-get transmit power (dBm) */
		strcpy(name, "SIOCGIWTXPOW"); break;
	case SIOCSIWRETRY:    /* 0x8B28-set retry limits and lifetime */
		strcpy(name, "SIOCSIWRETRY"); break;
	case SIOCGIWRETRY:    /* 0x8B29-get retry limits and lifetime */
		strcpy(name, "SIOCGIWRETRY"); break;

/* Encoding stuff (scrambling, hardware security, WEP...) */
	case SIOCSIWENCODE:   /* 0x8B2A-set encoding token & mode */
		strcpy(name, "SIOCSIWENCODE"); break;
	case SIOCGIWENCODE:   /* 0x8B2B-get encoding token & mode */
		strcpy(name, "SIOCGIWENCODE"); break;

/* Power saving stuff (power management, unicast and multicast) */
	case SIOCSIWPOWER:    /* 0x8B2C-set Power Management settings */
		strcpy(name, "SIOCSIWPOWER"); break;
	case SIOCGIWPOWER:    /* 0x8B2D-get Power Management settings */
		strcpy(name, "SIOCGIWPOWER"); break;
#if 0 //TODO LINUX_VERSION_CODE check + defines to wlan-core-ioctl.h
/* WPA : Generic IEEE 802.11 informatiom element (e.g., for WPA/RSN/WMM) */
	case SIOCSIWGENIE:    /* 0x8B30-set generic IE */
		strcpy(name, "SIOCSIWGENIE"); break;
	case SIOCGIWGENIE:    /* 0x8B31-get generic IE */
		strcpy(name, "SIOCGIWGENIE"); break;

/* WPA : IEEE 802.11 MLME requests */
	case SIOCSIWMLME:     /* 0x8B16-request MLME operation; uses struct iw_mlme */
	strcpy(name, "SIOCSIWMLME"); break;

/* WPA : Authentication mode parameters */
	case SIOCSIWAUTH:     /* 0x8B32-set authentication mode params */
		strcpy(name, "SIOCSIWAUTH"); break;
	case SIOCGIWAUTH:     /* 0x8B33-get authentication mode params */
		strcpy(name, "SIOCGIWAUTH"); break;

/* WPA : Extended version of encoding configuration */
	case SIOCSIWENCODEEXT: /* 0x8B34 set encoding token & mode */
		strcpy(name, "SIOCSIWENCODEEXT"); break;
	case SIOCGIWENCODEEXT: /* 0x8B35 get encoding token & mode */
		strcpy(name, "SIOCSIWENCODEEXT"); break;

/* WPA2 : PMKSA cache management */
	case SIOCSIWPMKSA:    /* 0x8B36-PMKSA cache operation */
	strcpy(name, "SIOCSIWPMKSA"); break;
#endif
	default: BLTS_DEBUG("Unknown request:%X, maybe new ioctl?\n", request); break;
	}
}

int IOCTL_CALL(wlan_core_data* data, int request, struct iwreq* wrqp)
{
	int err = 0;
	char CMD_NAME [20] = "";

	if(!data || !wrqp)
		return -EINVAL;

	get_ioctl_name(request, (char*) &CMD_NAME);

	if(!strlen(CMD_NAME))
		return -EINVAL;

	if(data->cmd->ifname && !wrqp->ifr_name)
	{
		BLTS_DEBUG("IOCTL_CALL -> use ifname:%s\n", data->cmd->ifname);
		strncpy(wrqp->ifr_name, data->cmd->ifname, IFNAMSIZ);
		wrqp->ifr_name[IFNAMSIZ-1] = 0;
	}

	CALL_IOCTL3(data->ioctl_sock, request, wrqp, CMD_NAME);

	return err;
}

