/*
 * test_hostdrv.c -- trivial test for USB host passthrough driver
 *
 * Copyright (C) 2010 by Nokia Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#define _GNU_SOURCE
#define _ATFILE_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include <poll.h>

#include "hostdrv_common.h"

#define TESTF0 "test_dev_node0"
#define TESTF1 "test_dev_node1"
#define TESTF2 "test_dev_node2"

#define BUF_SIZE (4096)
#define MAX_XFER_SIZE (1024 * 1024 * 1024L)
#define MAX_TEST_T (10.0)
#define MAX_PACKET (1024)

struct hostdrv_current_config config;
pthread_t logthread_id;

dev_t find_dev()
{
	unsigned dev = 0;
	char *line = 0;
	size_t line_sz = 0;
	ssize_t read_len = 0;

	FILE *f = fopen("/proc/devices", "r");

	if (!f) {
		printf("can't open devices\n");
		goto done;
	}
	while ((read_len = getline(&line, &line_sz, f)) != -1) {
		if (strstr(line, "blts_usb_host")) {
			if (sscanf(line, "%u", &dev) != 1)
				printf("can't get devnum\n");
			break;
		}
	}

done:
	if (line)
		free(line);
	if (f)
		fclose(f);
	return makedev(dev,0);
}

int test_openclose()
{
	int fd;
	fd = open(TESTF0, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -errno;
	}
	close(fd);
	fd = open(TESTF1, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -errno;
	}
	close(fd);
	fd = open(TESTF2, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -errno;
	}
	errno = 0;
	close(fd);
	return -errno;
}

int buf_ch_size(int fd, unsigned ep, unsigned newsize)
{
	if (ioctl(fd, HOSTDRV_IOCGCONF, &config) < 0) {
		perror("ioctl (get conf)");
		return -errno;
	}
	config.transfer_size[ep] = newsize;
	if (ioctl(fd, HOSTDRV_IOCSCONF, &config) < 0) {
		perror("ioctl (set conf)");
		return -errno;
	}
	return 0;
}

int test_read(unsigned buf_new_size)
{
	int fd, ret;
	long left;
	char *buf;
	struct timespec t_s, t_e;
	double t_tot = 0.0;
	long d_tot;
	unsigned size = buf_new_size? buf_new_size:BUF_SIZE;
	buf = malloc(size);
	memset(buf, 0, size);
	fd = open(TESTF1, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (buf_new_size)
		if (buf_ch_size(fd, 1, buf_new_size) < 0) {
			close(fd);
			return -1;
		}

	left = MAX_XFER_SIZE;
	d_tot = 0;
	clock_gettime(CLOCK_REALTIME, &t_s);
	while (left) {
		ret = read(fd, buf, size);
		if (ret < 0) {
			perror("read");
			break;
		}
		d_tot += ret;
		left -= ret;
		if (left < size)
			size = left;
		clock_gettime(CLOCK_REALTIME, &t_e);
		t_tot = t_e.tv_sec - t_s.tv_sec + 1E-9 * (t_e.tv_nsec - t_s.tv_nsec);
		if (t_tot > MAX_TEST_T)
			break;
	}
	printf("read %ld B in %.3lf s (%.3lf bytes/sec)\n", d_tot, t_tot, d_tot/t_tot);
	if (!ret && d_tot == 0) {
		printf("No data transfer -> fail\n");
		ret = -EIO;
	}
	close(fd);
	free(buf);
	return ret>=0?0:-errno;
}

int test_write(unsigned buf_new_size)
{
	int fd, ret;
	long left;
	char *buf;
	struct timespec t_s, t_e;
	double t_tot = 0.0;
	long d_tot;
	unsigned size = buf_new_size? buf_new_size:BUF_SIZE;
	buf = malloc(size);
	memset(buf, 0, size);
	/* NB: if tested with g_zero, use either all-zeros or mod63, depending
	   on module param */

	fd = open(TESTF2, O_WRONLY);
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (buf_new_size)
		if (buf_ch_size(fd, 2, buf_new_size) < 0) {
			close(fd);
			return -1;
		}

	left = MAX_XFER_SIZE;
	d_tot = 0;
	clock_gettime(CLOCK_REALTIME, &t_s);
	while (left) {
		ret = write(fd, buf, size);
		if (ret < 0) {
			perror("write");
			break;
		}
		d_tot += ret;
		left -= ret;
		if (left < size)
			size = left;
		clock_gettime(CLOCK_REALTIME, &t_e);
		t_tot = t_e.tv_sec - t_s.tv_sec + 1E-9 * (t_e.tv_nsec - t_s.tv_nsec);
		if (t_tot > MAX_TEST_T)
			break;
	}
	printf("wrote %ld B in %.3lf s (%.3lf bytes/sec)\n", d_tot, t_tot, d_tot/t_tot);
	if (!ret && d_tot == 0) {
		printf("No data transfer -> fail\n");
		ret = -EIO;
	}
	close(fd);
	free(buf);
	return ret>=0?0:-errno;
}

