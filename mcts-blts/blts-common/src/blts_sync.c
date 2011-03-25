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

#include "blts_sync.h"
#include "blts_log.h"

#define SYNC_SHM_NAME "/blts_sync_shared"


/** This will live in /dev/shm/SYNC_SHM_NAME */
struct shared_data {
	sem_t sync_mutex;              /**< Mutex for this structure */
	sem_t barrier_signal;          /**< Barrier arrival */
	sem_t barrier_continue_signal; /**< Barrier departure */
	unsigned barrier_active;       /**< Flag: first process to reach barrier sets & manages */
	unsigned client_count;         /**< Set in -syncprep, needed by barrier */
	sem_t completion_signal;       /**< Final cleanup signaling */
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

static int sync_mutex_trylock()
{
	return -1;
}

static int sync_mutex_timedlock(long msec)
{
	return -1;
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

/* static unsigned tag_count(__attribute__((unused)) char *tag) */
/* { */
/* 	return 0; */
/* } */

int blts_sync_add_tag(__attribute__((unused)) char *tag)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

int blts_sync_del_tag(__attribute__((unused)) char *tag)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

int blts_sync_tagged(__attribute__((unused)) char *tag)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

int blts_sync_tagged_to(__attribute__((unused)) char *tag, __attribute__((unused)) unsigned long msec)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

int blts_sync_anon_to(unsigned long msec)
{
	if(!init_done)
		return 0;
	BLTS_DEBUG("WARNING: %s does not do anything yet\n", __FUNCTION__);
	return 0;
}

/**
 * Basic barrier.
 * @return 0
 */
int blts_sync_anon()
{
	unsigned i;
	if(!init_done)
		return 0;
	BLTS_TRACE("[%d] >>> arrived at barrier\n", pid);
	sync_mutex_lock();
	if(!mapped_shm->barrier_active) {
		mapped_shm->barrier_active = 1;
		i = mapped_shm->client_count - 1;
		sync_mutex_unlock();
		while(i--) {
			/* BLTS_TRACE("[%d] barrier: 1st waiting others - %d\n",pid, i); */
			sem_wait(&mapped_shm->barrier_signal);
		}
		sync_mutex_lock();
		i = mapped_shm->client_count - 1;
		while(i--) {
			/* BLTS_TRACE("[%d] barrier: 1st signalling others - %d\n",pid, i); */
			sem_post(&mapped_shm->barrier_continue_signal);

		}
		i = mapped_shm->barrier_active = 0;
		sync_mutex_unlock();
		/* BLTS_TRACE("[%d] barrier: 1st done\n",pid); */
	} else {
		sync_mutex_unlock();
		/* BLTS_TRACE("[%d] barrier: other signalling 1st\n", pid); */
		sem_post(&mapped_shm->barrier_signal);

		/* BLTS_TRACE("[%d] barrier: other waiting signal\n",pid); */
		sem_wait(&mapped_shm->barrier_continue_signal);

		/* BLTS_TRACE("[%d] barrier: other done\n",pid); */
	}
	BLTS_TRACE("[%d] <<< leaving barrier\n", pid);

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
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init sync semaphore"); */
		goto daemon_done;
	}
	err = sem_init(&mapped_shm->completion_signal, 1, 0);
	if(err) {
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init completion semaphore"); */
		goto daemon_done;
	}
	err = sem_init(&mapped_shm->barrier_signal, 1, 0);
	if(err) {
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init barrier semaphore"); */
		goto daemon_done;
	}
	err = sem_init(&mapped_shm->barrier_continue_signal, 1, 0);
	if(err) {
		/* BLTS_LOGGED_PERROR("Sync: ERROR: Can't init barrier semaphore"); */
		goto daemon_done;
	}

	mapped_shm->client_count = client_count;
	mapped_shm->barrier_active = 0;

	sync_mutex_unlock();

	/* BLTS_TRACE("master: waiting for clients\n"); */
	for(i = 0; i < client_count; ++i) {
		sem_wait(&mapped_shm->completion_signal);
	}
	/* BLTS_TRACE("master: %d clients signaled\n", client_count); */

daemon_done:
	if(mapped_shm)
		munmap(mapped_shm, sizeof(struct shared_data));
	mapped_shm = NULL;
	shm_unlink(SYNC_SHM_NAME);
	exit(1);

done:
	return err;
}

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

void blts_sync_client_cleanup()
{
	if(mapped_shm)
		sem_post(&mapped_shm->completion_signal);
		munmap(mapped_shm, sizeof(struct shared_data));
	mapped_shm = NULL;
}

