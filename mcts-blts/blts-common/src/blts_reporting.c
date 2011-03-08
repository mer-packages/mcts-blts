/* blts_reporting.c -- Extended result reporting

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

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include "blts_reporting.h"
#include "blts_log.h"
#include "csv_file.h"

int blts_report_extended_result(char *tag, double value, char *unit, __attribute__((unused)) int use_limits)
{
	int ret;
	char *s = NULL;

	/* TODO: add limits if needed */
	ret = asprintf(&s, "%s;%lf;%s\n", tag, value, unit);
	if(ret < 0) {
		BLTS_LOGGED_PERROR("malloc");
		return ret;
	}

	ret = csv_result_add(s);
	if(s)
		free(s);
	return ret;
}

int blts_report_load_fail_limits(__attribute__((unused)) char *filename)
{
	BLTS_ERROR("Error: %s not implemented\n", __FUNCTION__);
	return -1;
}

int blts_report_set_fail_limit(__attribute__((unused)) char *tag,
			       __attribute__((unused)) double fail,
			       __attribute__((unused)) double *target,
			       __attribute__((unused)) int *low_is_good)
{
	BLTS_ERROR("Error: %s not implemented\n", __FUNCTION__);
	return -1;
}

int blts_report_get_fail_limit(__attribute__((unused)) char *tag,
			       __attribute__((unused)) double *fail,
			       __attribute__((unused)) double *target,
			       __attribute__((unused)) int *low_is_good)
{
	BLTS_ERROR("Error: %s not implemented\n", __FUNCTION__);
	return -1;
}

int blts_report_is_result_fail(__attribute__((unused)) char *tag,
			       __attribute__((unused)) double value)
{
	BLTS_ERROR("Error: %s not implemented\n", __FUNCTION__);
	return -1;
}


