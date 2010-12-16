/* -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* blts-fbdev-perf.c -- Frame buffer measurements

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* System libs */
#include <linux/fb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <float.h>

/* Project libs */
#include <blts_log.h>
#include <blts_timing.h>

/* Own includes */
#include "blts-fbdev-defs.h"
#include "blts-fbdev-perf.h"
#include "blts-fbdev-utils.h"

/* Arbitary default chunk size */
#define FBDEV_DEFAULT_CHUNK_SIZE 128
#define FBDEV_MINIMAL_RUN_LENGTH 5

/* Type definitions */
typedef struct {
        double *vals;   /* Array of results */
        unsigned int n_vals;  /* Actual number of results */
        size_t vals_sz; /* Size of array */
        char *unit;     /* Result unit */
} fbdev_measure_table;

typedef int (*fbdev_perf_measure_func) (blts_fbdev_data *data, double *result);

/* Utilities */

/* Create new measurement table */
static fbdev_measure_table *
fbdev_measure_table_new (const char *unit)
{
        fbdev_measure_table *table = malloc (sizeof (fbdev_measure_table));
        table->vals = malloc (FBDEV_DEFAULT_CHUNK_SIZE * sizeof (double));
        table->vals_sz = FBDEV_DEFAULT_CHUNK_SIZE;
        table->n_vals = 0;
        if (unit)
                table->unit = strdup (unit);
        else
                table->unit = NULL;
        return table;
}

/* Free the measurement table */
static void
fbdev_measure_table_free (fbdev_measure_table **table)
{
        if (!table || *table == NULL)
                return;

        free ((*table)->vals);
        free ((*table)->unit);
        free (*table);
        *table = NULL;
}

/* Append a value to the table */
static void
fbdev_measure_table_append_val (
    fbdev_measure_table **table,
    double                value)
{
        if(!table || !(*table)) return;

        fbdev_measure_table *t = *table;

        if((t->n_vals + 1) > t->vals_sz) {
                size_t new_size = t->vals_sz * 2;
                double* new_vals = realloc(t->vals, new_size * sizeof(double));
                if(!new_vals) {
                        BLTS_ERROR ("malloc failure, cannot save result\n");
                        return;
                }
                t->vals = new_vals;
                t->vals_sz = new_size;
        }
        t->vals[(t->n_vals)++] = value;

        *table = t;
}

/* Return arithmetic mean from table values. */
static double
fbdev_measure_table_average (fbdev_measure_table *table)
{
        if((!table) || table->n_vals <1) return DBL_MIN;
        double sum = 0.0;
        unsigned int i;

        for(i = 0; i < table->n_vals; ++i) sum += table->vals[i];

        return sum/table->n_vals;
}

/* Return minimum from table values. */
static double
fbdev_measure_table_min (fbdev_measure_table *table)
{
        unsigned int i;
        double c = DBL_MAX;

        for(i=0; i < table->n_vals; ++i) {
                if(table->vals[i] < c) c = table->vals[i];
        }
        return c;
}

/* Return maximum from table values. */
static double
fbdev_measure_table_max (fbdev_measure_table *table)
{
        unsigned int i;
        double c = DBL_MIN;

        for(i=0; i < table->n_vals; ++i) {
                if(table->vals[i] > c) c = table->vals[i];
        }
        return c;
}

/* Prints measurement table results */
static void
fbdev_print_measure_table (fbdev_measure_table *table)
{
        unsigned int i;

        BLTS_DEBUG("Results\n"
                   "-------------------------------------\n");

        if(!table) return;

        BLTS_DEBUG("Number of results: %u\n", table->n_vals);

        for(i = 0; i < table->n_vals; ++i) {
                /* nb. Print index from 1 */
                BLTS_DEBUG ("Result %u: %lf %s\n",
                            i + 1, table->vals[i], table->unit);
        }

        BLTS_DEBUG("-------------------------------------\n"
                   "Minimum: %lf %s\n"
                   "Maximum: %lf %s\n"
                   "Mean:    %lf %s\n",
                   fbdev_measure_table_min (table), table->unit,
                   fbdev_measure_table_max (table), table->unit,
                   fbdev_measure_table_average (table), table->unit);
}

