/* ------------------------------------------------------------------------- *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Matti Kosola <matti.kosola@jollamobile.com>
 * License: GPLv2
 * Note: Adapted from MCE display module
 * ------------------------------------------------------------------------- */

#include <stdlib.h>
#include <glob.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int brightness_glob_err_cb(const char *path, int err);
static gboolean get_brightness_controls(const gchar *dirpath,
                                        char **setpath,
					char **maxpath,
					char **subsyspath);

int main( int argc, const char* argv[] )
{
        static const char pattern[] = "/sys/class/backlight/*";

        static const char * const lut[] = {
                /* this seems to be some kind of "Android standard" path */
                "/sys/class/leds/lcd-backlight",
                NULL
        };

        gchar     *set = 0;
        gchar     *max = 0;
	gchar	  *subsys = 0;

        glob_t    gb;

	size_t i = 0;

        memset(&gb, 0, sizeof gb);

        /* Assume: Any match from fixed list will be true positive.
	   Check them before possibly ambiguous backlight class entries. */
        for( i = 0; lut[i]; ++i ) {
                if( get_brightness_controls(lut[i], &set, &max, &subsys) )
                        goto EXIT;
        }

        if( glob(pattern, 0, brightness_glob_err_cb, &gb) != 0 ) {
                printf("no backlight devices found\n");
                goto EXIT;
        }

        if( gb.gl_pathc > 1 )
                printf("several backlight devices present, "
                       "choosing the first usable one\n");
        for( i = 0; i < gb.gl_pathc; ++i ) {
                const char *path = gb.gl_pathv[i];

                if( get_brightness_controls(path, &set, &max, &subsys) )
                        goto EXIT;
        }

EXIT:
        /* Have we found both brightness and max_brightness files? */
        if( set && max && subsys ) {
		printf("subsystem_path=%s\n", subsys);
                printf("brightness_path=%s\n", set);
                printf("max_brightness_path=%s\n", max);

		/* Read max brightness */
		int fd;
		fd = open(max, O_RDONLY);
		if( fd == -1 ) {
			printf("cannot open max backlight file\n");
		}

		char buf[80];
		ssize_t temp;
		memset(buf, 0, sizeof(buf));
		temp = read(fd, buf, sizeof(buf));
		if( temp == -1 ) {
			printf("Reading max backlight failed\n");
		}

		close(fd);

		printf("max_brightness=%s", buf);
	}

        g_free(max);
        g_free(set);
	g_free(subsys);

        globfree(&gb);

	return EXIT_SUCCESS;
}


/** Callback function for logging errors within glob()
 *
 * @param path path to file/dir where error occurred
 * @param err  errno that occurred
 *
 * @return 0 (= do not stop glob)
 */
static int brightness_glob_err_cb(const char *path, int err)
{
	printf("%s: glob: %s\n", path, g_strerror(err));
	return 0;
}


/** Check if sysfs directory contains brightness and max_brightness entries
 *
 * @param sysfs directory to probe
 * @param setpath place to store path to brightness file
 * @param maxpath place to store path to max_brightness file
 * @param subsyspath place to store display subsystem path
 * @return TRUE if brightness and max_brightness files were found,
 *         FALSE otherwise
 */
static gboolean get_brightness_controls(const gchar *dirpath,
                                        char **setpath,
					char **maxpath,
					char **subsyspath)
{
        gboolean  res = FALSE;

        gchar *set = g_strdup_printf("%s/brightness", dirpath);
        gchar *max = g_strdup_printf("%s/max_brightness", dirpath);
	gchar *subsys = g_strdup_printf("%s", dirpath);

        if( set && max && !g_access(set, R_OK) && !g_access(max, R_OK) ) {
                *setpath = set, set = 0;
                *maxpath = max, max = 0;
		*subsyspath = subsys, subsys = 0;
                res = TRUE;
        }

        g_free(set);
        g_free(max);
	g_free(subsys);

        return res;
}
