/* csv_file.c -- CSV file handling (library internal use)

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

#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>

#include "csv_file.h"
#include "blts_log.h"

FILE *csv_file = NULL;
int csv_fd;

/**
 * Set up CSV file for measurement reporting.
 * @param path         Path to file
 * @param append_mode  If 0, clear file, otherwise just add to end.
 * @return 0 on success, -errno otherwise.
 */
int csv_result_file_open(char *path, int append_mode)
{
	FILE *f;
	char* o_flag = append_mode ? "ac" : "wc";
	if(csv_file)
		return -EBUSY;

	f = fopen(path, o_flag);

	if(!f) {
		BLTS_LOGGED_PERROR("blts-common: Cannot open CSV result file");
		return -errno;
	}
	csv_fd = fileno(f);
	csv_file = f;
	return 0;
}


/**
 * Safely write string to current CSV result file. Note that this will block
 * until access is possible if multiple threads use the file. Also note that
 * this is a silent no-op if no result file has been selected.
 * @param str  String to write
 * @return 0 on success, -errno otherwise.
 */
int csv_result_add(char *str)
{
	int err, ret = 0;
	if(!csv_file)
		return 0;

	err = flock(csv_fd, LOCK_EX);
	if(err) {
		BLTS_LOGGED_PERROR("blts-common: Failed locking CSV result file");
		return -errno;
	}

	err = fputs(str, csv_file);
	if(err == EOF) {
		BLTS_ERROR("blts-common: Failed writing CSV result entry\n");
		ret = -EIO;
	}
	fflush(csv_file);
	fdatasync(csv_fd);

	err = flock(csv_fd, LOCK_UN);
	if(err) {
		BLTS_LOGGED_PERROR("blts-common: Failed unlocking CSV result file");
		return -errno;
	}
	return ret;
}


/**
 * Close and sync current CSV result file.
 * @param str  String to write
 * @return 0 on success, -errno otherwise.
 */
int csv_result_file_close()
{
	int err;
	FILE *tmp;
	if(!csv_file)
		return 0;
	err = flock(csv_fd, LOCK_EX);
	if(err) {
		BLTS_LOGGED_PERROR("blts-common: Failed locking CSV result file");
		return -errno;
	}
	fsync(csv_fd);
	tmp = csv_file;
	csv_file = NULL;
	csv_fd = -1;
	fclose(tmp);
	return 0;
}