/* Prints a spammy header */
static void
fbdev_print_header (blts_fbdev_data *data)
{
        BLTS_DEBUG(
            "===========================================================\n"
            "Frame buffer performance test, Benchmark version %s\n"
            "-------------------------------------\n"
            "Reported buffer geometry:\n"
            "Buffer size(bytes): %lu Width %u Height %u\n"
            "Depth %u, Red, %u, Green, %u, Blue, %u, Alpha, %u\n",
            VERSION, data->device->fixed_info.smem_len,
            data->device->variant_info.xres,
            data->device->variant_info.yres,
            data->device->variant_info.bits_per_pixel,
            data->device->variant_info.red.length,
            data->device->variant_info.green.length,
            data->device->variant_info.blue.length,
            data->device->variant_info.transp.length);
}

/* Create test data, returns 0 if no memory */
static int
fbdev_test_data_new (blts_fbdev_data *data)
{
        size_t lut_sz = (data->device->variant_info.xres *
                         data->device->variant_info.yres);

        FUNC_ENTER();

        __u32 *lut = malloc (lut_sz * sizeof (__u32));

        if (!lut) {
                BLTS_ERROR ("OOM!\n");
                return 0;
        }

        __u32 x,y;
        __u32 line_start, x_offset;
        __u32 bpp_bytes = data->device->variant_info.bits_per_pixel >> 3;

        /* write offsets to table*/
        line_start = (data->device->fixed_info.line_length *
                      data->device->variant_info.yoffset +
                      bpp_bytes * data->device->variant_info.xoffset);

        __u32 i = 0;
        for (y = 0; y < data->device->variant_info.yres; ++y) {
                x_offset = 0;

                for (x = 0; x < data->device->variant_info.xres; ++x) {
                        lut[i++] = line_start + x_offset;
                        x_offset += bpp_bytes;
                }
                line_start += data->device->fixed_info.line_length;
        }

        /* shuffle (this is the Knuth shuffle, actually) */
        __u32 k, tmp, n = lut_sz;
        while (n)
        {
                k = rand() % n;
                n--;
                tmp = lut[n];
                lut[n] = lut[k];
                lut[k] = tmp;
        }

        data->shuffle_lut = lut;

        FUNC_LEAVE();

        return 1;
}

/* Create a test bitmap with random data, sized as the fb settings; free with
 * free()
 */
static int
fbdev_test_frame_new (blts_fbdev_data *data)
{
        size_t sz = (data->device->variant_info.xres *
                     data->device->variant_info.yres *
                     (data->device->variant_info.bits_per_pixel >> 3));
        uint i, j;

        FUNC_ENTER();

        for (i = 0; i < 2; i++) {
                data->testframe[i] = malloc (sz);
                if (!data->testframe[i]) {
                        BLTS_ERROR ("OOM");
                        return 0;
                }
                for (j = 0; j < sz; j++) {
                        data->testframe[i][j] = rand() & 0xff;
                }
        }

        FUNC_LEAVE();

        return 1;
}

/* Read to a mem buffer from the visible portion of frame buffer. Assumes the
 * buffer is properly sized and formatted (proper number of right-bpp pixels,
 * row by row).
 */
#ifdef __ARMEL__
static void __attribute__((always_inline))
#else
static inline void
#endif
fbdev_grab_buf_clip_vis (blts_fbdev_data *data, void *buffer)
{
        blts_fbdev_device *dev = data->device;
        /* Start of visible part of frame buffer */
        char* src = (dev->buffer + dev->fixed_info.line_length *
                     dev->variant_info.yoffset +
                     (dev->variant_info.bits_per_pixel >> 3) *
                     dev->variant_info.xoffset);
        size_t line_sz = (dev->variant_info.xres *
                          (dev->variant_info.bits_per_pixel >> 3));
        __u32 i;

        for(i = 0; i < dev->variant_info.yres ; ++i) {
                memcpy (buffer + i * line_sz, src, line_sz);
                src += dev->fixed_info.line_length;
        }
}

/* Blit a mem buffer to the visible portion of frame buffer. Assumes the
 * buffer is properly sized and formatted (proper number of right-bpp
 * pixels, row by row).
 */
