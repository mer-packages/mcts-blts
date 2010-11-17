/* input_test_cases.c -- Input device test cases

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
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <asm/bitsperlong.h>

#include <blts_cli_frontend.h>
#include <blts_timing.h>
#include <blts_log.h>

#include "input_test_cases.h"
#include "input_util.h"

#define BIT_WORD(nr) ((nr) / __BITS_PER_LONG)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

const char str_unknown[] = "unknown";

struct mt_touchpoint {
	int id;
	int x;
	int y;
};

struct input_vn {
	unsigned value;
	char name[32];
};

struct input_vn buses[] = {
	{ BUS_PCI, "PCI" },
	{ BUS_ISAPNP, "ISAPNP" },
	{ BUS_USB, "USB" },
	{ BUS_HIL, "HIL" },
	{ BUS_BLUETOOTH, "BLUETOOTH" },
	{ BUS_VIRTUAL, "VIRTUAL" },
	{ BUS_ISA, "ISA" },
	{ BUS_I8042, "I8042" },
	{ BUS_XTKBD, "XTKBD" },
	{ BUS_RS232, "RS232" },
	{ BUS_GAMEPORT, "GAMEPORT" },
	{ BUS_PARPORT, "PARPORT" },
	{ BUS_AMIGA, "AMIGA" },
	{ BUS_ADB, "ADB" },
	{ BUS_I2C, "I2C" },
	{ BUS_HOST, "HOST" },
	{ BUS_GSC, "GSC" },
	{ BUS_ATARI, "ATARI" },
};

struct input_vn ff_types[] = {
	{ FF_CONSTANT, "constant" },
	{ FF_PERIODIC, "periodic" },
	{ FF_SQUARE, "square" },
	{ FF_TRIANGLE, "triangle" },
	{ FF_SINE, "sine" },
	{ FF_SAW_UP, "saw up" },
	{ FF_SAW_DOWN, "saw down" },
	{ FF_CUSTOM, "custom" },
	{ FF_RAMP, "ramp" },
	{ FF_SPRING, "spring" },
	{ FF_FRICTION, "friction" },
	{ FF_DAMPER, "damper" },
	{ FF_RUMBLE, "rumble" },
	{ FF_INERTIA, "inertia" },
	{ FF_GAIN, "gain" },
	{ FF_AUTOCENTER, "autocenter" },
};

struct input_vn ev_types[] = {
	{ EV_SYN, "Synch Events" },
	{ EV_KEY, "Keys or Buttons" },
	{ EV_REL, "Relative Axes" },
	{ EV_ABS, "Absolute Axes" },
	{ EV_MSC, "Miscellaneous" },
	{ EV_SW, "Switch" },
	{ EV_LED, "LEDs" },
	{ EV_SND, "Sounds" },
	{ EV_REP, "Repeat" },
	{ EV_FF, "Force Feedback" },
	{ EV_FF_STATUS, "Force Feedback status" },
	{ EV_PWR, "Power Management" },
};

static int test_bit(int nr, const unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (__BITS_PER_LONG-1)));
}

static const char *id_to_str(unsigned id, struct input_vn *map, unsigned size)
{
	unsigned t;

	for (t = 0; t < size; t++)
		if(map[t].value == id)
			return map[t].name;

	return str_unknown;
}

static int is_key_pressed(int fd, int key)
{
	char key_b[KEY_MAX/8 + 1];

	memset(key_b, 0, sizeof(key_b));
	if (ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b) < 0)
		return -errno;

	return !!(key_b[key/8] & (1<<(key % 8)));
}

static int is_any_key_pressed(int fd)
{
	char key_b[KEY_MAX/8 + 1];
	unsigned t;

	memset(key_b, 0, sizeof(key_b));
	if (ioctl(fd, EVIOCGKEY(sizeof(key_b)), key_b) < 0)
		return -errno;

	for (t = 0; t < sizeof(key_b); t++) {
		if (key_b[t])
			return 1;
	}

	return 0;
}

static int print_ff_info(int fd)
{
	int t;
	unsigned char fftype[64];

	memset(fftype, 0, sizeof(fftype));
	if (ioctl(fd, EVIOCGBIT(EV_FF, sizeof(fftype)), fftype) < 0) {
		BLTS_LOGGED_PERROR("EVIOCGBIT");
		return -errno;
	}

	BLTS_DEBUG("Supported FF types:\n");

	for (t = 0; t < FF_MAX; t++) {
		if (test_bit(t, (unsigned long *)fftype)) {
			BLTS_DEBUG("  type 0x%02x (%s)\n", t,
				id_to_str(t, ff_types, ARRAY_SIZE(ff_types)));
		}
	}

	return 0;
}

static int print_event_types(int fd)
{
	int t;
	unsigned char evtype_b[64];

	memset(evtype_b, 0, sizeof(evtype_b));
	if (ioctl(fd, EVIOCGBIT(0, EV_MAX), evtype_b) < 0) {
		BLTS_LOGGED_PERROR("EVIOCGBIT");
		return -errno;
	}

	BLTS_DEBUG("Supported event types:\n");

	for (t = 0; t < EV_MAX; t++) {
		if (test_bit(t, (unsigned long *)evtype_b)) {
			BLTS_DEBUG("  0x%02x (%s)\n", t,
				id_to_str(t, ev_types, ARRAY_SIZE(ev_types)));

			if (t == EV_FF) {
				if (print_ff_info(fd))
					return -1;
			}
		}
	}

	return 0;
}

static int print_device_info(const char *dev_name)
{
	int fd, ret = -1;
	int version;
	struct input_id device_info;
	const char *bname;
	char name[256];

	BLTS_DEBUG("\nDevice %s\n", dev_name);

	fd = open(dev_name, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
		BLTS_LOGGED_PERROR("EVIOCGNAME");
		goto cleanup;
	}

	BLTS_DEBUG("Name: '%s'\n", name);

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		BLTS_LOGGED_PERROR("EVIOCGVERSION");
		goto cleanup;
	}

	BLTS_DEBUG("Driver version: %d.%d.%d\n", version >> 16,
		(version >> 8) & 0xff, version & 0xff);

	if (ioctl(fd, EVIOCGID, &device_info)) {
		BLTS_LOGGED_PERROR("EVIOCGID");
		goto cleanup;
	}

	BLTS_DEBUG("Vendor: %04hx\n", device_info.vendor);
	BLTS_DEBUG("Product: %04hx\n", device_info.product);
	BLTS_DEBUG("Version: %04hx\n", device_info.version);

	bname = id_to_str(device_info.bustype, buses, ARRAY_SIZE(buses));
	if (bname)
		BLTS_DEBUG("Bus type: %s\n", bname)
	else
		BLTS_DEBUG("Bus type: Unknown (%d)\n", device_info.bustype)

	/* Optional */
	if (ioctl(fd, EVIOCGPHYS(sizeof(name)), name) >= 0) {
		BLTS_DEBUG("Path: '%s'\n", name);
	}

	/* Optional */
	if (ioctl(fd, EVIOCGUNIQ(sizeof(name)), name) >= 0) {
		BLTS_DEBUG("Identifier: '%s'\n", name);
	}

	if (print_event_types(fd))
		goto cleanup;

	ret = 0;

