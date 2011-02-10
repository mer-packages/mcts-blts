/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* transfer-cases.c -- Data transfer cases

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
#include <blts_log.h>

#include "../common/hostdrv_common.h"
#include "transfer-cases.h"
#include "usb-general.h"
#include "usb-util.h"

#define EPNODE_FMT "test_dev_node%d"
static char node_names[USBDRV_MAX_ENDPOINTS][128];

#define BUF_SIZE (4096)
#define MAX_XFER_SIZE (1024 * 1024 * 1024)
#define MAX_TEST_T (10.0)
#define MAX_PACKET (1024)


struct hostdrv_current_config config;
pthread_t logthread_id;

struct usb_case_state {
	char* host_driver;
	char* host_driver_path;
	int data_transfer_timeout;

	my_usb_data *usb_data;

	int result;
};


struct rw_thread_data {
	pthread_t id;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	struct timespec to;
	int ret;
	char *node_name;
	unsigned transfer_size;
};

struct rw_thread_data thread_data[USBDRV_MAX_ENDPOINTS];

static void usb_state_finalize(struct usb_case_state *state)
{
	if (!state)
		return;

	if (state->host_driver)
		free(state->host_driver);

	if (state->host_driver_path)
		free(state->host_driver_path);

	free(state);
}

void do_cleanup()
{
	int t;
	sleep(1);
	if (logthread_id)
		pthread_cancel(logthread_id);

	for (t = 0; t < USBDRV_MAX_ENDPOINTS; t++)
		unlink(node_names[t]);

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
		BLTS_LOGGED_PERROR("log output open");
		return NULL;
	}

	fdp.fd = open("/sys/module/blts_usb_host/logging/log", O_RDWR);
	if (fdp.fd < 0) {
		BLTS_LOGGED_PERROR("sysfs log open");
		close(fdw);
		return NULL;
	}

	fdp.events = POLLPRI;
	fdp.revents = 0;

	while (1) {
		ret = poll(&fdp, 1, 10000);
		if (ret < 0) {
			BLTS_LOGGED_PERROR("log poll");
			break;
		} else if (ret > 0) {
			memset(buf, 0, sizeof(buf));
			lseek(fdp.fd, SEEK_SET, 0);
			ret = read(fdp.fd, buf, 4096);
			if (ret < 0) {
				BLTS_LOGGED_PERROR("log read");
				break;
			}
			ret = write(fdw, buf, ret);
			if (ret < 0) {
				BLTS_LOGGED_PERROR("log write");
				break;
			}
		}
	}

	close(fdw);
	close(fdp.fd);
	return NULL;
}