#ifdef __ARMEL__
/* weirdness with GCC there */
static void __attribute__((always_inline))
#else
static inline void
#endif
fbdev_blit_buf_clip_vis (blts_fbdev_data *data, void *buffer)
{
        __u32 i;
        blts_fbdev_device *dev = data->device;

        /* start of visible part of frame buffer */
        char* dest = (dev->buffer +
                      dev->fixed_info.line_length * dev->variant_info.yoffset +
                      (dev->variant_info.bits_per_pixel >> 3) *
                      dev->variant_info.xoffset);

        size_t line_sz = (dev->variant_info.xres *
                          (dev->variant_info.bits_per_pixel >> 3));

        for(i = 0; i < dev->variant_info.yres ; ++i) {
                memcpy (dest, buffer + i * line_sz, line_sz);
                dest += dev->fixed_info.line_length;
        }
}

/* Measurement functions */

/* Single pixel read from framebuffer
 * read 100 frames from frame buffer pixel by pixel, report number of
 * pixels x 1E-6 / time
 */
static int
fbdev_one_pixel_read (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);
        int ret = 0;
        unsigned int framecount = 0;
        double t_start, t_end;

        __u32 x, y;
        __u32 sum = 0;

        t_start = timing_elapsed();

        switch (dev->variant_info.bits_per_pixel) {
        case 32:
                for (framecount = 0; framecount < 100; ++framecount) {
                        /* The LUT has precomputed offsets to correct pixel
                         * locations. Computing the whole address there could
                         * shave 1-n instructions/pixel here, though.
                         */
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        sum += *((__u32*)(dev->buffer + data->shuffle_lut[y * dev->variant_info.xres + x]));
                                }
                        }
                }
                break;
        case 16:
                for (framecount = 0; framecount < 100; ++framecount) {
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        sum += *((__u16*)(dev->buffer + data->shuffle_lut[y*dev->variant_info.xres + x]));
                                }
                        }
                }
                break;

        default:
                /* Do 8/24 bpp frame buffers exist anywhere we care about? What
                 * about alignment issues? In any case, the fbdev api supports
                 * up to 32bpp (see kernel source and include/linux/fb.h).
                 */
                BLTS_WARNING("*** Unsupported display BPP value %u - test "
                             "skipped ***", dev->variant_info.bits_per_pixel);
                ret = -1;
                break;
        }
        t_end = timing_elapsed();

        /* Will never happen.. this is trickery so we actually compile the
         * reads. Note that using "volatile" there instead will be pretty
         * bad for performance, and that "register" is often treated as a
         * suggestion at best.
         */
        if (sum > 0xfffffff0)
                *result = -1.0;

        if ((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return ret;
}

/* Single pixel write into framebuffer
 * write 100 frames to frame buffer pixel by pixel, report number of
 * pixels x 1E-6 / time
 */
static int
fbdev_one_pixel_write (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);

        unsigned int framecount = 0;
        double t_start, t_end;

        __u32 x, y;
        __u32 pix32;
        __u16 pix16;
        int ret = 0;

        t_start = timing_elapsed();

        switch (dev->variant_info.bits_per_pixel) {
        case 32:
                for (framecount = 0; framecount < 100; ++framecount) {
                        pix32 = 0xff & (framecount);
                        /* Not the same as 16bpp case, but we'll assume the
                         * costs are about equal
                         */
                        pix32 = pix32 | pix32 << 16;
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        *((__u32*)(dev->buffer + data->shuffle_lut[y * dev->variant_info.xres + x])) = pix32;
                                }
                        }
                }
                break;
        case 16:
                for (framecount = 0; framecount < 100; ++framecount) {
                        pix16 = 0xff & (framecount);
                        pix16 = pix16 | pix16 << 8;
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        *((__u16*)(dev->buffer + data->shuffle_lut[y * dev->variant_info.xres + x])) = pix16;
                                }
                        }
                }
                break;
        default:
                BLTS_WARNING("*** Unsupported display BPP value %u - test "
                             "skipped ***", dev->variant_info.bits_per_pixel);
                ret = -1;
                break;
        }

        t_end = timing_elapsed();

        if((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return ret;
}

/* Single pixel read/write... read and invert all pixels in framebuffer
 * read 100 frames to frame buffer pixel by pixel, invert & write
 * value, report number of pixels x 1E-6 / time
 */
static int
fbdev_one_pixel_readwrite (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);
        unsigned int framecount = 0;
        double t_start, t_end;
        int ret = 0;
        __u32 x, y;

        t_start = timing_elapsed();

        switch (dev->variant_info.bits_per_pixel) {
        case 32:
                for (framecount = 0; framecount < 100; ++framecount) {
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        *((__u32*)(dev->buffer + data->shuffle_lut[y * dev->variant_info.xres + x])) ^= 0xffffffff;
                                }
                        }
                }
                break;
        case 16:
                for (framecount = 0; framecount < 100; ++framecount) {
                        for (y = 0; y < dev->variant_info.yres; ++y) {
                                for (x = 0; x < dev->variant_info.xres; ++x) {
                                        *((__u16*)(dev->buffer + data->shuffle_lut[y * dev->variant_info.xres + x])) ^= 0xffff;
                                }
                        }
                }
                break;
        default:
                BLTS_WARNING("*** Unsupported display BPP value %u - test "
                             "skipped ***", dev->variant_info.bits_per_pixel);
                ret = -1;
                break;
        }

        t_end = timing_elapsed();

        if((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return ret;
}

/* read 100 whole frames from frame buffer, report number of pixels x
 * 1E-6 / time
 */
static int
fbdev_buffer_read (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);
        unsigned int framecount = 0;
        double t_start, t_end;

        char* membuf[2];

        membuf[0] = malloc (frame_px_count *
                            (dev->variant_info.bits_per_pixel >> 3));
        membuf[1] = malloc (frame_px_count *
                            (dev->variant_info.bits_per_pixel >> 3));

        t_start = timing_elapsed();

        for (framecount = 0; framecount < 100; ++framecount) {
                fbdev_grab_buf_clip_vis (data, membuf[framecount&1]);
        }
        t_end = timing_elapsed();

        free (membuf[0]);
        free (membuf[1]);

        if ((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return 0;
}

/* write 100 whole frames to frame buffer, report number of pixels x
 * 1E-6 / time
 */
static int
fbdev_buffer_write (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);
        unsigned int framecount = 0;
        double t_start, t_end;

        t_start = timing_elapsed();

        for (framecount = 0; framecount < 100; ++framecount) {
                fbdev_blit_buf_clip_vis (data, data->testframe[framecount&1]);
        }
        t_end = timing_elapsed();

        if((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return 0;
}

/* read 100 whole frames to frame buffer, write back one of the test
 * frames, report number of pixels x 1E-6 / time
 */
static int
fbdev_buffer_modify (
    blts_fbdev_data *data,
    double          *result)
{
        blts_fbdev_device *dev = data->device;
        unsigned long frame_px_count = (dev->variant_info.xres *
                                        dev->variant_info.yres);
        unsigned int framecount = 0;
        double t_start, t_end;

        char* membuf;
        membuf = malloc (frame_px_count *
                         (dev->variant_info.bits_per_pixel >> 3));

        t_start = timing_elapsed();

        for (framecount = 0; framecount < 100; ++framecount) {
                fbdev_grab_buf_clip_vis (data, membuf);
                fbdev_blit_buf_clip_vis (
                    data, data->testframe[framecount&1]);
        }
        t_end = timing_elapsed();

        free (membuf);
        if ((t_end - t_start) < 1E-6)
                *result = -1.0;
        else
                *result = (100.0 * (double) frame_px_count * 1E-6 /
                           (t_end - t_start));

        return 0;
}

/* Run a measurement */

/* Repeat measurement until given run time over. Return results in table,
 * error: return <0
 */
static int
fbdev_run_measurement (
    fbdev_perf_measure_func  test_func,
    blts_fbdev_data         *data,
    double                   run_time)
{
        int count = 0;

        fbdev_measure_table *table = NULL;
        double measurement = 0.0;
        int test_ret = 0;
        int ret = 0;

        double iter_start = 0.0;
        double iter_end = 0.0;
        struct rusage usage_start;
        struct rusage usage_end;

        double used_time = 0.0;

        fbdev_print_header (data);

        table = fbdev_measure_table_new ("Mpix/s");

        if (!table) {
                BLTS_ERROR ("OOM!\n");
                return -1;
        }

        memset (data->device->buffer, 0, data->device->fixed_info.smem_len);

        timing_start();

        while (((iter_start = timing_elapsed()) < run_time) ||
               count < FBDEV_MINIMAL_RUN_LENGTH) {
                count++;

                getrusage (RUSAGE_SELF, &usage_start);
                test_ret = test_func (data, &measurement);

                fbdev_measure_table_append_val (
                    &table, test_ret < 0 ? -1.0 : measurement);

                if (test_ret < 0) ret = -1;

                /* figure out how much cpu time we used */
                iter_end = timing_elapsed();

                if (iter_end - iter_start > 1E-4) {
                        getrusage (RUSAGE_SELF, &usage_end);
                        used_time = usage_end.ru_utime.tv_sec;
                        used_time += usage_end.ru_utime.tv_usec * 1E-6;
                        used_time += usage_end.ru_stime.tv_sec;
                        used_time += usage_end.ru_stime.tv_usec * 1E-6;
                        used_time -= usage_start.ru_utime.tv_sec;
                        used_time -= usage_start.ru_utime.tv_usec * 1E-6;
                        used_time -= usage_start.ru_stime.tv_sec;
                        used_time -= usage_start.ru_stime.tv_usec * 1E-6;
                        BLTS_DEBUG ("CPU use %d: %lf %%\n",
                                    count, ((100.0 * used_time) /
                                            (iter_end - iter_start)));
                } else {
                        BLTS_DEBUG ("CPU use %d: --- %%\n", count);
                }
        }

        timing_stop();

        fbdev_print_measure_table (table);
        fbdev_measure_table_free (&table);

        memset (data->device->buffer, 0, data->device->fixed_info.smem_len);

        return ret;
}

/* Test cases */

/* One pixel read */
int
blts_fbdev_case_one_pixel_read (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (
            fbdev_one_pixel_read, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}

/* One pixel write */
int
blts_fbdev_case_one_pixel_write (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (
            fbdev_one_pixel_write, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}

/* One pixel write */
int
blts_fbdev_case_one_pixel_readwrite (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (
            fbdev_one_pixel_readwrite, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}

/* Read a fullscreen buffer test */
int
blts_fbdev_case_buffer_read (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (fbdev_buffer_read, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}

/* Write into a fullscreen buffer test */
int
blts_fbdev_case_buffer_write (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        if (!fbdev_test_frame_new (data)) {
                BLTS_ERROR ("Error: Failed to create test frames!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (fbdev_buffer_write, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}

/* Modify a fullscreen buffer test */
int
blts_fbdev_case_buffer_modify (void *test_data, int test_num)
{
        blts_fbdev_data *data = test_data;
        int              ret  = -1;

        FUNC_ENTER();

        if (!data || !data->device) {
                BLTS_ERROR ("Error: Test case not initted correctly!\n");
                goto ERROR;
        }

        if (!blts_fbdev_init (data->device)) {
                BLTS_ERROR ("Error: Failed to init device!\n");
                goto ERROR;
        }

        if (!blts_fbdev_get_variant_info (data->device)) {
                BLTS_ERROR ("Error: Failed to get variant screen info!\n");
                goto ERROR;
        }

        /* Test data */
        srand (42);

        if (!fbdev_test_data_new (data)) {
                BLTS_ERROR ("Error: Failed to create test data!\n");
                goto ERROR;
        }

        if (!fbdev_test_frame_new (data)) {
                BLTS_ERROR ("Error: Failed to create test frames!\n");
                goto ERROR;
        }

        /* In the end, the measure running will determine if the case was ok
         * or failed
         */
        ret = fbdev_run_measurement (fbdev_buffer_modify, data, 10.0);

        blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return ret;

ERROR:

        if (data)
                blts_fbdev_close (data->device);

        FUNC_LEAVE();

        return -1;
}