cleanup:
	close(fd);

	return ret;
}

static int read_input_events(int fd, struct input_event *evs, int size)
{
	int rb, ret;
	struct pollfd fdp;

	fdp.fd = fd;
	fdp.events = POLLIN|POLLERR;
	fdp.revents = 0;
	ret = poll(&fdp, 1, 1000);
	if (ret < 0) {
		BLTS_LOGGED_PERROR("poll");
		return -errno;
	} else if(!ret)
		return 0;

	rb = read(fd, evs, size);
	if (rb < (int)sizeof(struct input_event)) {
		BLTS_LOGGED_PERROR("read");
		return -errno;
	}

	return rb;
}

int input_enumerate_test(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	unsigned t, ret = 0;
	struct input_data *data = (struct input_data *)user_ptr;

	if (!data->num_devices) {
		BLTS_ERROR("No devices to test!\n");
		return -EINVAL;
	}

	for (t = 0; t < data->num_devices; t++) {
		if (print_device_info(data->devices[t])) {
			BLTS_ERROR("Could not read device info from '%s'\n",
				data->devices[t]);
			ret = -1;
		}
	}

	return ret;
}

/* For development purposes, not a test case */
int test_device(const char *device)
{
	int fd, t, ret = -1;
	size_t rb;
	struct input_event ev[64];

	fd = open(device, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	while (1) {
		rb = read(fd, ev, sizeof(struct input_event) * 64);
		if (rb < (int) sizeof(struct input_event)) {
			BLTS_LOGGED_PERROR("read");
			goto cleanup;
		}

		for (t = 0; t < (int) (rb / sizeof(struct input_event)); t++)
		{
			if (EV_KEY == ev[t].type)
				BLTS_DEBUG("%ld.%06ld ", ev[t].time.tv_sec,
					ev[t].time.tv_usec);

			BLTS_DEBUG("type %d code %d value %d\n", ev[t].type,
				ev[t].code, ev[t].value);
		}
	}

	ret = 0;

cleanup:
	close(fd);

	return ret;
}

int input_key_test(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int fd, t, ret = -1;
	int cnt = 0;
	size_t rb;
	struct input_event ev[64];
	struct input_data *data = (struct input_data *)user_ptr;
	struct key_mapping *key = &data->key;

	if (!strlen(key->device)) {
		BLTS_ERROR("No device to test!\n");
		return -EINVAL;
	}

	fd = open(key->device, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	if (is_any_key_pressed(fd)) {
		BLTS_DEBUG("Release any pressed keys\n");

		timing_start();
		while (is_any_key_pressed(fd) && timing_elapsed() < 60)
			usleep(100000);
		timing_stop();

		if (is_any_key_pressed(fd))
			goto cleanup;
	}

	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 1)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
			goto cleanup;
		}
	}

	/* TODO: Test also key repeat. Hold the button down for [n] seconds. */
	/* TODO: "Press key 'a/v jack'" sounds funny */

	BLTS_DEBUG("Press key '%s' (%d/%d)\n", key->description, key->code);

	while (timing_elapsed() < 10) {
		rb = read(fd, ev, sizeof(struct input_event) * 64);
		if (rb < (int) sizeof(struct input_event)) {
			BLTS_LOGGED_PERROR("read");
			goto cleanup;
		}

		for (t = 0; t < (int) (rb / sizeof(struct input_event)); t++)
		{
			if (EV_KEY == ev[t].type)
				BLTS_TRACE("%ld.%06ld ", ev[t].time.tv_sec,
					ev[t].time.tv_usec);

			BLTS_TRACE("type %d code %d value %d\n", ev[t].type,
				ev[t].code, ev[t].value);
			/* TODO: Check for incorrect events */
			if (ev[t].code == key->code && ++cnt == 2) {
				BLTS_DEBUG("Got correct event\n");
				ret = 0;
				goto cleanup;
			}
		}
	}
	timing_stop();
	BLTS_DEBUG("Timeout while waiting\n");

