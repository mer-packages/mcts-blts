/* peripheral-control.c -- Peripheral driver controller

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

#define BUF_SIZE (512)
#define EPNODE_FMT "gadget_dev_node%d"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <poll.h>
#include <blts_log.h>

#include "../common/gadgetdrv_common.h"
#include "peripheral-control.h"
#include "usb-general.h"
#include "usb-util.h"

#define EPNODE_FMT "gadget_dev_node%d"
static char node_names[N_GADGETDRV_DEVICES][128];

static pthread_t logthread_id;

struct rw_thread_data {
	pthread_t id;
	int ret;
	char *node_name;
	unsigned transfer_size;
};

struct rw_thread_data thread_data[N_GADGETDRV_DEVICES];

struct usb_case_state {
	char* peripheral_driver;
	char* peripheral_driver_path;
	int data_transfer_timeout;

	int endpoint_count;
	char* endpoint_type;
	int endpoint_max_packet_size;
	int endpoint_interval;
	int endpoint_transfer_size;


	my_usb_data *usb_data;

	int result;
};

static void usb_state_finalize(struct usb_case_state *state)
{
	if (!state)
		return;

	if (state->peripheral_driver)
		free(state->peripheral_driver);

	if (state->peripheral_driver_path)
		free(state->peripheral_driver_path);

	free(state);
}

void do_gadget_cleanup()
{
	int t;

	sleep(1);
	if (logthread_id)
		pthread_cancel(logthread_id);

	for (t = 0; t < N_GADGETDRV_DEVICES; t++)
		unlink(node_names[t]);
}

static void *log_thread_function(void *ptr)
{
	int ret;
	char buf[4096];
	struct pollfd fdp;
	int fdw;
	ptr = ptr; /* unused */
	fdw = open("gadgetdrv_log.txt", O_WRONLY|O_CREAT|O_APPEND, 00644);
	if (fdw < 0) {
		BLTS_LOGGED_PERROR("log output open");
		return NULL;
	}

	fdp.fd = open("/sys/module/blts_gadget/logging/log", O_RDWR);
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
			/* printf(buf); */
		}
	}

	close(fdw);
	close(fdp.fd);
	return NULL;
}

static struct usb_case_state *usb_state_init(my_usb_data *data, dev_t dev)
{
	int ret, t;
	struct usb_case_state *state = NULL;
	state = malloc(sizeof *state);
	if (!state) {
		BLTS_DEBUG("OOM\n");
		return 0;
	}
	memset(state, 0, sizeof *state);
	state->usb_data = data;

	for (t = 0; t < N_GADGETDRV_DEVICES; t++) {
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

dev_t find_gadget_dev(void* user_ptr)
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
		if (strstr(line, data->peripheral_driver))
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

static int test_write(char *node_name, unsigned size)
{
	int fdw, ret;
	unsigned i;
	char *buf;

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

	while (1) {
		ret = write(fdw, buf, size);
		if (ret < 0) {
			if (errno == EOVERFLOW) {
				BLTS_DEBUG("Write: Overflow.\n");
				/* Try to continue */
			} else {
				BLTS_LOGGED_PERROR("write");
				break;
			}
		}
	}

	free(buf);
	close(fdw);
	return ret >= 0 ? 0 : -errno;
}

static int test_read(char *node_name, unsigned size)
{
	int fdr, ret;
	unsigned i;
	char *buf;

	buf = malloc(size);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, size);

	fdr = open(node_name, O_RDONLY);
	if (fdr < 0) {
		BLTS_LOGGED_PERROR("open test_read()");
		free(buf);
		return -errno;
	}

	while (1) {
		ret = read(fdr, buf, size);
		if (ret < 0) {
			if (errno == EOVERFLOW) {
				BLTS_DEBUG("Read: Overflow.\n");
				/* Try to continue */
			} else {
				BLTS_LOGGED_PERROR("read");
				break;
			}
		}
		for (i = 0; i < size; i++) {
			if (buf[i] == i % 63)
				continue;
			BLTS_DEBUG("Data consistency error\n \tData[%i] = '%02X'\n \tShould be '%02X'\n", i, buf[i], i%63);
			BLTS_DEBUG("Current transfer of data is therefore corrupted, data frame size %i bytes\n", size);
			break;
		}

	}

	free(buf);
	close(fdr);
	return ret >= 0 ? 0 : -errno;
}

static void *read_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	data->ret = test_read(data->node_name, data->transfer_size);
	return NULL;
}

