/* blts_sync.c -- Multiclient test sync

   Copyright (C) 2000-2011, Nokia Corporation.

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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "blts_sync.h"
#include "blts_log.h"

#define SYNC_SHM_NAME "/blts_sync_shared"

#define SYNC_TAG_LEN 16
#define SYNC_TAG_COUNT 64

/** Data for each barrier. */
struct tagged_barrier_entry {
	sem_t mutex;                   /**< Mutex for this structure (only) */

	unsigned refcount;             /**< Number of registered users */
	unsigned active;               /**< 1/more users waiting on this barrier */

	sem_t barrier_arrive_signal;   /**< Collects arriving processes */
	sem_t barrier_depart_signal;   /**< Completion broadcast */

	char tag[SYNC_TAG_LEN];        /**< Name for this barrier */
};

/** This will live in /dev/shm/SYNC_SHM_NAME */
struct shared_data {
	sem_t sync_mutex;              /**< Mutex for this structure */
	sem_t completion_signal;       /**< Final cleanup signaling */

	struct tagged_barrier_entry barriers[SYNC_TAG_COUNT]; /**< Barriers; 0 = anon, others as reserved by clients. */

} __attribute__((packed));

static struct shared_data *mapped_shm;

static int shm_fd = -1;
static pid_t pid;
static unsigned init_done;   /**< Flag: process has sync set up (0 -> API calls are no-ops) */



static void sync_mutex_lock()
{
	int err /*, curr */;
	/* sem_getvalue(&mapped_shm->sync_mutex, &curr); */
	/* BLTS_TRACE("[%d] <mutex sem pre_lock = %d\n",pid,curr); */
	err = sem_wait(&mapped_shm->sync_mutex);

	if(err && errno != EINTR) {
		BLTS_LOGGED_PERROR("Sync: ERROR while locking master semaphor");
	}
	/* sem_getvalue(&mapped_shm->sync_mutex, &curr); */
	/* BLTS_TRACE("[%d] >mutex sem post_lock -> %d\n",pid,curr); */
}

static void sync_mutex_unlock()
{
	int err /*, curr*/;
	/* sem_getvalue(&mapped_shm->sync_mutex, &curr); */
	/* BLTS_TRACE("[%d] <mutex sem pre_unlock = %d\n",pid,curr); */
	err = sem_post(&mapped_shm->sync_mutex);

	if(err) {
		BLTS_LOGGED_PERROR("Sync: ERROR while unlocking master semaphor");
	}
	/* sem_getvalue(&mapped_shm->sync_mutex, &curr); */
	/* BLTS_TRACE("[%d] >mutex sem post_unlock -> %d\n",pid,curr); */
}

static void barrier_mutex_lock(struct tagged_barrier_entry *b)
{
	int err;
	err = sem_wait(&b->mutex);
	if(err && errno != EINTR) {
		BLTS_LOGGED_PERROR("Sync: ERROR while locking barrier mutex");
	}
}
static void barrier_mutex_unlock(struct tagged_barrier_entry *b)
{
	int err;
	err = sem_post(&b->mutex);
	if(err) {
		BLTS_LOGGED_PERROR("Sync: ERROR while unlocking barrier mutex");
	}
}

static int barrier_init(unsigned index, unsigned client_count, char *tag)
{
	int err;
	struct tagged_barrier_entry *b;
	BLTS_TRACE("[%d] Sync: barrier_init()\n", pid);
	b = &mapped_shm->barriers[index];
	if(b->refcount) {
		BLTS_ERROR("Sync: ERROR: Attempted to init barrier with refcount == 0\n");
		return -1;
	}

	memset(b, 0, sizeof(*b));

	err = sem_init(&b->mutex, 1, 0);
	if(err) {
		BLTS_LOGGED_PERROR("Sync: ERROR: Can't init barrier mutex");
		goto err_out;
	}
	err = sem_init(&b->barrier_arrive_signal, 1, 0);
	if(err) {
		BLTS_LOGGED_PERROR("Sync: ERROR: Can't init barrier arrival semaphore");
		goto err_out;
	}
	err = sem_init(&b->barrier_depart_signal, 1, 0);
	if(err) {
		BLTS_LOGGED_PERROR("Sync: ERROR: Can't init barrier departure semaphore");
		goto err_out;
	}

	b->active = 0;
	b->refcount = client_count;

	strncpy(b->tag, tag, SYNC_TAG_LEN);
	barrier_mutex_unlock(b);
	return 0;

err_out:
	b->refcount = 0;
	return err;
}