cleanup:
	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 0)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
		}
	}

	close(fd);

	return ret;
}

int input_pointer_test(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int fd, rb, t, pressed = 0, ret = -1;
	int lx, ly, x, y;
	struct input_event ev[64];
	struct input_data *data = (struct input_data *)user_ptr;
	struct window_struct ws;

	fd = open(data->pointer_device, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	ret = input_create_window(&ws, "Input test", 0, 0, 0);
	if (ret) {
		BLTS_ERROR("Failed to create window\n");
		goto cleanup;
	}
	/* mcompositor hack, remove later */
	sleep(1);
	XSync(ws.display, False);

	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 1)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
			goto cleanup;
		}
	}

	XSetLineAttributes(ws.display, ws.gc, 4, LineSolid, CapNotLast, JoinMiter);
	XSetForeground(ws.display, ws.gc, 0x00FFFFFF);
	for (x = 0; x < (int)ws.width * 3; x += 200) {
		XDrawLine(ws.display, ws.window, ws.gc, x - ws.width, 0, x, ws.height);
		XDrawLine(ws.display, ws.window, ws.gc, x, 0, x - ws.width, ws.height);
	}
	XSetForeground(ws.display, ws.gc, 0x00000F00);
	XFlush(ws.display);

	x = y = lx = ly = 0;

	timing_start();

	while (timing_elapsed() < 30) {
		rb = read_input_events(fd, ev, sizeof(struct input_event) * 64);
		if (!rb)
			continue;
		else if (rb < 0) {
			ret = rb;
			goto cleanup;
		}

		for (t = 0; t < (int) (rb / sizeof(struct input_event)); t++)
		{
			if (EV_KEY == ev[t].type)
				BLTS_TRACE("%ld.%06ld ", ev[t].time.tv_sec,
					ev[t].time.tv_usec);

			BLTS_TRACE("type %d code %d value %d\n", ev[t].type,
				ev[t].code, ev[t].value);

			if (ev[t].code == BTN_TOUCH)
				pressed = ev[t].value;

			if (ev[t].code == ABS_MT_TOUCH_MAJOR)
				XSetLineAttributes(ws.display, ws.gc,
					ev[t].value, LineSolid, CapNotLast, JoinMiter);

			if (pressed) {
				if (ev[t].code == ABS_MT_POSITION_X)
					x = ev[t].value;
				else if (ev[t].code == ABS_MT_POSITION_Y)
					y = ev[t].value;
			} else {
				x = y = lx = ly = 0;
			}
		}

		if (lx && ly)
			XDrawLine(ws.display, ws.window, ws.gc, x, y, lx, ly);

		XFlush(ws.display);
		lx = x; ly = y;
	}

	timing_stop();

	XImage *img = XGetImage(ws.display, ws.window, 0, 0,
		ws.width, ws.height, ~0, XYPixmap);
	if (img) {
		input_write_bmp(&ws, "testi.bmp", img);
		XFree(img);
	}

	ret = 0;

