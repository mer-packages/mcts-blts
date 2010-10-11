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
#include <pthread.h>
#include <poll.h>

#include "gadgetdrv_common.h"

#define EPNODE_FMT "gadget_dev_node%d"
static char node_names[N_GADGETDRV_DEVICES][128];

static pthread_t logthread_id;

struct rw_thread_data {
	pthread_t id;
	char *node_name;
	unsigned transfer_size;
};

struct rw_thread_data thread_data[N_GADGETDRV_DEVICES];

static dev_t find_dev()
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
		if (strstr(line, "blts_gadget")) {
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
	return makedev(dev, 0);
}

static void print_ep_type(unsigned t)
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

static void print_ep_state(unsigned s)
{
	if (s & USBDRV_EP_IN)
		printf("IN");
	if (s & USBDRV_EP_OUT)
		printf("OUT");
}

static int print_ep_configuration()
{
	int fd, i;
	struct gadgetdrv_current_config conf;

	memset(&conf, 0, sizeof(struct gadgetdrv_current_config));

	fd = open(node_names[0], O_RDONLY);
	if (fd < 0) {
		fd = open(node_names[0], O_WRONLY);
		if (fd < 0) {
			perror("open");
			return -errno;
		}
	}

	if (ioctl(fd, GADGETDRV_IOCGCONF, &conf) < 0) {
		perror("ioctl (get conf)");
		return -errno;
	}

	printf("conf (read 1):\n");
	for (i = 0; i < N_GADGETDRV_DEVICES; ++i) {
		printf("\t(");
		print_ep_type(conf.endpoints[i].type);
		printf(":");
		print_ep_state(conf.endpoints[i].direction);
		printf(":%u", conf.endpoints[i].max_packet_size);
		printf(":%u", conf.endpoints[i].interval);
		printf(":%u", conf.endpoints[i].transfer_size);
		printf(")\n");
	}

	close(fd);
	return 0;
}

static void *log_thread_function(void *ptr)
{
	int ret;
	char buf[4096];
	struct pollfd fdp;
	int fdw;
	ptr = ptr;

	fdw = open("gadgetdrv_log.txt", O_WRONLY|O_CREAT|O_APPEND, 00644);
	if (fdw < 0) {
		perror("open");
		return NULL;
	}

	fdp.fd = open("/sys/module/blts_gadget/logging/log", O_RDWR);
	if (fdp.fd < 0) {
		perror("open");
		close(fdw);
		return NULL;
	}

	fdp.events = POLLPRI;
	fdp.revents = 0;

	while (1) {
		ret = poll(&fdp, 1, 10000);
		if (ret < 0) {
			perror("poll");
			break;
		} else if (ret > 0) {
			lseek(fdp.fd, SEEK_SET, 0);
			ret = read(fdp.fd, buf, 4096);
			if (ret < 0) {
				perror("read");
				break;
			}
			ret = write(fdw, buf, ret);
			if (ret < 0) {
				perror("write");
				break;
			}
			/* printf(buf); */
		}
	}

	close(fdw);
	close(fdp.fd);
	return NULL;
}

static int test_write(char *node_name, unsigned size)
{
	int fdw, ret;
	char *buf;

	buf = malloc(size);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, size);

	fdw = open(node_name, O_WRONLY);
	if (fdw < 0) {
		perror("open");
		free(buf);
		return -errno;
	}

	while (1) {
		ret = write(fdw, buf, size);
		if (ret < 0) {
			if (errno == EOVERFLOW) {
				printf("write: Overflow\n");
				/* Try to continue */
			} else {
				perror("write");
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
	char *buf;

	buf = malloc(size);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, size);

	fdr = open(node_name, O_RDONLY);
	if (fdr < 0) {
		perror("open");
		free(buf);
		return -errno;
	}

	while (1) {
		ret = read(fdr, buf, size);
		if (ret < 0) {
			if (errno == EOVERFLOW) {
				printf("read: Overflow\n");
				/* Try to continue */
			} else {
				perror("read");
				break;
			}
		}
	}

	free(buf);
	close(fdr);
	return ret >= 0 ? 0 : -errno;
}

static void *read_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	test_read(data->node_name, data->transfer_size);
	return NULL;
}

