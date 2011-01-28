/* xrandr_tests.h -- Tests for X Randr extension

   Copyright (C) 2000-2010, Intel Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Zheng Kui <kui.zheng@intel.com>

*/

int test_enumerate (double execution_time);
int test_rotate (double execution_time);
int test_position (double execution_time);
int test_scale (double execution_time);
int test_backlight (double execution_time);
int test_modes (double execution_time);
int test_hotplug (double execution_time);
