/* ext_tools.c -- External tools wrapper

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <blts_log.h>

#include "ext_tools.h"

static pid_t hcidump_pid = 0;
static char *hcidump_executable = "/usr/sbin/hcidump";

static int have_saved_signals = 0;
static sigset_t sigmask_saved;
static sighandler_t sigcld_handler_saved;

static void save_reset_signals()
{
	sigset_t sigmask_clear;
	if (have_saved_signals)
		return;
	sigemptyset(&sigmask_clear);
	sigprocmask(SIG_SETMASK, &sigmask_clear, &sigmask_saved);
	sigcld_handler_saved = signal(SIGCLD, SIG_DFL);
	have_saved_signals = 1;
}

static void restore_saved_signals()
{
	if (!have_saved_signals)
		return;
	sigprocmask(SIG_SETMASK, &sigmask_saved, (void*) 0);
	signal(SIGCLD, sigcld_handler_saved);
	have_saved_signals = 0;
}

/* Check if we have rights to actually run hcidump */
static int hcidump_exec_ok()
{
	if (getuid() != 0)
		return 0;
	if (access(hcidump_executable, X_OK) != 0)
		return 0;

	return 1;
}

/*
 * Start capturing bluetooth packets with hcidump in dump_file,
 * from hci_device (null = default). 0 = start success.
 */
int start_hcidump(char *dump_file, char *hci_device)
{
	pid_t pid;
	if (hcidump_pid) {
		BLTS_DEBUG("hcidump already running.\n");
		return -EALREADY;
	}

	if (!dump_file) {
		BLTS_DEBUG("No dump file specified.\n");
		return -EINVAL;
	}

	if (!hcidump_exec_ok()) {
		BLTS_DEBUG("Can't run hcidump.\n");
		return -EPERM;
	}

	save_reset_signals();

	if ((pid = fork()) < 0) {
		BLTS_DEBUG("Failed to fork - %s", strerror(errno));
		restore_saved_signals();
		return -1;
	}

	if (pid) {
		/* parent */
		hcidump_pid = pid;
		usleep(250000);
		BLTS_DEBUG("hcidump running.\n");
		return 0;
	}

	/* child */
	char* exec_args[] = { hcidump_executable,
			      "-N", "-i", hci_device ? hci_device : "hci0",
			      "-w", dump_file,
			      (void*) 0 };
	char* exec_env[] = { (void*)0 };
	for (int j=0; j<3; ++j)
		close(j);

	if (execve(exec_args[0], exec_args, exec_env) < 0) {
		perror("execve failure()");
	}

	return -1; /* only on execve fail*/
}

/*
 * Stop ongoing hcidump child
 */
int stop_hcidump()
{
	int status = 0;
	if (!hcidump_pid) {
		BLTS_DEBUG("hcidump is not running.\n");
		return -ESRCH;
	}

	if (kill(hcidump_pid, SIGTERM) < 0) {
		BLTS_DEBUG("Could not terminate hcidump.\n");
		return -errno;
	}

	if (waitpid(hcidump_pid, &status, 0) < 0) {
		BLTS_DEBUG("Could wait for hcidump.\n");
		return -errno;
	}

	restore_saved_signals();
	hcidump_pid = 0;

	return 0;
}

/*
 * Parse hex dump and append result to array in output. Dynamically resize buffer given
 * old address and length.
 */
static int append_hex_dump(char* input, uint8_t **output, size_t *output_len)
{
	char *saveptr = 0, *token, *work;
	unsigned val;
	int count,i;
	uint8_t vals[256], *new_output;

	if (!input || !output || !output_len)
		return -EINVAL;

	work = strdup(input);

	count = 0;
	while (count < 256) {
		token = strtok_r(count ? 0 : work, " \n", &saveptr);
		if (!token)
			break;
		if (sscanf(token, "%x", &val) != 1)
			return -EINVAL;
		vals[count++] = val & 0xff;
	}

	free(work);
	new_output = realloc(*output, *output_len + count);
	for (i = 0; i < count ; ++i)
		new_output[*output_len + i] = vals[i];

	*output = new_output;
	*output_len += count;

	return 0;
}