void print_ep_type(unsigned t)
{
	switch (t) {
	case USBDRV_EP_TYPE_NONE:
		printf("<nothing>");
		break;
	case USBDRV_EP_TYPE_CONTROL:
		printf("CONTROL");
		break;
	case USBDRV_EP_TYPE_BULK:
		printf("BULK");
		break;
	case USBDRV_EP_TYPE_ISOC:
		printf("ISOC");
		break;
	case USBDRV_EP_TYPE_INT:
		printf("INT");
		break;
	default:
		printf("!!UNKNOWN!!");
	}
}

void print_ep_state(unsigned s)
{
	if (s & USBDRV_EP_IN)
		printf("IN");
	if (s & USBDRV_EP_OUT)
		printf("OUT");
	if (!s)
		printf("<none>");
}

int test_ioctl()
{
	int fd, i;

	memset(&config, 0, sizeof (struct hostdrv_current_config));

	fd = open(TESTF0, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (ioctl(fd, HOSTDRV_IOCGCONF, &config) < 0) {
		perror("ioctl (get conf)");
		return -errno;
	}

	printf("configuration:\n");
	for (i = 0; i < N_HOSTDRV_DEVICES; ++i) {
		printf("\t(");
		print_ep_type(config.endpoint_type[i]);
		printf(":");
		print_ep_state(config.endpoint_state[i]);
		printf(":%u",config.transfer_size[i]);
		printf(")\n");
	}

	for (i = 0; i < N_HOSTDRV_DEVICES; ++i)
		config.transfer_size[i] = 512;

	if (ioctl(fd, HOSTDRV_IOCSCONF, &config) < 0) {
		perror("ioctl (set conf)");
		return -errno;
	}

	if (ioctl(fd, HOSTDRV_IOCGCONF, &config) < 0) {
		perror("ioctl (get conf)");
		return -errno;
	}

	printf("conf (read 2): ");
	for (i = 0; i < N_HOSTDRV_DEVICES; ++i)
		printf("\t(%u:%u:%u)\n",
			config.endpoint_type[i],
			config.endpoint_state[i],
			config.transfer_size[i]);

	close(fd);
	return 0;
}

static void *log_thread_function(void *ptr)
{
	int ret;
	char buf[4096];
	struct pollfd fdp;
	int fdw;
	ptr = ptr; /* unused */
	fdw = open("hostdrv_log.txt", O_WRONLY|O_CREAT|O_APPEND, 00644);
	if (fdw < 0) {
		perror("log output open");
		return NULL;
	}

	fdp.fd = open("/sys/module/blts_usb_host/logging/log", O_RDWR);
	if (fdp.fd < 0) {
		perror("sysfs log open");
		close(fdw);
		return NULL;
	}

	fdp.events = POLLPRI;
	fdp.revents = 0;

	while (1) {
		ret = poll(&fdp, 1, 10000);
		if (ret < 0) {
			perror("log poll");
			break;
		} else if (ret > 0) {
			memset(buf, 0, sizeof(buf));
			lseek(fdp.fd, SEEK_SET, 0);
			ret = read(fdp.fd, buf, 4096);
			if (ret < 0) {
				perror("log read");
				break;
			}
			ret = write(fdw, buf, ret);
			if (ret < 0) {
				perror("log write");
				break;
			}
			/* printf(buf); */
		}
	}

	close(fdw);
	close(fdp.fd);
	return NULL;
}

void do_cleanup()
{
	sleep(1);
	if (logthread_id)
		pthread_cancel(logthread_id);
	unlink(TESTF0);
	unlink(TESTF1);
	unlink(TESTF2);
}

void do_setup(dev_t dev)
{
	int ret;
	do_cleanup();
	ret = mknodat(AT_FDCWD, TESTF0, S_IFCHR | 0600, dev);
	if (ret)
		printf("mknodat (%s) failed\n",TESTF0);
	ret = mknodat(AT_FDCWD, TESTF1, S_IFCHR | 0600, makedev(major(dev),1));
	if (ret)
		printf("mknodat (%s) failed\n",TESTF1);
	ret = mknodat(AT_FDCWD, TESTF2, S_IFCHR | 0600, makedev(major(dev),2));
	if (ret)
		printf("mknodat (%s) failed\n",TESTF1);
	ret = pthread_create(&logthread_id, NULL, log_thread_function, NULL);
	if (ret)
		printf("failed to create log reader thread (%d)!\n", ret);
}

int main()
{
	int ret = -1;
	unsigned size;
	dev_t dev = find_dev();
	printf("hostdrv: dev = %u:%u\n", major(dev), minor(dev));

	if (!major(dev)) {
		printf("bad major, aborting test\n");
		goto done;
	}

	do_setup(dev);
	printf("openclose test...");
	ret = test_openclose();
	printf("%s\n", ret?"failed":"passed");
	printf("ioctl test...");
	ret = test_ioctl();
	printf("%s\n", ret?"failed":"passed");
	printf("read test...");
	ret = test_read(0);
	printf("%s\n", ret?"failed":"passed");
	printf("write test...");
	ret = test_write(0);
	printf("%s\n", ret?"failed":"passed");
	for (size = 1024; size <= 4096<<2; size = size << 1) {
		printf("read test (%uk buf)...", size >> 10);
		ret = test_read(size);
		printf("%s\n", ret?"failed":"passed");
		printf("write test (%uk buf)...", size >> 10);
		ret = test_write(size);
		printf("%s\n", ret?"failed":"passed");
	}

done:
	do_cleanup();
	return ret;
}