cleanup:
	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 0)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
		}
	}

	input_close_window(&ws);

	close(fd);

	return ret;

}

static int wait_for_st_event(struct input_data *data,
	int x_min, int y_min, int x_max, int y_max)
{
	int fd, rb, t, ret = -1;
	int x, y, got_x, got_y, got_press, abs_min_x, abs_min_y;
	float x_scale, y_scale;
	int st = 0;
	struct input_event ev[64];
	struct input_absinfo abs;

	fd = open(data->pointer_device, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 1)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
			goto cleanup;
		}
	}

	if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs)) {
		BLTS_LOGGED_PERROR("EVIOCGABS(ABS_MT_POSITION_X)");
		ret = -errno;
		goto  cleanup;
	}

	if (!abs.minimum && !abs.maximum) {
		st = 1;
		if (ioctl(fd, EVIOCGABS(ABS_X), &abs)) {
			BLTS_LOGGED_PERROR("EVIOCGABS(ABS_X)");
			ret = -errno;
			goto  cleanup;
		}
	}
	BLTS_TRACE("X limits: %d - %d\n", abs.minimum, abs.maximum);
	if (abs.maximum - abs.minimum == 0) {
		BLTS_ERROR("Invalid X coordinate limits (%d - %d)\n",
			abs.minimum, abs.maximum);
		ret = -EINVAL;
		goto cleanup;
	}
	abs_min_x = abs.minimum;
	if (data->swap_xy) {
		x_scale = (float) data->scr_height /
			(float)(abs.maximum - abs.minimum);
	} else {
		x_scale = (float) data->scr_width /
			(float)(abs.maximum - abs.minimum);
	}

	if (st) {
		if (ioctl(fd, EVIOCGABS(ABS_Y), &abs)) {
			BLTS_LOGGED_PERROR("EVIOCGABS(ABS_Y)");
			ret = -errno;
			goto  cleanup;
		}
	} else {
		if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs)) {
			BLTS_LOGGED_PERROR("EVIOCGABS(ABS_MT_POSITION_Y)");
			ret = -errno;
			goto  cleanup;
		}
	}
	BLTS_TRACE("Y limits: %d - %d\n", abs.minimum, abs.maximum);
	if (abs.maximum - abs.minimum == 0) {
		BLTS_ERROR("Invalid Y coordinate limits (%d - %d)\n",
			abs.minimum, abs.maximum);
		ret = -EINVAL;
		goto cleanup;
	}
	abs_min_y = abs.minimum;
	if (data->swap_xy) {
		y_scale = (float) data->scr_width /
			(float)(abs.maximum - abs.minimum);
	} else {
		y_scale = (float) data->scr_height /
			(float)(abs.maximum - abs.minimum);
	}

	timing_start();

	got_x = got_y = got_press = 0;
	x = y = -1;
	while (timing_elapsed() < 30) {
		rb = read_input_events(fd, ev, sizeof(struct input_event) * 64);
		if (!rb)
			continue;
		else if (rb < 0) {
			ret = rb;
			goto cleanup;
		}

		for (t = 0; t < (int) (rb / sizeof(struct input_event)); t++) {
			if (EV_KEY == ev[t].type)
				BLTS_TRACE("%ld.%06ld ", ev[t].time.tv_sec,
					ev[t].time.tv_usec);

			BLTS_TRACE("type 0x%x code 0x%x value %d\n", ev[t].type,
				ev[t].code, ev[t].value);

			if (st) {
				if (ev[t].type == EV_KEY &&
					ev[t].code == BTN_TOUCH) {
					got_press = 1;
				} else if (ev[t].type == EV_ABS &&
					ev[t].code == ABS_X) {
					x = ev[t].value;
					got_x = 1;
				} else if(ev[t].type == EV_ABS &&
					ev[t].code == ABS_Y) {
					y = ev[t].value;
					got_y = 1;
				}
			} else {
				if (ev[t].type == EV_KEY &&
					ev[t].code == BTN_TOUCH) {
					got_press = 1;
				} else if (ev[t].type == EV_ABS &&
					ev[t].code == ABS_MT_POSITION_X) {
					x = ev[t].value;
					got_x = 1;
				} else if(ev[t].type == EV_ABS &&
					ev[t].code == ABS_MT_POSITION_Y) {
					y = ev[t].value;
					got_y = 1;
				}
			}
		}

		if (got_x && got_y && got_press) {
			if (data->swap_xy) {
				t = y;
				y = (float)(x - abs_min_x) * x_scale;
				x = (float)(t - abs_min_y) * y_scale;
			} else {
				x = (float)(x - abs_min_x) * x_scale;
				y = (float)(y - abs_min_y) * y_scale;
			}

			if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
				BLTS_DEBUG("Inside the area (x: %d, y: %d)\n",
					x, y);
				ret = 0;
			} else {
				BLTS_DEBUG("Outside the area (x: %d, y: %d)\n",
					x, y);
				ret = -1;
			}
			goto cleanup;
		}
	}
	ret = -1;

