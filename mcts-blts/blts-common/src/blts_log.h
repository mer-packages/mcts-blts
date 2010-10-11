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

//old trace definition
//#define TRACE(s...) fprintf(stderr,s)
#define LOG(s...) { log_print(s); }
#define LOGERR(s...) { log_print(s); }
#define LOGTRACE(s...) fprintf(stderr,s)

// debug levels

// every tiny bit of information
#define LEVEL_TRACE		5
// debug messages
#define LEVEL_DEBUG		4
// warning level
#define LEVEL_WARN		3
// error cases
#define LEVEL_ERROR		2
// program crash level warning
#define LEVEL_FATAL		1
// no output
#define LEVEL_OFF		0

// used at the beginning and end of functions
#define FUNC_ENTER() { log_print_level(LEVEL_TRACE, "Entered %s\n", __FUNCTION__); }
#define FUNC_LEAVE() { log_print_level(LEVEL_TRACE, "Leaving %s\n", __FUNCTION__); }

// shortcuts
#define BLTS_TRACE(s...) { log_print_level(LEVEL_TRACE, s); }
#define BLTS_DEBUG(s...) { log_print_level(LEVEL_DEBUG, s); }
#define BLTS_WARNING(s...) { log_print_level(LEVEL_WARN, s); }
#define BLTS_ERROR(s...) { log_print_level(LEVEL_ERROR, s); }
#define BLTS_FATAL(s...) { log_print_level(LEVEL_FATAL, s); }


#define logged_perror(s)					\
	do {							\
		int saved_err = errno;				\
		perror(s);					\
		log_print(s " - %s\n",strerror(saved_err));	\
		errno = saved_err;				\
	} while(0)


int log_open(const char *filename, int log_to_console);

int log_close();

int log_print(const char *format, ...);

void log_set_timestamp(int set_timestamp);

int log_print_level(int level, const char *format, ...);

void log_set_level(int level);

// system log

int start_syslog_capture();
int end_syslog_capture();

#endif