/** Block until all referrers reach barrier. Call locked! */
static void barrier_sync(unsigned barrier_index)
{
	int i;
	struct tagged_barrier_entry *b;

	b = &mapped_shm->barriers[barrier_index];
	BLTS_TRACE("[%d] >>> arrived at barrier\n", pid);
	barrier_mutex_lock(b);

	if(!b->active) {
		b->active = 1;
		i = b->refcount - 1;
		barrier_mutex_unlock(b);
		sync_mutex_unlock();

		while(i--) {
			/* BLTS_TRACE("[%d] barrier: 1st waiting others - %d\n",pid, i); */
			sem_wait(&b->barrier_arrive_signal);
		}
		barrier_mutex_lock(b);
		i = b->refcount - 1;
		while(i--) {
			/* BLTS_TRACE("[%d] barrier: 1st signalling others - %d\n",pid, i); */
			sem_post(&b->barrier_depart_signal);

		}
		b->active = 0;
		barrier_mutex_unlock(b);
		/* BLTS_TRACE("[%d] barrier: 1st done\n",pid); */
	} else {
		barrier_mutex_unlock(b);
		sync_mutex_unlock();
		/* BLTS_TRACE("[%d] barrier: other signalling 1st\n", pid); */
		sem_post(&b->barrier_arrive_signal);

		/* BLTS_TRACE("[%d] barrier: other waiting signal\n",pid); */
		sem_wait(&b->barrier_depart_signal);

		/* BLTS_TRACE("[%d] barrier: other done\n",pid); */
	}
	BLTS_TRACE("[%d] <<< leaving barrier\n", pid);
	sync_mutex_lock();
}

/** Return index of barrier with given tag; <0 if none. Call locked. */
static int find_tagged_barrier(char *tag)
{
	int i, len;
	len = strlen(tag);
	if(len > SYNC_TAG_LEN)
		len = SYNC_TAG_LEN;
	for(i = 0; i < SYNC_TAG_COUNT; ++i)
		if(mapped_shm->barriers[i].refcount &&
		   !strncmp(mapped_shm->barriers[i].tag, tag, len))
			return i;
	return -1;
}

/**
 * Insert new tagged barrier, or increment refcount of existing one.
 * return index to it or <0 if failed. Call locked.
 */
static int tagged_barrier_ref(char *tag)
{
	int i, err;
	i = find_tagged_barrier(tag);
	if(i >= 0) {
		barrier_mutex_lock(&mapped_shm->barriers[i]);
		mapped_shm->barriers[i].refcount++;
		BLTS_TRACE("Sync: '%s'.refcount == %u\n", tag, mapped_shm->barriers[i].refcount);
		barrier_mutex_unlock(&mapped_shm->barriers[i]);
		return i;
	}
	for(i=0; i < SYNC_TAG_COUNT; ++i)
		if(!mapped_shm->barriers[i].refcount) {
			BLTS_TRACE("Sync: '%s'.refcount == 1 (creating)\n", tag);
			err = barrier_init(i, 1, tag);
			if(err)
				return err;
			return i;
		}
	BLTS_ERROR("Sync: Tag table full, cannot add another.\n");
	return -ENOSPC;
}

/**
 * Decrement refcount of tagged barrier, free it if if it reaches zero. Call locked.
 */
static void tagged_barrier_unref(char *tag)
{
	int i;
	struct tagged_barrier_entry *b;
	i = find_tagged_barrier(tag);
	if(i < 0)
		return;
	b = &mapped_shm->barriers[i];

	if(!--b->refcount) {
		BLTS_TRACE("Sync: Destroying '%s'.\n", tag);
		sem_destroy(&b->mutex);
		sem_destroy(&b->barrier_arrive_signal);
		sem_destroy(&b->barrier_depart_signal);
		memset(b, 0, sizeof(*b));
	}
	BLTS_TRACE("Sync: '%s'.refcount == %u\n", tag, mapped_shm->barriers[i].refcount);
}