static void *write_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	data->ret = test_write(data->node_name, data->transfer_size);
	return NULL;
}

static int test_loop()
{
	int i;
	int fd, ret = 0;
	struct gadgetdrv_current_config conf;

	memset(&conf, 0, sizeof(struct gadgetdrv_current_config));
	memset(thread_data, 0, sizeof(thread_data));


	fd = open(node_names[0], O_RDONLY);
	if (fd < 0)
	{
		BLTS_LOGGED_PERROR("open test_loop()");
		BLTS_DEBUG("open (%s) failed.\n", node_names[0]);
		return -errno;
	}

	if (ioctl(fd, GADGETDRV_IOCGCONF, &conf) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (get conf)");
		ret = -errno;
		goto cleanup;
	}

	for (i = 1; i < N_GADGETDRV_DEVICES; i++)
	{
		if (conf.endpoints[i].type != USBDRV_EP_TYPE_NONE)
		{
			thread_data[i].transfer_size =
				conf.endpoints[i].transfer_size;
			thread_data[i].node_name = node_names[i];

			if (conf.endpoints[i].direction == USBDRV_EP_OUT)
			{
				BLTS_DEBUG("Starting read thread for ep %d...\n", i);
				ret = pthread_create(&thread_data[i].id, NULL,
					read_thread_function, &thread_data[i]);
				if (ret) {
					BLTS_DEBUG("Failed to create read thread "\
						"(%d)!\n", ret);
					goto cleanup;
				}
			} else {
				BLTS_DEBUG("Starting write thread for ep %d...\n", i);
				ret = pthread_create(&thread_data[i].id, NULL,
					write_thread_function, &thread_data[i]);
				if (ret) {
					BLTS_DEBUG("failed to create write thread "\
						"(%d)!\n", ret);
					goto cleanup;
				}
			}
		}
	}

	for (i = 0; i < N_GADGETDRV_DEVICES; i++)
		if (thread_data[i].id) {
			pthread_join(thread_data[i].id, NULL);
			thread_data[i].id = 0;
			if (thread_data[i].ret)
				ret = thread_data[i].ret;
		}

cleanup:
	close(fd);
	for (i = 0; i < N_GADGETDRV_DEVICES; i++)
		if (thread_data[i].id)
			pthread_cancel(thread_data[i].id);

	return ret;
}

static int configure_gadget(struct gadgetdrv_current_config *conf)
{
	int fd, ret = 0;

	fd = open(node_names[0], O_RDONLY);
	if (fd < 0)
	{
		BLTS_LOGGED_PERROR("open configure_gadget()");
		return -errno;
	}

	if (ioctl(fd, GADGETDRV_IOCSTOP) < 0 && errno != ENODEV)
	{
		BLTS_LOGGED_PERROR("ioctl (IOCSTOP)");
		ret = -errno;
		goto cleanup;
	}

	if (ioctl(fd, GADGETDRV_IOCSCONF, conf) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (set conf)");
		ret = -errno;
		goto cleanup;
	}

	/* done, start the gadget */
	if (ioctl(fd, GADGETDRV_IOCSTART) < 0)
	{
		BLTS_LOGGED_PERROR("ioctl (IOCSTART)");
		ret = -errno;
	}

cleanup:
	close(fd);
	return ret;
}

static enum usbdrv_device_speed speed_from_str(const char *str)
{
	if (!strcmp(str, "any"))
		return USBDRV_SPEED_UNKNOWN;
	if (!strcmp(str, "low"))
		return USBDRV_SPEED_LOW;
	if (!strcmp(str, "full"))
		return USBDRV_SPEED_FULL;
	if (!strcmp(str, "high"))
		return USBDRV_SPEED_HIGH;
	return USBDRV_SPEED_UNKNOWN;
}

static enum usbdrv_endpoint_type eptype_from_str(const char *str)
{
	if (!strcmp(str, "control"))
		return USBDRV_EP_TYPE_CONTROL;
	if (!strcmp(str, "bulk"))
		return USBDRV_EP_TYPE_BULK;
	if (!strcmp(str, "isoc"))
		return USBDRV_EP_TYPE_ISOC;
	if (!strcmp(str, "int"))
		return USBDRV_EP_TYPE_INT;
	return USBDRV_EP_TYPE_NONE;
}