/*
 * Call hcidump to parse given dump, calling trace_handler for each event in the sequence.
 * Format:
 * HCI sniffer - Bluetooth packet analyzer ver 1.42
 * 947121827.449127 > 04 13 05 01 0B 00 02 00
 * 947121827.460418 > 02 0B 20 10 00 0C 00 01 00 0B 01 08 00 02 00 00 00 03 00 00
 *   00
 * 947121827.460510 < 02 0B 20 10 00 0C 00 01 00 03 01 08 00 40 00 40 00 00 00 00
 *   00
 * 947121827.478790 > 04 07 FF 00 F1 A0 C0 6E 1D 00 4A 75 73 73 69 00 00 00 00 00
 *   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 */
int parse_hcidump(char *dump_file, hcidump_trace_handler trace_handler, void *user_data)
{
	int ret = 0, status = 0, current_handled = 1, lineno = 0;
	char packet_dir;
	char *cmd = 0, *buf = 0, *data = 0;
	FILE *dump = 0;
	struct stat sb;
	size_t buf_sz = 0;
	ssize_t read_len = 0;

	struct hcidump_trace current = {0,0,0,0,0};

	if (!dump_file || stat(dump_file,&sb) < 0) {
		BLTS_DEBUG("No dump to parse.\n");
		return -ENOENT;
	}

	if (!hcidump_exec_ok()) {
		BLTS_DEBUG("Can't run hcidump.\n");
		return -EPERM;
	}

	if (asprintf(&cmd, "%s -tR -r %s", hcidump_executable, dump_file) < 0) {
		BLTS_DEBUG("malloc() - %s", strerror(errno));
		return -errno;
	}

	save_reset_signals();

	fflush(stdout);
	dump = popen(cmd, "r");
	if (!dump) {
		BLTS_DEBUG("Failed to execute hcidump.\n");
		ret = errno?-errno:-1;
		goto cleanup;
	}

	while ((read_len = getline(&buf, &buf_sz, dump)) != -1) {
		lineno++;

		char* pch = NULL;
		pch = strchr(buf,'.'); /* search for time stamp nnnn.nnnnnn */

		switch (buf[0]) {
		case 'H': /* header */
			break;
		case ' ': /* data for last packet or new packet with time stamp*/
			if (!pch) { /* data for last packet if '.' not found*/
				if ((buf_sz <= 2) ||
					append_hex_dump(buf+2, &current.data,
					&current.data_len) < 0) {
					BLTS_DEBUG("Parse error on dump line %d\n", lineno);
					ret = -1;
					goto cleanup;
				}
				break;
			}
		case '0':/* new packet */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			/* Previous event now has all data, so handle it */
			if (trace_handler && (!current_handled)) {
				if (trace_handler(&current, user_data)) {
					BLTS_DEBUG("Failure in handler on dump line %d.\n",lineno);
					ret = -1;
					goto cleanup;
				}
				current_handled = 1;
				if (current.data)
					free(current.data);
				memset(&current, 0, sizeof (current));
			}
			/* time.stamp dir hexdump */
			if (sscanf(buf, "%lu.%lu %c %a[^\n]", &current.tv_sec, &current.tv_usec,
				&packet_dir, &data) != 4) {
				BLTS_DEBUG("Parse error on dump line %d\n", lineno);
				ret = -1;
				goto cleanup;
			}
			current.incoming = (packet_dir == '>');
			if (append_hex_dump(data, &current.data, &current.data_len) < 0) {
				BLTS_DEBUG("Parse error on dump line %d\n", lineno);
				ret = -1;
				goto cleanup;
			}
			current_handled = 0;
			break;
		default: /* ??? */
			BLTS_DEBUG("Warning: no parse for line %d\n", lineno);
			break;
		}
	}
	if (!current_handled && trace_handler) {
		if (trace_handler(&current, user_data)) {
			BLTS_DEBUG("Failure in handler on dump line %d.\n",lineno);
			ret = -1;
		}
	}

cleanup:
	if (cmd)
		free(cmd);
	if (buf)
		free(buf);

	if (dump) {
		status = pclose(dump);

		if (status < 0) {
			BLTS_DEBUG("Failed to wait for hcidump\n");
			ret = -1;
		} else if (status > 0) {
			BLTS_DEBUG("Failure in hcidump (%d)\n", status);
			ret = -1;
		}
	}
	restore_saved_signals();

	return ret;
}

