/* alsa_async.c -- Asynchronous IO handling

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
#include <signal.h>
#include <stdlib.h>
#include <blts_log.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/syscall.h>

#include "alsa_async.h"
#include "alsa_util.h"

#define ASYNC_USES_RTSIGNAL

struct async_handler {
	async_callback callback;
	pcm_device *hw;
	int fd;
	void *data;
};

static int async_signo;
static struct slist *async_handlers = 0;

extern int __libc_allocate_rtsig (int high);
static void async_init()
{
	static int called = 0;
	long have_rtsig;
	if (called)
		return;

	FUNC_ENTER();
	if (async_handlers)
		return;

	called++;

	have_rtsig = 0;
	async_signo = SIGIO;

#ifdef ASYNC_USES_RTSIGNAL
#ifdef _POSIX_REALTIME_SIGNALS
	have_rtsig = sysconf(_SC_REALTIME_SIGNALS);
	if (SIGRTMIN + 4 > SIGRTMAX)
		have_rtsig = 0;
	if (have_rtsig > 0) {
		async_signo = __libc_allocate_rtsig(0);
		BLTS_TRACE("Using realtime signal (%d)\n", async_signo);
	} else {
		BLTS_TRACE("Using SIGIO (can't determine if realtime supported)\n");
	}
#endif
#endif
}

/* Generic dispatcher for AIO; calls any callbacks associated with the fd that
   the signal was for. */
static void async_dispatch(__attribute__((unused)) int signo,
	siginfo_t *siginfo, __attribute__((unused)) void *context)
{
	struct slist *head;
	struct async_handler *handler;

	if (!async_handlers)
		return;
	/* BLTS_TRACE("Signal for fd %d\n",siginfo->si_fd); */

	head = async_handlers;
	while (head) {
		handler = (struct async_handler *) head->item;
		if (!handler || !(handler->callback) || !(handler->hw))
			continue;
		if (siginfo->si_fd != handler->fd)
			BLTS_WARNING("Signal for invalid fd !!! \n");
		handler->callback(handler->hw, handler->data);
		head = head->next;
	}
}

/* Sets the AIO dispatcher as signal handler. 0 = ok. */
static int set_signal_cb()
{
	struct sigaction sa;
	int ret;
	FUNC_ENTER();

	memset(&sa, 0, sizeof sa);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sa.sa_sigaction = async_dispatch;
	sigemptyset(&sa.sa_mask);

	ret = sigaction(async_signo, &sa, 0);
	if (ret) {
		BLTS_ERROR("Failed to set signal handler - \n", strerror(errno));
		return ret;
	}
	return 0;
}

/* Reset default signal handler */
static int unset_signal_cb()
{
	struct sigaction sa;
	int ret;
	FUNC_ENTER();

	if (async_handlers)
		BLTS_WARNING("%s : handler list not empty while removing dispatch\n");

	memset(&sa, 0, sizeof sa);
	sa.sa_handler = SIG_DFL;

	ret = sigaction(async_signo, &sa, 0);
	if (ret) {
		BLTS_ERROR("Failed to clear signal handler - \n", strerror(errno));
		return ret;
	}
	return 0;
}

static void dump_handlers()
{
	struct slist *head = async_handlers;
	struct async_handler *handler;
/* 	BLTS_TRACE("Handlers:\n"); */

	while(head) {
		handler = head->item;
/* 		BLTS_TRACE("  cb=%p, hw=%p, data=%p\n",handler->callback, handler->hw, handler->data); */
		head=head->next;
	}
}

/* helper for cleanups */
static void free_handler(void *p)
{
	if (!p)
		return;
	struct async_handler * handler = (struct async_handler *) p;
	free(handler);
}

