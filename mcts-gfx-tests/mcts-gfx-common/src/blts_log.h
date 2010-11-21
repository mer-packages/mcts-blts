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

/* Messages with this level are not output anywhere without -vv argument */
#define LEVEL_TRACE	0 /* tracing-level logging */
/* Messages with these levels are output to the log file by default,
 * and to stdout when using -v argument */
#define LEVEL_DEBUG	1 /* debug messages */
#define LEVEL_WARN	2 /* warning level */
#define LEVEL_ERROR	3 /* error cases */

/* used at the beginning and end of functions */
#define FUNC_ENTER() { \
	blts_log_print_level(LEVEL_TRACE, "Entered %s\n", __FUNCTION__); }
#define FUNC_LEAVE() { \
	blts_log_print_level(LEVEL_TRACE, "Leaving %s\n", __FUNCTION__); }

/* use these instead of calling blts_log_print_level */
#define BLTS_TRACE(s...) { blts_log_print_level(LEVEL_TRACE, s); }
#define BLTS_DEBUG(s...) { blts_log_print_level(LEVEL_DEBUG, s); }
#define BLTS_WARNING(s...) { blts_log_print_level(LEVEL_WARN, s); }
#define BLTS_ERROR(s...) { blts_log_print_level(LEVEL_ERROR, s); }
#define BLTS_LOGGED_PERROR(s) \
	do { \
		int saved_err = errno; \
		blts_log_print_level(LEVEL_ERROR, s " - %s\n", \
			strerror(saved_err)); \
		errno = saved_err; \
	} while(0)

/* possible flags for blts_log_open */
#define BLTS_LOG_FLAG_FILE	1 /* Output log to file */
#define BLTS_LOG_FLAG_STDOUT	2 /* Output log to stdout */
#define BLTS_LOG_FLAG_TS	4 /* Include timestamps */

int blts_log_open(const char *filename, unsigned int flags);
int blts_log_close();
void blts_log_set_level(int level);
int blts_log_get_level();
void blts_log_print_level(int level, const char *format, ...);

int blts_log_start_syslog_capture();
int blts_log_end_syslog_capture();

/* ========= Old functions and macros, will be removed, do not use ========== */

#define LOG(s...) { log_print(s); }
#define LOGERR(s...) { log_print(s); }
#define LOGTRACE(s...) { log_print_level(LEVEL_TRACE, s); }

#define logged_perror(s)					\
	do {							\
		int saved_err = errno;				\
		log_print(s " - %s\n",strerror(saved_err));	\
		errno = saved_err;				\
	} while(0)

int log_open(const char *filename, int log_to_console) __attribute__ ((deprecated));
int log_close() __attribute__ ((deprecated));
int log_print(const char *format, ...) __attribute__ ((deprecated));
void log_set_timestamp(int set_timestamp) __attribute__ ((deprecated));
int log_print_level(int level, const char *format, ...) __attribute__ ((deprecated));
void log_set_level(int level) __attribute__ ((deprecated));
int start_syslog_capture() __attribute__ ((deprecated));
int end_syslog_capture() __attribute__ ((deprecated));

#endif
