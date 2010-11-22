/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-blanking.c -- Framebuffer blanking tests

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

/* System includes */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

/* BLTS common */
#include <blts_log.h>

/* Own includes */
#include "blts-fbdev-defs.h"
#include "blts-fbdev-backlight.h"
#include "blts-fbdev-utils.h"

/* Own definitions */

/* Current light level(s) */
#define BLTS_FBDEV_BACKLIGHT_CURRENT "/brightness"
#define BLTS_FBDEV_BACKLIGHT_ACTUAL  "/actual_brightness"
#define BLTS_FBDEV_BACKLIGHT_MAX     "/max_brightness"

/* Private functions */

/* Read a brightness value from a file */
static int
blts_fbdev_get_brightness (blts_fbdev_data *data, const char *brightness_file)
{
        int   brightness  = 0;
        char *full_path   = NULL;
        FILE *brightnessf = NULL;
        size_t foo, foo2, len;

        FUNC_ENTER();

        foo  = strlen (data->backlight_subsystem);
        foo2 = strlen (brightness_file);
        len  = foo + foo2 + 1;

        full_path = malloc (sizeof (char) * len);
        memset (full_path, 0, len);

        strcat (full_path, data->backlight_subsystem);
        strcat (full_path, brightness_file);

        brightnessf = fopen (full_path, "r");

        if (!brightnessf) {
                BLTS_ERROR ("Error: Failed to open brightness file %s: %s\n",
                            full_path, strerror (errno));
                goto ERROR;
        }

        BLTS_TRACE ("Reading brightness from %s\n", full_path);

        if (fscanf (brightnessf, "%d", &brightness) != 1) {
                BLTS_ERROR ("Error: Failed to read brightness file %s: %s\n",
                            full_path, strerror (errno));
                goto ERROR;
        }

        if (full_path) {
                free (full_path);
                full_path = NULL;
        }

        if (brightnessf) {
                fclose (brightnessf);
                brightnessf = NULL;
        }

        BLTS_TRACE ("Brightness -> %d\n", brightness);

        FUNC_LEAVE();

        return brightness;

ERROR:

        if (full_path) {
                free (full_path);
                full_path = NULL;
        }

        if (brightnessf) {
                fclose (brightnessf);
                brightnessf = NULL;
        }

        FUNC_LEAVE();

        return -1;
}

/* Write a brightness value into a file */
static int
blts_fbdev_set_brightness (blts_fbdev_data *data, int brightness)
{
        char *full_path   = NULL;
        FILE *brightnessf = NULL;
        size_t foo, foo2, len;

        FUNC_ENTER();

        FUNC_ENTER();

        foo  = strlen (data->backlight_subsystem);
        foo2 = strlen (BLTS_FBDEV_BACKLIGHT_CURRENT);
        len  = foo + foo2 + 1;

        full_path = malloc (sizeof (char) * len);
        memset (full_path, 0, len);

        strcat (full_path, data->backlight_subsystem);
        strcat (full_path, BLTS_FBDEV_BACKLIGHT_CURRENT);

        brightnessf = fopen (full_path, "w");

        if (!brightnessf) {
                BLTS_ERROR ("Error: Failed to open (and truncate) brightness "
                            "file %s: %s\n", full_path, strerror (errno));
                goto ERROR;
        }

        BLTS_TRACE ("Setting brightness %d to %s\n", brightness, full_path);

        if (fprintf (brightnessf, "%d", brightness) < 1) {
                BLTS_ERROR ("Error: Failed to write brightness file %s: %s\n",
                            full_path, strerror (errno));
                goto ERROR;
        }

        if (full_path) {
                free (full_path);
                full_path = NULL;
        }

        if (brightnessf) {
                fclose (brightnessf);
                brightnessf = NULL;
        }

        FUNC_LEAVE();

        return 0;

ERROR:
        if (full_path) {
                free (full_path);
                full_path = NULL;
        }

        if (brightnessf) {
                fclose (brightnessf);
                brightnessf = NULL;
        }

        FUNC_LEAVE();

        return -1;
}

/* Public functions */

/* Verify limit values based off config */
int
blts_fbdev_case_backlight_verify (void *test_data, int test_num)
{
        blts_fbdev_data *data  = test_data;
        int current_brightness = 0;
        int max_brightness     = 0;
        int actual_brightness  = 0;
        int new_brightness     = 0;

        FUNC_ENTER();

        if (!data || !data->backlight_subsystem) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        current_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_CURRENT);

        if (current_brightness < 0) {
                BLTS_ERROR ("Error: Failed to get current brightness!\n");
                goto ERROR;
        }

        max_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_MAX);

        if (max_brightness < 0) {
                BLTS_ERROR ("Error: Failed to get max brightness!\n");
                goto ERROR;
        }

        if (max_brightness != data->maximum_light) {
                BLTS_ERROR ("Error: Configured max brightness (%d) does not "
                            "match reported max brightness (%d)\n",
                            data->maximum_light, max_brightness);
                goto ERROR;
        }

        /* First check the minimum value */
        new_brightness = data->minimum_light;

        if (blts_fbdev_set_brightness (data, new_brightness) < 0) {
                BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                            new_brightness);
                goto ERROR;
        }

        actual_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

        if (actual_brightness != new_brightness) {
                BLTS_ERROR ("Error: Actual brightness %d isn't the same as "
                            "requested one %d!\n", actual_brightness,
                            new_brightness);
                goto ERROR;
        }

        /* Then the configured max */
        new_brightness = data->maximum_light;

        if (blts_fbdev_set_brightness (data, new_brightness) < 0) {
                BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                            new_brightness);
                goto ERROR;
        }

        actual_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

        if (actual_brightness != new_brightness) {
                BLTS_ERROR ("Error: Actual brightness %d isn't the same as "
                            "requested one %d!\n", actual_brightness,
                            new_brightness);
                goto ERROR;
        }

        /* Then beyond the max */
        new_brightness = data->maximum_light + 1;

        /* We don't care if this works or not, we're more interested in
         * reported values while we go over the limits
         */
        blts_fbdev_set_brightness (data, new_brightness);

        actual_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

        if (actual_brightness == new_brightness) {
                BLTS_ERROR ("Error: Actual brightness %d is the same as "
                            "requested one %d!\n", actual_brightness,
                            new_brightness);
                goto ERROR;
        }

        /* Then below the minimum */
        new_brightness = data->minimum_light - 1;

        /* We don't care if this works or not, we're more interested in
         * reported values while we go over the limits
         */
        blts_fbdev_set_brightness (data, new_brightness);

        actual_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

        if (actual_brightness == new_brightness) {
                BLTS_ERROR ("Error: Actual brightness %d is the same as "
                            "requested one %d!\n", actual_brightness,
                            new_brightness);
                goto ERROR;
        }

        /* Restore the original brightness */
        new_brightness = current_brightness;

        if (blts_fbdev_set_brightness (data, new_brightness) < 0) {
                BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                            new_brightness);
                goto ERROR;
        }

        FUNC_LEAVE();

        return 0;