static struct usb_case_state *usb_state_init(my_usb_data *data, dev_t dev)
{
	int ret, t;
	struct usb_case_state *state;
	state = malloc(sizeof *state);
	if (!state) {
		BLTS_DEBUG("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->usb_data = data;

	do_cleanup();

	for (t = 0; t < USBDRV_MAX_ENDPOINTS; t++) {
		sprintf(node_names[t], EPNODE_FMT, t);
		ret = mknodat(AT_FDCWD, node_names[t], S_IFCHR | 0600,
                              makedev(major(dev), t));
		if (ret)
			BLTS_DEBUG("mknodat (%s) failed.\n", node_names[t]);
	}


	ret = pthread_create(&logthread_id, NULL, log_thread_function, NULL);
	if (ret)
		BLTS_DEBUG("Failed to create log reader thread (%d)!\n", ret);

	return state;
}

dev_t find_dev(void* user_ptr)
{
	my_usb_data *data = ((my_usb_data *) user_ptr);
	unsigned dev = 0;
	char *line = 0;
	size_t line_sz = 0;
	ssize_t read_len = 0;

	FILE *f = fopen("/proc/devices", "r");

	if (!f) {
		BLTS_DEBUG("Can't open devices.\n");
		goto done;
	}
	while ((read_len = getline(&line, &line_sz, f)) != -1)
	{
		if (strstr(line, data->host_driver))
		{
			if (sscanf(line, "%u", &dev) != 1)
				BLTS_DEBUG("Can't get devnum.\n");
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

int buf_ch_size(int fd, unsigned ep, unsigned newsize)
{
	if (ioctl(fd, HOSTDRV_IOCGCONF, &config) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (get conf)");
		return -errno;
	}
	config.transfer_size[ep] = newsize;
	if (ioctl(fd, HOSTDRV_IOCSCONF, &config) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (set conf)");
		return -errno;
	}
	return 0;
}


static int test_read_real(char *node_name, unsigned size)
{
	int fdr, ret, left;
	unsigned i;
	char *buf;
	struct timespec t_s, t_e;
	double t_tot = 0.0;
	long d_tot;

	buf = malloc(size);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, size);

	fdr = open(node_name, O_RDONLY);
	if (fdr < 0) {
		BLTS_LOGGED_PERROR("open");
		free(buf);
		return -errno;
	}

	left = MAX_XFER_SIZE;
	d_tot = 0;
	clock_gettime(CLOCK_REALTIME, &t_s);

	while (left)
	{
		ret = read(fdr, buf, size);
		if (ret < 0)
		{
			BLTS_LOGGED_PERROR("read");
			break;
		}
		for (i = 0; i < size; i++) {
			if (buf[i] == i % 63)
				continue;
			BLTS_DEBUG("Data consistency error\n \tData[%i] = '%02X'\n \tShould be '%02X'\n", i, buf[i], i%63);
			BLTS_DEBUG("Current transfer of data is therefore corrupted, data frame size %i bytes\n", size);
			ret = -1;
			goto cleanup;
		}
		d_tot += ret;
		left -= ret;
		if (left < (int)size)
			size = left;
		clock_gettime(CLOCK_REALTIME, &t_e);
		t_tot = t_e.tv_sec - t_s.tv_sec + 1E-9 * (t_e.tv_nsec - t_s.tv_nsec);
		if (t_tot > MAX_TEST_T)
			break;
	}
	BLTS_DEBUG("%s: read %ld B in %.3lf s (%.3lf bytes/sec)\n", node_name, d_tot, t_tot, d_tot/t_tot);
	if (!ret || d_tot == 0)
	{
		BLTS_DEBUG("No data transfer detected, transfer failed.\n");
		ret = -EIO;
	}

cleanup:
	free(buf);
	close(fdr);

	if(errno && ret)
		return -errno;

	return ret;
}


static int test_write_real(char *node_name, unsigned size)
{
	int fdw, ret, left;
	unsigned i;
	char *buf;
	struct timespec t_s, t_e;
	double t_tot = 0.0;
	long d_tot;


	buf = malloc(size);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, size);
	for (i = 0; i < size; i++)
                buf[i] = (char) (i % 63);

	fdw = open(node_name, O_WRONLY);
	if (fdw < 0) {
		BLTS_LOGGED_PERROR("open test_write()");
		free(buf);
		return -errno;
	}

	left = MAX_XFER_SIZE;
	d_tot = 0;
	clock_gettime(CLOCK_REALTIME, &t_s);
	while (left)
	{
		ret = write(fdw, buf, size);
		if (ret < 0) {
			if (errno == EOVERFLOW) {
				BLTS_DEBUG("Write: Overflow.\n");
                                //Try to continue
			} else {
				BLTS_LOGGED_PERROR("write");
				break;
			}
		}
		d_tot += ret;
		left -= ret;
		if (left < (int)size)
			size = left;
		clock_gettime(CLOCK_REALTIME, &t_e);
		t_tot = t_e.tv_sec - t_s.tv_sec + 1E-9 * (t_e.tv_nsec - t_s.tv_nsec);
		if (t_tot > MAX_TEST_T)
			break;
	}
	BLTS_DEBUG("%s: wrote %ld B in %.3lf s (%.3lf bytes/sec)\n", node_name, d_tot, t_tot, d_tot/t_tot);
	if (!ret || d_tot == 0)
	{
		BLTS_DEBUG("No data transfer detected, transfer failed.\n");
		ret = -EIO;
	}
	free(buf);
	close(fdw);

	if(errno && ret)
		return -errno;

	return ret;
}

static void *read_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	data->ret = test_read_real(data->node_name, data->transfer_size);

	pthread_mutex_lock(&data->mutex);
	pthread_cond_signal(&data->cond);
	pthread_mutex_unlock(&data->mutex);

	return NULL;
}

static void *write_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	data->ret = test_write_real(data->node_name, data->transfer_size);

	pthread_mutex_lock(&data->mutex);
	pthread_cond_signal(&data->cond);
	pthread_mutex_unlock(&data->mutex);

	return NULL;
}


void print_ep_type(int type)
{
	switch (type)
	{
        case USBDRV_EP_TYPE_BULK:
                BLTS_DEBUG("\tBulk endpoint\n");
                break;
        case USBDRV_EP_TYPE_CONTROL:
                BLTS_DEBUG("\tControl endpoint\n");
                break;
        case USBDRV_EP_TYPE_INT:
                BLTS_DEBUG("\tInterrupt endpoint\n");
                break;
        case USBDRV_EP_TYPE_ISOC:
                BLTS_DEBUG("\tIsochronous endpoint\n");
                break;
        default:
                BLTS_DEBUG("\tUnknown endpoint type\n");
                break;
	}
}

int test_read(unsigned buf_new_size)
{
	int fd, ret, i, nodes_found=0;
	char *buf;
	struct hostdrv_current_config conf;

	unsigned size = buf_new_size? buf_new_size:BUF_SIZE;
	buf = malloc(size);
	memset(buf, 0, size);

	fd = open(node_names[0], O_RDONLY);
	if (fd < 0)
	{
		BLTS_LOGGED_PERROR("error on open");
		return -errno;
	}

	if (buf_new_size) {
		if (buf_ch_size(fd, 1, buf_new_size) < 0)
		{
			close(fd);
			return -1;
		}
        }

	if (ioctl(fd, HOSTDRV_IOCGCONF, &conf) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (get conf)");
		return -errno;
	}

	for (i = 1; i < USBDRV_MAX_ENDPOINTS; i++)
	{
		if (conf.endpoint_type[i] != USBDRV_EP_TYPE_NONE)
		{
			thread_data[i].transfer_size =
                        conf.transfer_size[i];
			thread_data[i].node_name = node_names[i];

			if (conf.endpoint_state[i] == USBDRV_EP_IN)
			{
				BLTS_DEBUG("Starting read thread for ep %d...\n", i);
				print_ep_type(conf.endpoint_type[i]);
				BLTS_DEBUG("\tEndpoint transfer size: %i\n", conf.transfer_size[i]);
				nodes_found++;
				pthread_mutex_init(&thread_data[i].mutex, NULL);
				pthread_cond_init(&thread_data[i].cond, NULL);
				clock_gettime(CLOCK_REALTIME, &thread_data[i].to);
				thread_data[i].to.tv_sec += MAX_TEST_T + 5;
				pthread_mutex_lock(&thread_data[i].mutex);
				ret = pthread_create(&thread_data[i].id, NULL,
                                                     read_thread_function, &thread_data[i]);
				if (ret) {
					BLTS_DEBUG("Failed to create read thread "\
                                                   "(%d)!\n", ret);
					goto cleanup;
				}
			} else {
				// just read test
				BLTS_DEBUG("NOT Starting write thread for ep %d...\n", i);
                        }
                }
        }

	for (i = 0; i < USBDRV_MAX_ENDPOINTS; i++) {
		if (thread_data[i].id) {
			int e;

			e = pthread_cond_timedwait(&thread_data[i].cond,
				&thread_data[i].mutex, &thread_data[i].to);
			pthread_mutex_unlock(&thread_data[i].mutex);
			if (e == ETIMEDOUT) {
				BLTS_DEBUG("Thread %d timed out!\n", i);
				ret = -1;
				pthread_cancel(thread_data[i].id);
			} else {
				pthread_join(thread_data[i].id, NULL);
			}
			thread_data[i].id = 0;

			if (thread_data[i].ret && ret >= 0)
				ret = thread_data[i].ret;
		}
        }

cleanup:
	close(fd);
	for (i = 0; i < USBDRV_MAX_ENDPOINTS; i++) {
		if (thread_data[i].id)
			pthread_cancel(thread_data[i].id);
		pthread_cond_destroy(&thread_data[i].cond);
		pthread_mutex_destroy(&thread_data[i].mutex);
        }
	if(nodes_found == 0)
	{
		BLTS_DEBUG("No endpoints found for read testing\n");
		ret = -1;
	}
	return ret;
}

int test_write(unsigned buf_new_size)
{
	int fd, ret, i, nodes_found=0;
	char *buf;
	struct hostdrv_current_config conf;

	unsigned size = buf_new_size? buf_new_size:BUF_SIZE;
	buf = malloc(size);
	memset(buf, 0, size);

	fd = open(node_names[0], O_RDONLY);
	if (fd < 0)
	{
		BLTS_LOGGED_PERROR("open");
		return -errno;
	}

	if (buf_new_size)
		if (buf_ch_size(fd, 1, buf_new_size) < 0)
		{
			close(fd);
			return -1;
		}

	if (ioctl(fd, HOSTDRV_IOCGCONF, &conf) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (get conf)");
		return -errno;
	}

	for (i = 1; i < USBDRV_MAX_ENDPOINTS; i++)
	{
		if (conf.endpoint_type[i] != USBDRV_EP_TYPE_NONE)
		{
			thread_data[i].transfer_size =
                        conf.transfer_size[i];
			thread_data[i].node_name = node_names[i];

			if (conf.endpoint_state[i] == USBDRV_EP_OUT)
			{
				BLTS_DEBUG("Starting write thread for ep %d...\n", i);
				print_ep_type(conf.endpoint_type[i]);
				BLTS_DEBUG("\tEndpoint transfer size: %i\n", conf.transfer_size[i]);
				nodes_found++;
				pthread_mutex_init(&thread_data[i].mutex, NULL);
				pthread_cond_init(&thread_data[i].cond, NULL);
				clock_gettime(CLOCK_REALTIME, &thread_data[i].to);
				thread_data[i].to.tv_sec += MAX_TEST_T + 5;
				pthread_mutex_lock(&thread_data[i].mutex);
				ret = pthread_create(&thread_data[i].id, NULL,
                                                     write_thread_function, &thread_data[i]);
				if (ret) {
					BLTS_DEBUG("failed to create write thread "\
                                                   "(%d)!\n", ret);
					goto cleanup;
                                } else {
                                        // just write test
                                        BLTS_DEBUG("NOT Starting read thread for ep %d...\n", i);
				}
			}
		}
	}

	for (i = 0; i < USBDRV_MAX_ENDPOINTS; i++) {
		if (thread_data[i].id) {
			int e = 0;

			e = pthread_cond_timedwait(&thread_data[i].cond,
				&thread_data[i].mutex, &thread_data[i].to);
			pthread_mutex_unlock(&thread_data[i].mutex);
			if (e == ETIMEDOUT) {
				BLTS_DEBUG("Thread %d timed out!\n", i);
				ret = -1;
				pthread_cancel(thread_data[i].id);
			} else {
				pthread_join(thread_data[i].id, NULL);
			}
			thread_data[i].id = 0;

			if (thread_data[i].ret && ret >= 0)
				ret = thread_data[i].ret;
		}
        }

cleanup:
	close(fd);
	for (i = 0; i < USBDRV_MAX_ENDPOINTS; i++) {
		if (thread_data[i].id)
			pthread_cancel(thread_data[i].id);
		pthread_cond_destroy(&thread_data[i].cond);
		pthread_mutex_destroy(&thread_data[i].mutex);
	}
	if(nodes_found == 0)
	{
		BLTS_DEBUG("No endpoints found for write testing\n");
		ret = -1;
	}

	return ret;
}

/* Test cases ------------> */

/* Transfer data using bulk endpoints */
int blts_data_read(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret = -1;
	struct usb_case_state *test = NULL;
	my_usb_data *data = ((my_usb_data *) user_ptr);

	if (!data)
		return -EINVAL;

	dev_t dev = find_dev(data);
	BLTS_DEBUG("Driver:: dev = %u:%u\n", major(dev), minor(dev));

	if (!major(dev))
	{
		BLTS_DEBUG("Bad major, aborting test.\n");
		goto done;
	}

	test = usb_state_init(data, dev);

	if (!test)
		return ret;

	BLTS_DEBUG("Running... ");
	ret = test_read(data->endpoint_transfer_size);

done:
	do_cleanup();
	usb_state_finalize(test);

	return ret;
}

int blts_data_write(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret = -1;
	struct usb_case_state *test = NULL;
	my_usb_data *data = ((my_usb_data *) user_ptr);

	if (!data)
		return -EINVAL;

	dev_t dev = find_dev(data);
	BLTS_DEBUG("Driver:: dev = %u:%u\n", major(dev), minor(dev));

	if (!major(dev))
	{
		BLTS_DEBUG("Bad major, aborting test.\n");
		goto done;
	}

	test = usb_state_init(data, dev);

	if (!test)
		return ret;

	BLTS_DEBUG("Running... \n");
	ret = test_write(data->endpoint_transfer_size);

done:
	do_cleanup();
	usb_state_finalize(test);

	return ret;
}

int blts_setup_host_driver(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret = -1;
	char *command;
	my_usb_data *data = ((my_usb_data *) user_ptr);

	if (!data)
		return -EINVAL;

	BLTS_DEBUG("Setup host driver: %s%s.ko\n", data->host_driver_path, data->host_driver);

	ret = asprintf(&command, "insmod %s%s.ko", data->host_driver_path, data->host_driver);
	if (ret<0)
		return -ENOMEM;
	ret = (WEXITSTATUS(system(command)));
	free (command);

	return ret;
}

/* Convert generated parameters to test case format. */
void *data_transfer_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *usb_host_driver = 0;
	char *usb_host_driver_path = 0;
	my_usb_data *data = ((my_usb_data *) user_ptr);
	if (!data)
		return 0;

	usb_host_driver = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	usb_host_driver_path = strdup(blts_config_boxed_value_get_string(args));

	/* These are already non-zero, if set on command line */

	if (data->host_driver)
		free(usb_host_driver);
	else
		data->host_driver = usb_host_driver;

	if (data->host_driver_path)
		free(usb_host_driver_path);
	else
		data->host_driver_path = usb_host_driver_path;

	return data;
}
