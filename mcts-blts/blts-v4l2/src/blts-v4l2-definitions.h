/* blts-v4l2-cases.h -- Case definitions

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

#ifndef BLTS_V4L2_DEFINITIONSL_H
#define BLTS_V4L2_DEFINITIONSL_H

#include "camera.h"

/* Test case numbering */
enum {
	CORE_OPEN_DEVICE = 1,
	CORE_READ_DEVICE,
	CORE_CHECK_CONTROLS,
	CORE_CHECK_CAPABILITIES,
	CORE_CHECK_FORMATS,
	CORE_MEASURE_DEVICE,
	CORE_STREAM_TO_SCREEN,
	CORE_MEASURE_FPS,
	CORE_TAKE_JPEG,
	CORE_DEVICE_ENUMERATION,
	CORE_STREAM_AND_CROP,
	CORE_READ_WITH_POLL,
	CORE_TEST_STD_CONTROLS,
	CORE_TEST_EXT_CONTROLS,
	CORE_CHECK_PRIORITY,
	CORE_VARY_FRAME_RATE,
	CORE_GET_STREAM_QUALITY,
	CORE_SET_STREAM_QUALITY,
	CORE_DEBUG_CAPABILITY,
	CORE_SELECT_INPUT_STREAM,
	CORE_SELECT_OUTPUT_STREAM,
	CORE_TEST_STANDARDS
};

#define CROP_WIDTH               (1L<<0)
#define CROP_HEIGHT              (1L<<1)
#define CROP_LEFT                (1L<<2)
#define CROP_TOP                 (1L<<3)

typedef struct
{
        unsigned int flags;
        v4l2_dev_data *device;
        char *snapshot_filename;
        char *img_verify_tool;
        char *img_save_path;
} v4l2_data;

#define CLI_FLAG_PROFILING 1
#define CLI_FLAG_VERBOSE_LOG 2
#define CLI_FLAG_IMGSAVE 4

#endif /* BLTS_V4L2_DEFINITIONSL_H */
