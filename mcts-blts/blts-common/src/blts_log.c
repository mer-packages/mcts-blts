/* log.c -- Logging functionality

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

#include "blts_log.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/klog.h>
#include <signal.h>

static char *logging_level_message[] =
{
		"[TRACE] ",
		"[DEBUG] ",
		"[WARN] ",
		"[ERROR] "
};

static FILE *log_file = 0;
static int log_console = 0;
static unsigned int logging_flags = 0;

static int print_timestamp = 0;
static int logging_level = LEVEL_DEBUG;

static pid_t local_syslog_capture_pid = 0;

/* default path used */
static const char log_path[] = "/var/log/tests";

/* system messages */
static const char syslog_path[] = "/var/log/tests/kmsg.log";

/* general buffer for syslog capture */
static char buffer[4096];

/* Open given log file; < 0 = fail */
int blts_log_open(const char *filename, unsigned int flags)
{
	char *real_filename;
	char *path_and_file;
	int n;

	if (!filename)
		log_file = fopen("/dev/null","a");
	else {
		real_filename = basename(filename);

		if(log_file)
			return -1;
		/* check directory and create if necessary (all rights) */
		if( access(log_path,X_OK) != 0 )
			mkdir(log_path, 0777);

		n = asprintf(&path_and_file, "%s/%s", log_path, real_filename);

		log_file = fopen(path_and_file, "a");
		free(path_and_file);
	}

	if (!log_file) {
		perror("log open");
		return -1;
	}

	/* TODO: Remove later. log_console is used by deprecated funcs. */
	if (flags & BLTS_LOG_FLAG_STDOUT)
		log_console = 1;

	logging_flags = flags;
	return 0;
}

/* Close current log file; < 0 = fail */
int blts_log_close()
{
	int ret = 0;

	if (!log_file)
		return -1;

	if(fclose(log_file) != 0) {
		perror("log close");
		ret = -1;
	}
	log_file = 0;
	return ret;
}

void blts_log_set_level(int level)
{
	logging_level = level;
}

int blts_log_get_level()
{
	return logging_level;
}