/* Add new AIO handler to the list to be called on io signal. */
int async_handler_add(async_callback callback, pcm_device *hw, int trigger_fd,
	void *data)
{
	struct async_handler *handler;
	int ret;
	FUNC_ENTER();

	if (!callback || !hw || trigger_fd < 0) {
		BLTS_ERROR("%s: invalid argument (this is a bug)\n", __FUNCTION__);
		return -EINVAL;
	}
	async_init();

	handler = malloc(sizeof *handler);
	if (!handler)
		return -ENOMEM;
	memset(handler, 0, sizeof *handler);

	handler->callback = callback;
	handler->hw = hw;
	handler->data = data;
	handler->fd = trigger_fd;

	if (!async_handlers) {
		ret = set_signal_cb();
		if (ret)
			return ret;
	}

	async_handlers = slist_prepend(async_handlers, handler);

 	dump_handlers();

	return 0;
}

/* Delete AIO handler added with matching arguments */
int async_handler_del(async_callback callback, pcm_device *hw, int trigger_fd,
	void *data)
{
	struct slist *head, *prev;
	struct async_handler *handler;
	int found = 0, ret;
	FUNC_ENTER();

	prev = 0;
	head = async_handlers;
	while (head) {
		if (!head->item) {
			prev = head;
			head = head->next;
			continue;
		}

		handler = (struct async_handler *) head->item;
		if (handler->callback == callback
			&& handler->hw == hw && handler->data==data
			&& handler->fd == trigger_fd) {
			found++;
			head = slist_delete_head(head, free_handler);
			if (!prev)
				async_handlers = head;
		} else {
			prev = head;
			head = head->next;
		}
	}

	ret = found ? 0 : -1;

	if (!async_handlers)
		ret |= unset_signal_cb();

	return ret;
}

/* Clear all AIO handlers. */
int async_handler_clear()
{
	int ret;
	FUNC_ENTER();
	if (!async_handlers)
		return 0;

	while ((async_handlers = slist_delete_head(async_handlers, free_handler)));

	ret = unset_signal_cb();

	return ret;
}



/* Toggle AIO flags / signals for opened device.
   REVISIT: multithreading (man 2 fcntl) */
static int set_async_io(int fd, int enable)
{
	long flags;
	int ret;
	FUNC_ENTER();

	if (fd < 0)
		return -EINVAL;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		ret = -errno;
		BLTS_ERROR("%s: Cannot F_GETFL - %s\n", __FUNCTION__, strerror(-ret));
		return ret;
	}
	if (enable)
		flags |= O_ASYNC | O_NONBLOCK;
	else
		flags &= ~O_ASYNC;
	ret = fcntl(fd, F_SETFL, flags);
	if (ret) {
		ret = -errno;
		BLTS_ERROR("%s: Cannot F_SETFL - %s\n", __FUNCTION__, strerror(-ret));
		return ret;
	}
	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		ret = -errno;
		BLTS_ERROR("%s: Cannot F_GETFL - %s\n", __FUNCTION__, strerror(-ret));
		return ret;
	}
	if (enable && !(flags & O_ASYNC)) {
		ret = -EINVAL;
		BLTS_ERROR("%s: Async mode not set.\n", __FUNCTION__);
		return ret;
	} else	if (!enable && (flags & O_ASYNC)) {
		ret = -EINVAL;
		BLTS_ERROR("%s: Async mode still set.\n", __FUNCTION__);
		return ret;
	}
	ret = fcntl(fd, F_SETSIG, (long) async_signo);
	if (ret) {
		ret = -errno;
		BLTS_ERROR("%s: Cannot change signal - %s\n", __FUNCTION__, strerror(-ret));
		return ret;
	}

	ret = fcntl(fd, F_SETOWN, (long) syscall(SYS_gettid));
	if (ret) {
		ret = -errno;
		BLTS_ERROR("%s: Cannot change signal - %s\n", __FUNCTION__, strerror(-ret));
		return ret;
	}
	return 0;
}

/* Start raising signals on opened device whenever IO is possible.
   Call this when ready to begin IO. */
int async_io_enable(int fd)
{
	FUNC_ENTER();
	return set_async_io(fd, 1);
}

/* Stop signal raising on IO */
int async_io_disable(int fd)
{
	FUNC_ENTER();
	return set_async_io(fd, 0);
}