cleanup:
	timing_stop();

	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 0)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
		}
	}
	close(fd);
	return ret;
}

int input_single_touch_test(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int ret = 0;
	struct input_data *data = (struct input_data *)user_ptr;
	struct window_struct ws;

	ret = input_create_window(&ws, "Input test", 0, 0, 0);
	if (ret) {
		BLTS_ERROR("Failed to create window\n");
	}

	if (ws.display) {
		sleep(1);
		XSync(ws.display, False);
		XSetForeground(ws.display, ws.gc, 0x00FFFFFF);

		/* Upper left corner */
		XClearWindow(ws.display, ws.window);
		XFillRectangle(ws.display, ws.window, ws.gc,
			0, 0, ws.width / 2, ws.height / 2);
		XSync(ws.display, False);
	}

	BLTS_DEBUG("Touch upper left corner (white box) within 30 seconds\n");

	ret = wait_for_st_event(data, 0, 0,
		data->scr_width / 2, data->scr_height / 2);
	if (ret)
		goto cleanup;

	BLTS_DEBUG("ok\n");
	sleep(2);

	/* Upper right corner */
	if (ws.display) {
		XClearWindow(ws.display, ws.window);
		XFillRectangle(ws.display, ws.window, ws.gc,
			ws.width / 2, 0, ws.width, ws.height / 2);
		XSync(ws.display, False);
	}

	BLTS_DEBUG("Touch upper right corner (white box) within 30 seconds\n");

	ret = wait_for_st_event(data, data->scr_width / 2, 0,
		data->scr_width, data->scr_height / 2);
	if (ret)
		goto cleanup;

	BLTS_DEBUG("ok\n");
	sleep(2);

	/* Lower right corner */
	if (ws.display) {
		XClearWindow(ws.display, ws.window);
		XFillRectangle(ws.display, ws.window, ws.gc,
			ws.width / 2, ws.height / 2, ws.width, ws.height);
		XSync(ws.display, False);
	}

	BLTS_DEBUG("Touch lower right corner (white box) within 30 seconds\n");

	ret = wait_for_st_event(data, data->scr_width / 2, data->scr_height / 2,
		data->scr_width, data->scr_height);
	if (ret)
		goto cleanup;

	BLTS_DEBUG("ok\n");
	sleep(2);

	/* Lower left corner */
	if (ws.display) {
		XClearWindow(ws.display, ws.window);
		XFillRectangle(ws.display, ws.window, ws.gc,
			0, ws.height / 2, ws.width / 2, ws.height);
		XSync(ws.display, False);
	}

	BLTS_DEBUG("Touch lower left corner (white box) within 30 seconds\n");

	ret = wait_for_st_event(data, 0, data->scr_height / 2,
		data->scr_width / 2, data->scr_height);
	if (ret)
		goto cleanup;

	BLTS_DEBUG("ok\n");
	sleep(2);

cleanup:
	input_close_window(&ws);

	return ret;
}