ERROR:

        FUNC_LEAVE();

        return -1;
}

/* Linearly tests different backlight values */
int
blts_fbdev_case_backlight_linear (void *test_data, int test_num)
{
        blts_fbdev_data *data  = test_data;
        int current_brightness = 0;
        int i = 0;

        FUNC_ENTER();

        if (!data || !data->backlight_subsystem) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        current_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_CURRENT);

        if (current_brightness < 0) {
                BLTS_ERROR ("Error: Failed to get current brightness!\n");
                goto ERROR;
        }

        /* Check each level linearly. Sleep for 2 seconds for external
         * verification between levels.
         */
        for (i = data->minimum_light; i <= data->maximum_light; i++) {
                int actual_brightness = 0;

                if (blts_fbdev_set_brightness (data, i) < 0) {
                        BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                                    i);
                        goto ERROR;
                }

                actual_brightness = blts_fbdev_get_brightness (
                        data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

                if (actual_brightness != i) {
                        BLTS_ERROR ("Error: Actual brightness %d isn't the "
                                    "same as requested one %d!\n",
                                    actual_brightness, i);
                        goto ERROR;
                }

                sleep (2);

                /* TODO:
                 * - Support for external verification tool?
                 */
        }

        for (i = data->maximum_light; i >= data->minimum_light; i--) {
                int actual_brightness = 0;

                if (blts_fbdev_set_brightness (data, i) < 0) {
                        BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                                    i);
                        goto ERROR;
                }

                actual_brightness = blts_fbdev_get_brightness (
                        data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

                if (actual_brightness != i) {
                        BLTS_ERROR ("Error: Actual brightness %d isn't the "
                                    "same as requested one %d!\n",
                                    actual_brightness, i);
                        goto ERROR;
                }

                sleep (2);

                /* TODO:
                 * - Support for external verification tool?
                 */
        }

        /* Restore the original brightness */
        if (blts_fbdev_set_brightness (data, current_brightness) < 0) {
                BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                            current_brightness);
                goto ERROR;
        }

        FUNC_LEAVE();

        return 0;

ERROR:

        FUNC_LEAVE();

        return -1;
}

/* Logarithmically tests different backlight values */
int
blts_fbdev_case_backlight_logarithmic (void *test_data, int test_num)
{
        blts_fbdev_data *data  = test_data;
        int current_brightness = 0;
        int i = 0;

        FUNC_ENTER();

        if (!data || !data->backlight_subsystem) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        current_brightness = blts_fbdev_get_brightness (
                data, BLTS_FBDEV_BACKLIGHT_CURRENT);

        if (current_brightness < 0) {
                BLTS_ERROR ("Error: Failed to get current brightness!\n");
                goto ERROR;
        }

        if (data->maximum_light <= 1) {
                BLTS_ERROR ("Error: Cannot test logarithmic light levels if "
                            "max light is 1 or less!\n");
                goto ERROR;
        }

        if (data->minimum_light < 1) {
                BLTS_ERROR ("Error: Cannot test logarithmic light levels if "
                            "minimum light is less than 1!\n");
                goto ERROR;
        }

        double multiplier = data->maximum_light / log10 (data->maximum_light);

        for (i = data->minimum_light; i < data->maximum_light; i++) {
                double light_level = 0;
                int    actual      = 0;

                light_level = multiplier * log10 (i);

                if (blts_fbdev_set_brightness (data, (int)light_level) < 0) {
                        BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                                    light_level);
                        goto ERROR;
                }

                actual = blts_fbdev_get_brightness (
                        data, BLTS_FBDEV_BACKLIGHT_ACTUAL);

                if (actual != (int)light_level) {
                        BLTS_ERROR ("Error: Actual brightness %d isn't the "
                                    "same as requested one %d!\n",
                                    actual, (int)light_level);
                        goto ERROR;
                }

                sleep (2);

                /* TODO:
                 * - Support for external verification tool?
                 */
        }

        /* Restore the original brightness */
        if (blts_fbdev_set_brightness (data, current_brightness) < 0) {
                BLTS_ERROR ("Error: Failed to set brightness to %d!\n",
                            current_brightness);
                goto ERROR;
        }

        FUNC_LEAVE();

        return 0;

ERROR:

        FUNC_LEAVE();

        return -1;
}
