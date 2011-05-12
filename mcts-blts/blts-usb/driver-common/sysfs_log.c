/*
 * sysfs_log.c -- BLTS sysfs logging functionality
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

#include <linux/kernel.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 28) || !defined (CONFIG_RING_BUFFER)

int sysfs_log_add(void)
{
	printk(KERN_INFO "sysfs logging disabled (ring buffer not available)\n");
	return 0;
}

void sysfs_log_remove(void)
{
}

int sysfs_log_write(const char *fmt, ...)
{
	return 0;
}

#else

#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/ring_buffer.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/cpumask.h>

#define RING_BUF_MAX_SIZE 32768
#define TRACE_BUF_SIZE 128

static struct ring_buffer *buffer;
static struct kobject *log_kobj;
static struct workqueue_struct *log_workqueue;
static struct delayed_work log_work;

struct sysfs_log_entry {
	int len;
	char buf[TRACE_BUF_SIZE];
};

static struct ring_buffer_event *get_next_rb_event(int *event_cpu)
{
	int cpu, next_cpu = -1;
	u64 ts, next_ts = 0;
	struct ring_buffer_event *event;
	struct ring_buffer_event *next_event = NULL;

	for_each_present_cpu(cpu) {
		if (ring_buffer_empty_cpu(buffer, cpu))
			continue;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
		event = ring_buffer_peek(buffer, cpu, &ts, NULL);
#else
		event = ring_buffer_peek(buffer, cpu, &ts);
#endif
		if (event && (!next_event || ts < next_ts)) {
			next_ts = ts;
			next_cpu = cpu;
			next_event = event;
		}
	}

	if (next_event)
		*event_cpu = next_cpu;

	return next_event;
}

static ssize_t sysfs_log_show(struct kobject *kobj,
	struct kobj_attribute *attr, char *buf)
{
	int cpu;
	ssize_t len = 0;
	struct ring_buffer_event *event = NULL;
	struct sysfs_log_entry *entry = NULL;

	while ((event = get_next_rb_event(&cpu))) {
		entry = ring_buffer_event_data(event);
		if (!entry)
			break;
		if (entry->len + len >= PAGE_SIZE) {
			sysfs_notify(log_kobj, NULL, "log");
			break;
		}
		memcpy(&buf[len], entry->buf, entry->len);
		len += entry->len;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
		ring_buffer_consume(buffer, cpu, NULL, NULL);
#else
		ring_buffer_consume(buffer, cpu, NULL);
#endif
	}

	return len;
}

static ssize_t sysfs_log_store(struct kobject *kobj,
	struct kobj_attribute *attr, const char *buf, size_t count)
{
	/* TODO: Set loglevel functionality */
	return -EINVAL;
}

static struct kobj_attribute log_attribute =
	__ATTR(log, 0666, sysfs_log_show, sysfs_log_store);

static struct attribute *attrs[] = {
	&log_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static void log_notify(struct work_struct *work)
{
	sysfs_notify(log_kobj, NULL, "log");
}

int sysfs_log_add(void)
{
	int retval;

	buffer = ring_buffer_alloc(RING_BUF_MAX_SIZE, RB_FL_OVERWRITE);
	if (!buffer)
		return -ENOMEM;

	log_kobj = kobject_create_and_add("logging", &THIS_MODULE->mkobj.kobj);
	if (!log_kobj) {
		retval = -ENOMEM;
		goto error_exit;
	}

	retval = sysfs_create_group(log_kobj, &attr_group);
	if (retval)
		goto error_exit;

	log_workqueue = create_singlethread_workqueue("sysfs_log_queue");
	if (!log_workqueue) {
		retval = -ENOMEM;
		goto error_exit;
	}
	INIT_DELAYED_WORK(&log_work, log_notify);

	return retval;

error_exit:
	if (log_kobj)
		kobject_put(log_kobj);
	if (buffer)
		ring_buffer_free(buffer);
	return retval;
}

void sysfs_log_remove(void)
{
	if (log_workqueue) {
		cancel_delayed_work(&log_work);
		flush_workqueue(log_workqueue);
		destroy_workqueue(log_workqueue);
	}

	if (log_kobj)
		kobject_put(log_kobj);

	if (buffer) {
		ring_buffer_free(buffer);
		buffer = NULL;
	}
}

int sysfs_log_write(const char *fmt, ...)
{
	struct ring_buffer_event *event = NULL;
	struct sysfs_log_entry *entry = NULL;
	va_list args;
	int retval;
	unsigned msec = jiffies_to_msecs(jiffies);

	if (!buffer)
		return -EFAULT;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
	event = ring_buffer_lock_reserve(buffer, sizeof(*entry), NULL);
#else
	event = ring_buffer_lock_reserve(buffer, sizeof(*entry));
#endif
	if (!event)
		return -ENOMEM;

	entry = ring_buffer_event_data(event);

	sprintf(entry->buf, "[ %d.%03d ] ", msec / 1000, msec % 1000);
	entry->len = strlen(entry->buf);

	va_start(args, fmt);
	entry->len += vsnprintf(&entry->buf[entry->len],
		TRACE_BUF_SIZE - entry->len - 1, fmt, args) + 1;
	va_end(args);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 30)
	retval = ring_buffer_unlock_commit(buffer, event, NULL);
#else
	retval = ring_buffer_unlock_commit(buffer, event);
#endif

	queue_delayed_work(log_workqueue, &log_work, 5);

	return retval;
}

#endif /* kernel < 2.6.28 || !CONFIG_RING_BUFFER */