static enum usbdrv_endpoint_direction_flags epdirection_from_str(
	const char *str)
{
	if (!strcmp(str, "in"))
		return USBDRV_EP_IN;
	return USBDRV_EP_OUT;
}


struct gadgetdrv_current_config *read_config(void* user_ptr)
{
	my_usb_data *data = ((my_usb_data *) user_ptr);


	FILE *fp = NULL;
	struct gadgetdrv_current_config *conf = NULL;
	size_t len = 0;
	ssize_t read;
	char *line = NULL;
	char type[32];
	char direction[32];
	char speed[32];
	char used_speed[32];
	int maxpacket = 0;
	int interval = 0;
	int ep_num = 0;
	int maxpower = 0;

	conf = malloc(sizeof(*conf));
	if (!conf) {
		BLTS_LOGGED_PERROR("malloc");
		return NULL;
	}

	memset(conf, 0, sizeof(*conf));

	BLTS_DEBUG("opening %s\n", data->endpoint_configuration_file);
	fp = fopen(data->endpoint_configuration_file, "r");
	if (!fp) {
		BLTS_LOGGED_PERROR("fopen");
		goto error_exit;
	}

	conf->speed = USBDRV_SPEED_HIGH;
	conf->max_power = 100;

	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[0] == '#')
			continue;
		if (sscanf(line, "%d %s %s %d %d", &ep_num, type,
			direction, &maxpacket, &interval) == 5) {

			if (ep_num < 1 || ep_num > 15) {
				BLTS_DEBUG("invalid endpoint in conf file (%s)\n",
					line);
				goto error_exit;
			}

			conf->endpoints[ep_num].type = eptype_from_str(type);
			conf->endpoints[ep_num].direction = epdirection_from_str(direction);
			conf->endpoints[ep_num].max_packet_size = maxpacket;
			conf->endpoints[ep_num].interval = interval;
			conf->endpoints[ep_num].transfer_size = data->endpoint_transfer_size;
		} else if (sscanf(line, "%s %s %d", speed, used_speed,
			&maxpower) == 3) {
			conf->max_power = maxpower;
			conf->speed = speed_from_str(speed);
			conf->used_speed = speed_from_str(used_speed);
		}
	}

	if (line)
		free(line);
	fclose(fp);
	return conf;

error_exit:
	if (line)
		free(line);
	if (conf)
		free(conf);
	if (fp)
		fclose(fp);
	return NULL;

/*	struct gadgetdrv_current_config *conf = NULL;
	int i = 0;

	conf = malloc(sizeof(*conf));
	if (!conf) {
		perror("malloc");
		return NULL;
	}

	memset(conf, 0, sizeof(*conf));

	if(data->endpoint_count > 15)
	{
		BLTS_DEBUG("Too many endpoints.\n");
		goto error_exit;
	}

	BLTS_DEBUG("*---------------------*\n");
	BLTS_DEBUG("Number of endpoints: %d\n", data->endpoint_count);
	BLTS_DEBUG("Type: %s\n", data->endpoint_type);
	BLTS_DEBUG("Max packet size: %d\n", data->endpoint_max_packet_size);
	BLTS_DEBUG("Interval: %d\n", data->endpoint_interval);
	BLTS_DEBUG("Transfer size: %d\n", data->endpoint_transfer_size);
	BLTS_DEBUG("Max power: %d\n", data->usb_max_power);
	BLTS_DEBUG("Speed: %s\n", data->usb_speed);
	BLTS_DEBUG("*---------------------*\n");

	for(i=1;i<=data->endpoint_count;i++)
	{
		if(i%2)
			conf->endpoints[i].direction = USBDRV_EP_IN;
		else
			conf->endpoints[i].direction = USBDRV_EP_OUT;
		conf->endpoints[i].type = eptype_from_str(data->endpoint_type);
		conf->endpoints[i].max_packet_size = data->endpoint_max_packet_size;
		conf->endpoints[i].interval = data->endpoint_interval;
		conf->endpoints[i].transfer_size = data->endpoint_transfer_size;
	}

	conf->max_power = data->usb_max_power;
	conf->speed = speed_from_str(data->usb_speed);
	conf->used_speed = speed_from_str(data->usb_speed);

	return conf;

error_exit:
	if (conf)
		free(conf);
	return NULL;*/
}

