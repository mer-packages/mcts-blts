# Copyright (C) 2010 Intel Corporation
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

AC_INIT([mcts-pulseaudio-tests], [1.0.0], [shuang.wan@intel.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

AC_CONFIG_FILES([Makefile  src/Makefile data/Makefile utils/Makefile])

AC_CHECK_HEADERS([pulse/pulseaudio.h], [], [AC_MSG_FAILURE([*** pulseaudio development header file not found.])]) 

AC_OUTPUT
