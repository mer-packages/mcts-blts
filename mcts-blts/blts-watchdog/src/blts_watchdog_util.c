/* blts_watchdog_util.c -- Various helper functions

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
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <blts_dep_check.h>
#include "blts_watchdog_util.h"

#define DEP_RULES "/usr/lib/tests/blts-watchdog-tests/blts-watchdog-req_files.cfg"

static int wdt_open()
{
	int fd;

	fd = open("/dev/watchdog", O_WRONLY);
	if(fd == -1)
	{
		if(errno == EBUSY)
		{
			BLTS_DEBUG("\n/dev/watchdog seems to exist, but is in use by some other process.\n"
			"Close that process and run this test again.\n"
			"Use 'lsof /dev/watchdog' to find out who is using watchdog.\n\n");
		}
		BLTS_LOGGED_PERROR("watchdog open");
		return -1;
	}

	return fd;
}

static void wdt_close(int fd)
{
	close(fd);
}

static int wdt_get_timeout(int fd)
{
	int timeout = 0;

	if(ioctl(fd, WDIOC_GETTIMEOUT, &timeout))
	{
		BLTS_LOGGED_PERROR("WDIOC_GETTIMEOUT");
		return -1;
	}

	BLTS_DEBUG("Current watchdog timeout: %d seconds\n", timeout);
	return timeout;
}

static int wdt_set_timeout(int fd, int timeout)
{
	if(ioctl(fd, WDIOC_SETTIMEOUT, &timeout))
	{
		BLTS_LOGGED_PERROR("WDIOC_SETTIMEOUT");
		return -1;
	}

	BLTS_DEBUG("Current watchdog timeout: %d seconds\n", timeout);
	return 0;
}

static int wdt_get_info(int fd)
{
	struct watchdog_info wdt_info;
	int boot_status;

	if(ioctl(fd, WDIOC_GETSUPPORT, &wdt_info))
	{
		BLTS_LOGGED_PERROR("WDIOC_GETSUPPORT");
		return -1;
	}

	BLTS_DEBUG("Watchdog module: %s\n", (char *)wdt_info.identity);
	BLTS_DEBUG("Firmware revision: %d\n", wdt_info.firmware_version);

	BLTS_DEBUG("Supported options:\n");
	if(0 == wdt_info.options)
		BLTS_DEBUG("\tnone\n");
	if(WDIOF_OVERHEAT & wdt_info.options)
		BLTS_DEBUG("\tOVERHEAT\n");
	if(WDIOF_FANFAULT & wdt_info.options)
		BLTS_DEBUG("\tFANFAULT\n");
	if(WDIOF_EXTERN1 & wdt_info.options)
		BLTS_DEBUG("\tEXTERN1\n");
	if(WDIOF_EXTERN2 & wdt_info.options)
		BLTS_DEBUG("\tEXTERN2\n");
	if(WDIOF_POWERUNDER & wdt_info.options)
		BLTS_DEBUG("\tPOWERUNDER \n");
	if(WDIOF_CARDRESET & wdt_info.options)
		BLTS_DEBUG("\tCARDRESET\n");
	if(WDIOF_POWEROVER & wdt_info.options)
		BLTS_DEBUG("\tPOWEROVER\n");
	if(WDIOF_SETTIMEOUT & wdt_info.options)
		BLTS_DEBUG("\tSETTIMEOUT\n");
	if(WDIOF_KEEPALIVEPING & wdt_info.options)
		BLTS_DEBUG("\tKEEPALIVEPING\n");

	if(ioctl(fd, WDIOC_GETBOOTSTATUS, &boot_status))
	{
		BLTS_LOGGED_PERROR("WDIOC_GETBOOTSTATUS");
		return -1;
	}

	BLTS_DEBUG("Last reboot was %sinitiated by watchdog timer.\n",
		(boot_status & WDIOF_CARDRESET) ? "" : "not ");

	return 0;
}

static int wdt_keepalive(int fd)
{
	if(ioctl(fd, WDIOC_KEEPALIVE, 0))
	{
		BLTS_LOGGED_PERROR("WDIOC_KEEPALIVE");
		return -1;
	}

	return 0;
}

/* Simple open/get some info/close */
int wdt_open_close()
{
	int fd = 0;
	int ret = 0;

	BLTS_DEBUG("Opening /dev/watchdog...\n");
	if((fd = wdt_open()) < 0)
	{
		ret = -1;
		goto cleanup;
	}

	if(wdt_get_info(fd))
	{
		ret = -1;
		goto cleanup;
	}

cleanup:
	wdt_close(fd);

	return ret;
}

/* Set keepalive timeout and send keepalive messages to watchdog */
int wdt_send_keepalive(double execution_time)
{
	int fd = 0;
	int ret = 0;
	int timeout;

	BLTS_DEBUG("Opening /dev/watchdog...\n");
	if((fd = wdt_open()) < 0)
	{
		ret = -1;
		goto cleanup;
	}

	if((timeout = wdt_get_timeout(fd)) < 0)
	{
		BLTS_ERROR("Failed to get current timeout value\n");
		ret = -1;
		goto cleanup;
	}

	/* Set timeout to 10 seconds */
	timeout = 10;
	if(wdt_set_timeout(fd, timeout))
	{
		BLTS_ERROR("Failed to set timeout value\n");
		ret = -1;
		goto cleanup;
	}

	timing_start();
	while(timing_elapsed() < execution_time)
	{
		if(wdt_keepalive(fd))
		{
			ret = -1;
			BLTS_ERROR("Failed to send keepalive msg\n");
			break;
		}
		sleep(1);
	}
	timing_stop();

cleanup:
	wdt_close(fd);

	return ret;
}

int wdt_dep_check()
{
	BLTS_DEBUG("Checking required components...\n");

	return depcheck(DEP_RULES,1);
}

