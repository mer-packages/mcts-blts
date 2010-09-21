/* test_blitter.h -- Graphics benchmarks

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

#ifndef TEST_BLITTER_H
#define TEST_BLITTER_H

/* Max/min and other parameters */
#define NUM_VIDEO_IMAGES 8
#define MAX_DESKTOPS 16
#define MAX_SCENES 16
#define MAX_WIDGETS 16
#define MAX_WIDGET_IMAGES 4
#define MAX_PARTICLES 40
#define PARTICLE_LIFETIME 1.0f

/* Possible flags for test_blitter */
#define T_FLAG_BLEND 1
#define T_FLAG_BLUR 2
#define T_FLAG_WIDGETS 4
#define T_FLAG_WIDGET_SHADOWS 8
#define T_FLAG_ROTATE 16
#define T_FLAG_ZOOM 32
#define T_FLAG_PARTICLES 64
#define T_FLAG_VIDEO_WIDGETS 128
#define T_FLAG_CONVOLUTION 256

#endif // TEST_BLITTER

