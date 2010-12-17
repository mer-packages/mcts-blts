/* log.h -- Logging functionality

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

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>

/* Logging levels */

/**
 * Messages with LEVEL_TRACE are not output anywhere without -vv argument.
 * Higher levels are output to the log file by default, and to stdout when
 * using -v argument.
 */
enum {
	LEVEL_TRACE = 0, /**< tracing-level logging */
	LEVEL_DEBUG,     /**< debug messages */
	LEVEL_WARN,      /**< warning level */
	LEVEL_ERROR,     /**< error cases */
};

/** Log a trace entry for beginning of a function */
#define FUNC_ENTER() { \
	blts_log_print_level(LEVEL_TRACE, "Entered %s\n", __FUNCTION__); }
/** Log a trace entry for end of a function */
#define FUNC_LEAVE() { \
	blts_log_print_level(LEVEL_TRACE, "Leaving %s\n", __FUNCTION__); }

/* Use these instead of calling blts_log_print_level */
/** Log a message on LEVEL_TRACE */
#define BLTS_TRACE(s...) { blts_log_print_level(LEVEL_TRACE, s); }
/** Log a message on LEVEL_DEBUG */
#define BLTS_DEBUG(s...) { blts_log_print_level(LEVEL_DEBUG, s); }
/** Log a message on LEVEL_WARNING */
#define BLTS_WARNING(s...) { blts_log_print_level(LEVEL_WARN, s); }
/** Log a message on LEVEL_ERROR */
#define BLTS_ERROR(s...) { blts_log_print_level(LEVEL_ERROR, s); }
/** Take current errno and log a formatted @c strerror on LEVEL_ERROR,
 * prefixed by string @c s */
#define BLTS_LOGGED_PERROR(s) \
	do { \
		int saved_err = errno; \
		blts_log_print_level(LEVEL_ERROR, s " - %s\n", \
			strerror(saved_err)); \
		errno = saved_err; \
	} while(0)

/** Possible flags for blts_log_open @see blts_log_open */
enum blts_log_open_flags {
	BLTS_LOG_FLAG_FILE =	1, /**< Output log to file */
	BLTS_LOG_FLAG_STDOUT =	2, /**< Output log to stdout */
	BLTS_LOG_FLAG_TS =	4, /**< Include timestamps */
};

int blts_log_open(const char *filename, unsigned int flags);
int blts_log_close();
void blts_log_set_level(int level);
int blts_log_get_level();
void blts_log_print_level(int level, const char *format, ...);

int blts_log_start_syslog_capture();
int blts_log_end_syslog_capture();

/* ========= Old functions and macros, will be removed, do not use ========== */

/** @deprecated Use BLTS_* macros instead */
#define LOG(s...) { log_print(s); }
/** @deprecated Use BLTS_ERROR instead */
#define LOGERR(s...) { log_print(s); }
/** @deprecated Use BLTS_TRACE instead */
#define LOGTRACE(s...) { log_print_level(LEVEL_TRACE, s); }

/** @deprecated Use BLTS_LOGGED_PERROR instead */
#define logged_perror(s)					\
	do {							\
		int saved_err = errno;				\
		log_print(s " - %s\n",strerror(saved_err));	\
		errno = saved_err;				\
	} while(0)

/** @deprecated Use blts_log_open instead */
int log_open(const char *filename, int log_to_console) __attribute__ ((deprecated));
/** @deprecated Use blts_log_close instead */
int log_close() __attribute__ ((deprecated));
/** @deprecated Use blts_log_print_level instead */
int log_print(const char *format, ...) __attribute__ ((deprecated));
/** @deprecated Don't use */
void log_set_timestamp(int set_timestamp) __attribute__ ((deprecated));
/** @deprecated Use blts_log_print_level instead */
int log_print_level(int level, const char *format, ...) __attribute__ ((deprecated));
/** @deprecated Use blts_log_set_level instead */
void log_set_level(int level) __attribute__ ((deprecated));
/** @deprecated Use blts_log_start_syslog_capture instead */
int start_syslog_capture() __attribute__ ((deprecated));
/** @deprecated Use blts_log_end_syslog_capture instead */
int end_syslog_capture() __attribute__ ((deprecated));

#endif