/* printf to current log file (std args, return vals) */
void blts_log_print_level(int level, const char *format, ...)
{
	va_list ap;
	time_t t;
	struct tm *tm_s;

	if(!log_file)
		return;

	if (level > 5)
		level = 5;
	else if (level < 0)
		level = 0;

	if (level < logging_level)
		return;

	if (logging_flags & BLTS_LOG_FLAG_TS) {
		t = time(NULL);
		tm_s = localtime(&t);
		if (logging_flags & BLTS_LOG_FLAG_FILE) {
			fprintf(log_file, "[%02d.%02d.%04d %02d:%02d:%02d] ",
				tm_s->tm_mday, tm_s->tm_mon + 1, tm_s->tm_year + 1900,
				tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
		}

		if (logging_flags & BLTS_LOG_FLAG_STDOUT) {
			printf("[%02d.%02d.%04d %02d:%02d:%02d] ",
				tm_s->tm_mday, tm_s->tm_mon + 1, tm_s->tm_year + 1900,
				tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
			fflush(stdout);
		}
	}

	if (logging_flags & BLTS_LOG_FLAG_FILE) {
		fprintf(log_file, "%s", logging_level_message[level]);
		va_start(ap, format);
		vfprintf(log_file, format, ap);
		va_end(ap);
		fflush(log_file);
	}

	if (logging_flags & BLTS_LOG_FLAG_STDOUT) {
		va_start(ap, format);
		vfprintf(stdout, format, ap);
		va_end(ap);

		fflush(stdout);
	}
}

/** syslog capture, runs as it's own in childprocess */
int blts_log_run_syslog_capture()
{
	int read_bytes = 0;
	int written_bytes = 0;
	time_t t;
	struct tm *tm_s;
	FILE* kmsg_output_file = fopen(syslog_path, "w");
	if(!kmsg_output_file)
		return -1;

	/* run until process killed */
	while(1) {
		/* read and clean message buffer */
		read_bytes = klogctl(4, buffer, 4095);
		if (read_bytes < 0) {
			BLTS_LOGGED_PERROR("klogctl");
			break;
		} else if (!read_bytes)
			continue;

		if (print_timestamp) {
			t = time(NULL);
			tm_s = localtime(&t);
			written_bytes = fprintf(kmsg_output_file,
				"[%02d.%02d.%04d %02d:%02d:%02d] ",
				tm_s->tm_mday, tm_s->tm_mon + 1, tm_s->tm_year + 1900,
				tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
		}

		written_bytes = fwrite(buffer, 1, read_bytes, kmsg_output_file);
		if (written_bytes < 0) {
			BLTS_LOGGED_PERROR("Writing kmsg failed");
			exit(0);
		}
		fflush(kmsg_output_file);
	}

	return 0;
}

//; < 0 = fail
int blts_log_start_syslog_capture()
{
	/* clear log */
	if (klogctl(4, buffer, 4095) < 0) {
		BLTS_LOGGED_PERROR("klogctl");
		return -errno;
	}

	pid_t pid = fork();

	if (pid == 0) {
		fflush(stdout);
		blts_log_run_syslog_capture();
		exit(0);
	} else if (pid < 0)
		return -1;

	local_syslog_capture_pid = pid;
	return 0;
}

//; < 0 = fail
int blts_log_end_syslog_capture()
{
	if (local_syslog_capture_pid == 0)
		return -1;

	kill(local_syslog_capture_pid, SIGKILL);
	local_syslog_capture_pid = 0;

	return 0;
}

/* ========================== REMOVE THESE LATER ===========================*/

/* Open given log file; < 0 = fail */
int log_open(const char *filename, int log_to_console)
{
	char *real_filename;
	char *path_and_file;
	int n;

	if (!filename) {
		log_file = fopen("/dev/null","a");
	}
	else {
		real_filename = basename(filename);

		if(log_file) return -1;
		// check directory and create if necessary (all rights)
		if( access(log_path,X_OK) != 0 )
			mkdir(log_path, 0777);

		n = asprintf(&path_and_file, "%s/%s", log_path, real_filename);

		log_file = fopen(path_and_file, "a");
		free(path_and_file);
	}
	if(!log_file)
	{
		perror("log open");
		return -1;
	}
	log_console = log_to_console;
	logging_flags = BLTS_LOG_FLAG_FILE;
	if (log_console)
		logging_flags |= BLTS_LOG_FLAG_STDOUT;

	return 0;
}

/* Close current log file; < 0 = fail */
int log_close()
{
	int ret = 0;

	if(!log_file) return -1;

	if(fclose(log_file) != 0)
	{
		perror("log close");
		ret = -1;
	}
	log_file = 0;
	return ret;
}

/* set wheter to printf timestamp to log */
void log_set_timestamp(int set_timestamp)
{
	print_timestamp = set_timestamp;
}

/* printf to current log file (std args, return vals) */
int log_print(const char *format, ...)
{
	va_list ap;
	int n = 0;

	time_t t;
	struct tm *tm_s;

	if(!log_file) return -ENOENT;

	if(print_timestamp)
	{
		t = time(NULL);
		tm_s = localtime(&t);
		n = fprintf(log_file, "[%02d.%02d.%04d %02d:%02d:%02d] ",
				tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
				tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);

		if(log_console)
		{
			printf("[%02d.%02d.%04d %02d:%02d:%02d] ",
				tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
				tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
			fflush(stdout);

		}
	}

	va_start(ap, format);
	if(print_timestamp)
		n = n + vfprintf(log_file, format, ap);
	else
		n = vfprintf(log_file, format, ap);
	va_end(ap);

	fflush(log_file);

	if(log_console)
	{
		va_start(ap, format);
		vfprintf(stdout, format, ap);
		va_end(ap);

		fflush(stdout);
	}

	return n;
}

/* printf to current log file (std args, return vals) */
int log_print_level(int level, const char *format, ...)
{
	va_list ap;
	int n = 0;

	time_t t;
	struct tm *tm_s;

	if(!log_file) return -ENOENT;
	if(level > 3)
		level = 3;
	if(level < 0)
		level = 0;
	if(level >= logging_level)
	{
		if(print_timestamp)
		{
			t = time(NULL);
			tm_s = localtime(&t);
			n = fprintf(log_file, "[%02d.%02d.%04d %02d:%02d:%02d] ",
					tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
					tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
			if(log_console)
			{
				printf("[%02d.%02d.%04d %02d:%02d:%02d] ",
					tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
					tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
				fflush(stdout);

			}
		}

		n = n + fprintf(log_file, "%s", logging_level_message[level]);
		va_start(ap, format);
		n = n + vfprintf(log_file, format, ap);
		va_end(ap);

		fflush(log_file);

		if(log_console)
		{
			va_start(ap, format);
			vfprintf(stdout, format, ap);
			va_end(ap);

			fflush(stdout);
		}

	}
	return n;
}

void log_set_level(int level)
{
	logging_level = level;
}

/** syslog capture, runs as it's own in childprocess */
int run_syslog_capture()
{
	int read_bytes = 0;
	int written_bytes = 0;
	time_t t;
	struct tm *tm_s;
	FILE* kmsg_output_file = fopen(syslog_path, "w");
	if(!kmsg_output_file)
	{
		return -1;
	}
/* 	fprintf(kmsg_output_file, "testi\n"); */
	// run until process killed
	while(1)
	{
		// read and clean message buffer
		read_bytes = klogctl(4, buffer, 4095);
		if(read_bytes)
		{
			if(print_timestamp)
			{
				t = time(NULL);
				tm_s = localtime(&t);
				written_bytes = fprintf(kmsg_output_file, "[%02d.%02d.%04d %02d:%02d:%02d] ",
						tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
						tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
				if(log_console)
				{
					printf("[%02d.%02d.%04d %02d:%02d:%02d] ",
						tm_s->tm_mday, tm_s->tm_mon+1, tm_s->tm_year+1900,
						tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec);
					fflush(stdout);

				}
			}
			written_bytes = fwrite(buffer, 1, read_bytes, kmsg_output_file);
			if(written_bytes < 0)
			{
				BLTS_ERROR("Writing kmsg failed : %s\n", strerror(errno));
				BLTS_ERROR("Message capture ended\n");
				exit(0);
			}
			fflush(kmsg_output_file);

		}

	}

	return 0;	// should never happed
}

//; < 0 = fail
int start_syslog_capture()
{
	// clean log
	klogctl(4, buffer, 4095);
	pid_t pid = fork();

	if (pid == 0) // child process
	{
		fflush(stdout);
		run_syslog_capture();
		exit(0); // if fails, just quit process
	}

	else if (pid < 0)
	{
		// return false
		return -1;
	}

	local_syslog_capture_pid=pid;
	return 0;
}

//; < 0 = fail
int end_syslog_capture()
{
	if(local_syslog_capture_pid==0)
	{
		return -1;
	}

	kill( local_syslog_capture_pid, SIGKILL );
	local_syslog_capture_pid=0;

	return 0;
}
