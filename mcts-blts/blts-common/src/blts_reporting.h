/* blts_reporting.h -- Extended result reporting interface

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

#ifndef BLTS_REPORTING_H
#define BLTS_REPORTING_H

int blts_report_extended_result(char *tag, double value, char *unit, int use_limits);

int blts_report_load_fail_limits(char *filename);
int blts_report_set_fail_limit(char *tag, double fail, double *target, int *low_is_good);
int blts_report_get_fail_limit(char *tag, double *fail, double *target, int *low_is_good);
int blts_report_is_result_fail(char *tag, double value);

#endif /* BLTS_REPORTING_H */