/**
 * Register test process as user of named barrier. This must be used before
 * calling blts_sync_tagged().
 * @param tag Name to register
 * @return 0 on success
 */
int blts_sync_add_tag(char *tag)
{
	int ret;
	if(!init_done)
		return 0;

	sync_mutex_lock();
	ret = tagged_barrier_ref(tag);
	sync_mutex_unlock();
	return ret;
}

/**
 * Remove a test process' registration of the named barrier.
 * @param tag Name to deregister
 * @return 0 on success
 */
int blts_sync_del_tag(char *tag)
{
	if(!init_done)
		return 0;
	sync_mutex_lock();
	tagged_barrier_unref(tag);
	sync_mutex_unlock();
	return 0;
}

/**
 * Block process until each registered user reaches named barrier. Note that
 * the participating processes must take care to have their sync_*() calls
 * in the same order to avoid likely deadlock.
 * @param tag Name of barrier, registered earlier with blts_sync_add_tag()
 * @return 0 on success
 */
int blts_sync_tagged(char *tag)
{
	int i, ret = 0;
	if(!init_done)
		return 0;
	sync_mutex_lock();
	i = find_tagged_barrier(tag);
	if(i < 0) {
		BLTS_ERROR("Sync: Named barrier '%s' not available\n", tag);
		ret = -EINVAL;
	} else {
		BLTS_TRACE("Sync: Arriving at named barrier '%s'\n", tag);
		barrier_sync(i);
	}
	sync_mutex_unlock();
	return ret;
}

/** Not implemented */
int blts_sync_tagged_to(__attribute__((unused)) char *tag, __attribute__((unused)) unsigned long msec)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

/** Not implemented */
int blts_sync_anon_to(__attribute__((unused)) unsigned long msec)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

/**
 * Default barrier. This will block until all participating processes reach
 * this point. Note that the participating processes must take care to have
 * their sync_*() calls in the same order to avoid likely deadlock.
 * @return 0 (always)
 */
int blts_sync_anon()
{
	if(!init_done)
		return 0;

	sync_mutex_lock();
	barrier_sync(0);
	sync_mutex_unlock();
	return 0;
}

static int setup_shm_master()
{
	int fd, err;
	void *map;

	if(shm_fd >= 0 || mapped_shm)
		return 0;

	BLTS_TRACE("master:start shm setup\n");
	fd = shm_open(SYNC_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);

	if(fd < 0) {
		BLTS_LOGGED_PERROR("Sync: Failed creating shared memory area");
		return -errno;
	}
	shm_fd = fd;

	err = ftruncate(fd, sizeof(struct shared_data));
	if(err) {
		BLTS_LOGGED_PERROR("Sync: ftruncate() of shared memory area failed");
		goto err_done;
	}

	BLTS_TRACE("master:mmap()\n");
	map = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
		   MAP_SHARED, shm_fd, 0);

	if(!map) {
		BLTS_LOGGED_PERROR("Sync: cannot mmap shared memory");
		err = -errno;
		goto err_done;
	}

	close(shm_fd);
	shm_fd = -1;
	mapped_shm = map;
	memset(mapped_shm, 0, sizeof(*mapped_shm));
	BLTS_TRACE("master:shm setup done\n");
	init_done = 1;
	return 0;

err_done:

	if(mapped_shm) {
		BLTS_TRACE("master:error path munmap()\n");
		munmap(mapped_shm, sizeof(struct shared_data));
		mapped_shm = NULL;
	}
	if(shm_fd >= 0) {
		close(shm_fd);
		shm_fd = -1;
	}
	return err;
}