/* Test cases ------------> */

/* Transfer data using bulk endpoints */
int blts_setup_peripheral_driver(void* user_ptr, __attribute__((unused)) int testnum)
{
	int ret = -1;
	char *command;
	struct usb_case_state *test = NULL;
	struct gadgetdrv_current_config *conf = NULL;
	my_usb_data *data = ((my_usb_data *) user_ptr);

	if (!data)
		return -EINVAL;

	BLTS_DEBUG("Setup peripheral driver: %s%s.ko\n", data->peripheral_driver_path, data->peripheral_driver);

	ret = asprintf(&command, "insmod %s%s.ko", data->peripheral_driver_path, data->peripheral_driver);
	if (ret<0)
		return -ENOMEM;
	ret = (WEXITSTATUS(system(command)));

	dev_t dev = find_gadget_dev(data);
	BLTS_DEBUG("Driver:: dev = %u:%u\n", major(dev), minor(dev));

	conf = read_config(data);
	if (!conf) {
		BLTS_DEBUG("Bad configuration, aborting test.\n");
		goto done;
	}

	if (!major(dev))
	{
		BLTS_DEBUG("Bad major, aborting test.\n");
		goto done;
	}

	test = usb_state_init(data, dev);

	BLTS_DEBUG("Configuring driver...\n");
	sleep(1);
	ret = configure_gadget(conf);
	BLTS_DEBUG("Connect USB cable and press key to continue...\n");
	getchar();
	BLTS_DEBUG("Starting loop... (Press CTRL-C to stop.)\n");
	ret = test_loop();

done:
	if (conf)
		free(conf);
	free (command);

	do_gadget_cleanup();
	usb_state_finalize(test);

	return ret;
}

/* Convert generated parameters to test case format. */
void *peripheral_variant_set_arg_processor(struct boxed_value *args, void *user_ptr)
{
	char *usb_peripheral_driver = 0;
	char *usb_peripheral_driver_path = 0;
	int endpoint_count = 0;
	char* endpoint_type = 0;
	char* endpoint_configuration_file;
	int endpoint_max_packet_size = 0;
	int endpoint_interval = 0;
	int endpoint_transfer_size = 0;
	my_usb_data *data = ((my_usb_data *) user_ptr);
	if (!data)
		return 0;

	usb_peripheral_driver = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	usb_peripheral_driver_path = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	endpoint_transfer_size = blts_config_boxed_value_get_int(args);
	args = args->next;
	endpoint_configuration_file = strdup(blts_config_boxed_value_get_string(args));
	/*endpoint_count = blts_config_boxed_value_get_int(args);
	args = args->next;
	endpoint_type = strdup(blts_config_boxed_value_get_string(args));
	args = args->next;
	endpoint_max_packet_size = blts_config_boxed_value_get_int(args);
	args = args->next;
	endpoint_interval = blts_config_boxed_value_get_int(args);*/
/*	args = args->next;
	endpoint_transfer_size = blts_config_boxed_value_get_int(args);*/

	/* These are already non-zero, if set on command line */

	if (data->peripheral_driver)
		free(usb_peripheral_driver);
	else
		data->peripheral_driver = usb_peripheral_driver;

	if (data->peripheral_driver_path)
		free(usb_peripheral_driver_path);
	else
		data->peripheral_driver_path = usb_peripheral_driver_path;

	if (!data->endpoint_count)
		data->endpoint_count = endpoint_count;

	if (data->endpoint_type)
		free(endpoint_type);
	else
		data->endpoint_type = endpoint_type;

	if (data->endpoint_configuration_file)
		free(endpoint_configuration_file);
	else
		data->endpoint_configuration_file = endpoint_configuration_file;

	if (!data->endpoint_max_packet_size)
		data->endpoint_max_packet_size = endpoint_max_packet_size;

	if (!data->endpoint_interval)
		data->endpoint_interval = endpoint_interval;

	if (!data->endpoint_transfer_size)
		data->endpoint_transfer_size = data->endpoint_max_packet_size;

	return data;
}