int input_multi_touch_test(void *user_ptr, int test_num)
{
	BLTS_UNUSED_PARAM(test_num)
	int ret, rb, fd, t, x, y, current_tp;
	int num_tps, got_x, got_y, abs_min_x, abs_min_y;
	float x_scale, y_scale;
	struct input_data *data = (struct input_data *)user_ptr;
	struct window_struct ws;
	struct input_event ev[64];
	struct mt_touchpoint tps[64];
	struct input_absinfo abs;

	fd = open(data->pointer_device, O_RDWR);
	if (!fd) {
		BLTS_LOGGED_PERROR("Failed to open device");
		return -errno;
	}

	if (!data->no_grab) {
		if (ioctl(fd, EVIOCGRAB, 1)) {
			BLTS_LOGGED_PERROR("EVIOCGRAB");
			ret = -errno;
			goto cleanup;
		}
	}

	ret = input_create_window(&ws, "Input test", 0, 0, 0);
	if (ret) {
		BLTS_ERROR("Failed to create window\n");
	}

	if (ws.display) {
		sleep(1);
		XSync(ws.display, False);

		XSetForeground(ws.display, ws.gc, 0x00FFFFFF);
		XClearWindow(ws.display, ws.window);

		XFillRectangle(ws.display, ws.window, ws.gc,
			0, 0, ws.width / 2, ws.height / 2);
		XFillRectangle(ws.display, ws.window, ws.gc,
			ws.width / 2, ws.height / 2, ws.width, ws.height);
		XSync(ws.display, False);
	}

	if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs)) {
		BLTS_LOGGED_PERROR("EVIOCGABS(ABS_MT_POSITION_X)");
		ret = -errno;
		goto  cleanup;
	}

	BLTS_TRACE("X limits: %d - %d\n", abs.minimum, abs.maximum);
	if (abs.maximum - abs.minimum == 0) {
		BLTS_ERROR("Invalid X coordinate limits (%d - %d)\n",
			abs.minimum, abs.maximum);
		ret = -EINVAL;
		goto cleanup;
	}
	abs_min_x = abs.minimum;
	if (data->swap_xy) {
		x_scale = (float) data->scr_height /
			(float)(abs.maximum - abs.minimum);
	} else {
		x_scale = (float) data->scr_width /
			(float)(abs.maximum - abs.minimum);
	}

	if (ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs)) {
		BLTS_LOGGED_PERROR("EVIOCGABS(ABS_MT_POSITION_Y)");
		ret = -errno;
		goto  cleanup;
	}
	BLTS_TRACE("Y limits: %d - %d\n", abs.minimum, abs.maximum);
	if (abs.maximum - abs.minimum == 0) {
		BLTS_ERROR("Invalid Y coordinate limits (%d - %d)\n",
			abs.minimum, abs.maximum);
		ret = -EINVAL;
		goto cleanup;
	}
	abs_min_y = abs.minimum;
	if (data->swap_xy) {
		y_scale = (float) data->scr_width /
			(float)(abs.maximum - abs.minimum);
	} else {
		y_scale = (float) data->scr_height /
			(float)(abs.maximum - abs.minimum);
	}

	BLTS_DEBUG("Touch upper left and lower right corners (white boxes) "\
		"simultaneously within 30 seconds\n");

	timing_start();

	memset(tps, 0, sizeof(tps));
	num_tps = got_x = got_y = current_tp = 0;
	x = y = -1;

	/*
	Event sequences depend on driver, for example drivers with no TP
	tracking;
	ABS_MT_POSITION_X
	ABS_MT_POSITION_Y
	SYN_MT_REPORT
	ABS_MT_POSITION_X
	ABS_MT_POSITION_Y
	SYN_MT_REPORT
	SYN_REPORT

	And drivers with tracking;
	ABS_MT_POSITION_X
	ABS_MT_POSITION_Y
	ABS_MT_TRACKING_ID (0)
	SYN_MT_REPORT
	ABS_MT_POSITION_X
	ABS_MT_POSITION_Y
	ABS_MT_TRACKING_ID (1)
	SYN_MT_REPORT
	SYN_REPORT
	*/

	while (timing_elapsed() < 30) {
		rb = read_input_events(fd, ev, sizeof(struct input_event) * 64);
		if (!rb)
			continue;
		else if (rb < 0) {
			ret = rb;
			goto cleanup;
		}

		for (t = 0; t < (int) (rb / sizeof(struct input_event)); t++) {
			if (EV_KEY == ev[t].type)
				BLTS_TRACE("%ld.%06ld ", ev[t].time.tv_sec,
					ev[t].time.tv_usec);

			BLTS_TRACE("type 0x%x code 0x%x value %d\n", ev[t].type,
				ev[t].code, ev[t].value);

			if (ev[t].type == EV_ABS &&
				ev[t].code == ABS_MT_POSITION_X) {
				x = ev[t].value;
				got_x = 1;
			} else if(ev[t].type == EV_ABS &&
				ev[t].code == ABS_MT_POSITION_Y) {
				y = ev[t].value;
				got_y = 1;
			} else if(ev[t].type == EV_SYN &&
				ev[t].code == SYN_MT_REPORT) {
				if (got_x && got_y) {
					if (data->swap_xy) {
						t = y;
						y = (float)(x - abs_min_x) * x_scale;
						x = (float)(t - abs_min_y) * y_scale;
					} else {
						x = (float)(x - abs_min_x) * x_scale;
						y = (float)(y - abs_min_y) * y_scale;
					}
					tps[current_tp].x = x;
					tps[current_tp].y = y;
					tps[current_tp].id = current_tp;
					current_tp++;
					got_x = got_y = 0;
				}
			} else if(ev[t].type == EV_SYN &&
				ev[t].code == SYN_REPORT) {
				num_tps = current_tp;
				current_tp = 0;
			}
		}

		if (num_tps >= 2)
			break;
	}

	timing_stop();

	if (num_tps < 2) {
		ret = -1;
		goto cleanup;
	}

	ret = -2;
	for (t = 0; t < num_tps; t++) {
		if (tps[t].x > 0 &&
			tps[t].y > 0 &&
			tps[t].x < (int)data->scr_width / 2 &&
			tps[t].y < (int)data->scr_height / 2) {
			BLTS_DEBUG("Touchpoint %d is inside upper left area (x: %d, y: %d)\n",
				tps[t].id, x, y);
			ret++;
		} else if (tps[t].x > (int)data->scr_width / 2 &&
			tps[t].y > (int)data->scr_height / 2 &&
			tps[t].x < (int)data->scr_width &&
			tps[t].y < (int)data->scr_height) {
			BLTS_DEBUG("Touchpoint %d is inside lower right area (x: %d, y: %d)\n",
				tps[t].id, x, y);
			ret++;
		} else {
			BLTS_DEBUG("Touchpoint %d is outside both areas (x: %d, y: %d)\n",
				tps[t].id, x, y);
		}
	}

cleanup:
	input_close_window(&ws);

	close(fd);

	return ret;
}