static int attach_shm_client()
{
	int fd, err;
	void *map;

	if(shm_fd >= 0 || mapped_shm)
		return 0;
	BLTS_TRACE("client:shm attach start\n");

	fd = shm_open(SYNC_SHM_NAME, O_RDWR, 0666);

	if(fd < 0) {
		BLTS_LOGGED_PERROR("Sync: Failed opening shared memory area");
		return -errno;
	}
	shm_fd = fd;

	BLTS_TRACE("client:mmap\n");

	map = mmap(NULL, sizeof(struct shared_data), PROT_READ | PROT_WRITE,
		   MAP_SHARED, shm_fd, 0);

	if(!map) {
		BLTS_LOGGED_PERROR("Sync: cannot mmap shared memory");
		err = -errno;
		goto err_done;
	}

	close(shm_fd);
	shm_fd = -1;
	mapped_shm = map;
	BLTS_TRACE("client:shm attach done\n");
	init_done = 1;
	return 0;

err_done:

	if(mapped_shm) {
		BLTS_TRACE("client:error path munmap\n");
		munmap(mapped_shm, sizeof(struct shared_data));
		mapped_shm = NULL;
	}
	if(shm_fd >= 0) {
		close(shm_fd);
		shm_fd = -1;
	}
	return err;
}

/**
 * Prepare shared state and launch a daemon to clean up afterwards.  Don't
 * call this unless not using the common CLI.
 * @param client_count Number of processes to track
 * @return 0 on success.
 */
int blts_sync_master_init(unsigned client_count)
{
	int err, devnull;
	unsigned i;
	pid_t p;

	/* Detach and daemonize */
	p = fork();
	if(p < 0) {
		BLTS_LOGGED_PERROR("Sync: Cannot fork master");
		err = -errno;
		goto done;
	} else if(p > 0) { /* parent */
		err = 0;
		goto done;
	}
	setpgid(0, 0);
	p = fork();
	if(p < 0) {
		fprintf(stderr, "Sync: Cannot detach - %s\n", strerror(errno));
		exit(1);
	} else if(p > 0) { /* child of first fork() */
		_exit(0);
	}
	umask(0077);
	if(chdir("/") < 0) exit(1);
	devnull = open("/dev/null", O_RDWR);
	dup2(devnull, 0);
	dup2(devnull, 1);
	dup2(devnull, 2);
	for(i = sysconf(_SC_OPEN_MAX)-1; i >= 3; --i)
		close(i);


	/* Ok, now the actual setup. */

	pid = getpid();
	err = setup_shm_master();
	if(err)
		goto daemon_done;

	/* Start locked */
	/* BLTS_TRACE("master: sem_init\n"); */

	err = sem_init(&mapped_shm->sync_mutex, 1, 0);
	if(err) {
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init master mutex"); */
		goto daemon_done;
	}
	err = sem_init(&mapped_shm->completion_signal, 1, 0);
	if(err) {
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init completion semaphore"); */
		goto daemon_done;
	}

	err = barrier_init(0, client_count, "default");
	if(err)
		goto daemon_done;

	sync_mutex_unlock();

	/* BLTS_TRACE("master: waiting for clients\n"); */
	for(i = 0; i < client_count; ++i) {
		sem_wait(&mapped_shm->completion_signal);
	}
	/* BLTS_TRACE("master: %d clients signaled\n", client_count); */

	/* final cleanup */

	sync_mutex_lock();
	for(i = 0; i < SYNC_TAG_COUNT; ++i) {
		if(&mapped_shm->barriers[i].refcount) {
			sem_destroy(&mapped_shm->barriers[i].mutex);
			sem_destroy(&mapped_shm->barriers[i].barrier_arrive_signal);
			sem_destroy(&mapped_shm->barriers[i].barrier_depart_signal);
		}
	}
	sem_destroy(&mapped_shm->completion_signal);
	sync_mutex_unlock();
	sem_destroy(&mapped_shm->sync_mutex);
daemon_done:
	if(mapped_shm)
		munmap(mapped_shm, sizeof(struct shared_data));
	mapped_shm = NULL;
	shm_unlink(SYNC_SHM_NAME);
	exit(1);

done:
	return err;
}

/**
 * Set up shared state for a test process. Don't call this unless not using the common CLI.
 * @return 0 on success
 */
int blts_sync_client_init()
{
	int err;
	pid = getpid();
	err = attach_shm_client();
	if(err)
		goto done;

	BLTS_TRACE("[%d] client: init done\n", pid);

done:
	return err;
}

/**
 * Clean up shared state after a test process. Don't call this unless not
 * using the common CLI.
 */
void blts_sync_client_cleanup()
{
	if(mapped_shm) {
		sem_post(&mapped_shm->completion_signal);
		munmap(mapped_shm, sizeof(struct shared_data));
		mapped_shm = NULL;
	}
}

