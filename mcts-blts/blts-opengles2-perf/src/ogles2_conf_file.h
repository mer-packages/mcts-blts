/* ogles2_conf_file.h -- Configuration file parser

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
 
#ifndef OGLES2_CONF_FILE_H
#define OGLES2_CONF_FILE_H

#include "test_common.h"

int read_config(const char* filename, test_configuration_file_params* cases);
 
#endif // OGLES2_CONF_FILE_H

