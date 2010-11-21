# Makefile -- Makefile for the CLI example
#
#  Copyright (C) 2000-2010, Nokia Corporation.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

GCC_FLAGS := -Wall -O2 -lbltscommon
GCC_FILES := blts_cli_example_module.c

default:
	$(CC) $(GCC_FLAGS) $(GCC_FILES) -o blts_cli_example

clean:
	rm -f *.o blts_cli_example

