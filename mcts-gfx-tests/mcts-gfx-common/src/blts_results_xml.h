/* blts_results_xml.h -- Results XML writer

   Copyright (C) 2010, Nokia Corporation.

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

#ifndef BLTS_RESULTS_XML_H
#define BLTS_RESULTS_XML_H


int xml_result_open(const char *filename, const char *suite, const char *set,
	int append);
void xml_result_close();

int xml_result_start_case(const char *name, int manual, int result);
int xml_result_write_step(int expected_result, int return_code, time_t *start,
	time_t *end, const char *cmdline, const char *failure_info);
int xml_result_end_case();

#endif /* BLTS_RESULTS_XML_H */