static void *write_thread_function(void *ptr)
{
	struct rw_thread_data *data = (struct rw_thread_data *)ptr;
	test_write(data->node_name, data->transfer_size);
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
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (ioctl(fd, GADGETDRV_IOCGCONF, &conf) < 0) {
		perror("ioctl (get conf)");
		ret = -errno;
		goto cleanup;
	}

	for (i = 1; i < N_GADGETDRV_DEVICES; i++) {
		if (conf.endpoints[i].type != USBDRV_EP_TYPE_NONE) {
			thread_data[i].transfer_size =
				conf.endpoints[i].transfer_size;
			thread_data[i].node_name = node_names[i];

			if (conf.endpoints[i].direction == USBDRV_EP_OUT) {
				printf("Starting read thread for ep %d\n", i);
				ret = pthread_create(&thread_data[i].id, NULL,
					read_thread_function, &thread_data[i]);
				if (ret) {
					printf("failed to create read thread "\
						"(%d)!\n", ret);
					goto cleanup;
				}
			} else {
				printf("Starting write thread for ep %d\n", i);
				ret = pthread_create(&thread_data[i].id, NULL,
					write_thread_function, &thread_data[i]);
				if (ret) {
					printf("failed to create write thread "\
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
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (ioctl(fd, GADGETDRV_IOCSCONF, conf) < 0) {
		perror("ioctl (set conf)");
		ret = -errno;
		goto cleanup;
	}

	/* done, start the gadget */
	if (ioctl(fd, GADGETDRV_IOCSTART) < 0) {
		perror("ioctl (IOCSTART)");
		ret = -errno;
	}

	ret = print_ep_configuration();

cleanup:
	close(fd);
	return ret >= 0 ? 0 : -errno;
}

static int stop_gadget()
{
	int fd, ret = 0;

	fd = open(node_names[0], O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -errno;
	}

	if (ioctl(fd, GADGETDRV_IOCSTOP) < 0) {
		perror("ioctl (IOCSTOP)");
		ret = -errno;
	}

	close(fd);
	return ret >= 0 ? 0 : -errno;
}

static void do_setup(dev_t dev)
{
	int ret, t;

	for (t = 0; t < N_GADGETDRV_DEVICES; t++) {
		sprintf(node_names[t], EPNODE_FMT, t);
		ret = mknodat(AT_FDCWD, node_names[t], S_IFCHR | 0600,
			makedev(major(dev), t));
		if (ret)
			printf("mknodat (%s) failed\n", node_names[t]);
	}

	ret = pthread_create(&logthread_id, NULL, log_thread_function, NULL);
	if (ret)
		printf("failed to create log reader thread (%d)!\n", ret);
}

static void do_cleanup()
{
	int t;

	sleep(1);
	if (logthread_id)
		pthread_cancel(logthread_id);

	for (t = 0; t < N_GADGETDRV_DEVICES; t++)
		unlink(node_names[t]);
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

static enum usbdrv_endpoint_direction_flags epdirection_from_str(
	const char *str)
{
	if (!strcmp(str, "in"))
		return USBDRV_EP_IN;
	return USBDRV_EP_OUT;
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

struct gadgetdrv_current_config *read_config()
{
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
	int tr_size = 0;

	conf = malloc(sizeof(*conf));
	if (!conf) {
		perror("malloc");
		return NULL;
	}

	memset(conf, 0, sizeof(*conf));

	printf("opening test_gadgetdrv.cfg\n");
	fp = fopen("test_gadgetdrv.cfg", "r");
	if (!fp) {
		perror("fopen");
		goto error_exit;
	}

	conf->speed = USBDRV_SPEED_HIGH;
	conf->max_power = 100;

	while ((read = getline(&line, &len, fp)) != -1) {
		if (line[0] == '#')
			continue;
		if (sscanf(line, "%d %s %s %d %d %d", &ep_num, type,
			direction, &maxpacket, &interval, &tr_size) == 6) {

			if (ep_num < 1 || ep_num > 15) {
				printf("invalid endpoint in conf file (%s)\n",
					line);
				goto error_exit;
			}

			conf->endpoints[ep_num].type = eptype_from_str(type);
			conf->endpoints[ep_num].direction =
				epdirection_from_str(direction);
			conf->endpoints[ep_num].max_packet_size = maxpacket;
			conf->endpoints[ep_num].interval = interval;
			conf->endpoints[ep_num].transfer_size = tr_size;
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
}

int main(int argc, char **argv)
{
	int ret = -1;
	struct gadgetdrv_current_config *conf = NULL;
	dev_t dev = find_dev();
	printf("gadgetdrv: dev = %u:%u\n", major(dev), minor(dev));

	if (argc <= 1) {
		printf("Usage: test_gadgetdrv [-c] [-r] [-w] [-l] [-p] [-g]\n");
		printf("-c: Configure gadget <- use this after loading "\
			"blts_gadget.ko\n");
		printf("-s: Stop gadget\n");
		printf("-r: Read test\n");
		printf("-w: Write test\n");
		printf("-g: g_zero like traffic generation\n");
		printf("-p: Print current ep configuration\n");
		goto done;
	}

	conf = read_config();
	if (!conf) {
		printf("could not open configuration file!\n");
		goto done;
	}

	if (!major(dev)) {
		printf("bad major, aborting test\n");
		goto done;
	}

	do_setup(dev);

	if (!strcmp(argv[1], "-c"))
		ret = configure_gadget(conf);
	else if (!strcmp(argv[1], "-s"))
		ret = stop_gadget();
	else if (!strcmp(argv[1], "-r"))
		ret = test_read(node_names[2], 4096);
	else if (!strcmp(argv[1], "-w"))
		ret = test_write(node_names[1], 4096);
	else if (!strcmp(argv[1], "-p"))
		ret = print_ep_configuration();
	else if (!strcmp(argv[1], "-g"))
		ret = test_loop();

done:
	if (conf)
		free(conf);

	do_cleanup();
	return ret;
}

