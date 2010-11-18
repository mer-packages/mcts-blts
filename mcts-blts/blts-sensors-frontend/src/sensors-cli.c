/* sensors-cli.c -- Sensor tests command line interface

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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dlfcn.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>
#include <blts_timing.h>

#include "blts_sensor_variable.h"

typedef int (*init_plugin_data_t)(void** plugin_data);
typedef int (*deinit_plugin_data_t)(void* plugin_data);
typedef int (*init_plugin_t)(void** plugin_data);
typedef int (*deinit_plugin_t)(void* plugin_data);
typedef const struct plugin_variable_spec* (*get_variable_spec_t)(const char* var_name);
typedef blts_cli_testcase* sensors_cases_t;

static init_plugin_data_t plugin_init_plugin_data;
static deinit_plugin_data_t plugin_deinit_plugin_data;
static init_plugin_t plugin_init_plugin;
static deinit_plugin_t plugin_deinit_plugin;
static get_variable_spec_t plugin_get_variable_spec;
static sensors_cases_t plugin_sensors_cases;

enum test_type {
    INIT,
    NOISE,
    RANGE,
    AXIS,
    RATE,
    TESTVALUE,
    VALUECHANGE,
    NOVALUECHANGE,

    NUMTESTTYPES
};

// These must be in the same order as the values of the test_type enum
static const char* test_type_names[] =
{
    "init",
    "noise",
    "range",
    "axis",
    "rate",
    "test_value",
    "value_change",
    "no_value_change"
};

typedef struct {
	void* plugin_private_data;

	const blts_cli_testcase* testcases;

	// Used by all or multiple test types
	enum test_type test_type;
	const char* sensor_input_name;
	int duration;

	// Noise test
	int max_noise;

	// Range test
	int range_min;
	int range_max;
	int range_deviation;

        // Rate test
        int input_is_polled;
        int max_rate_difference_pct;

        // Axis test
        int axis_rotation_threshold;
        int axis_index;

        // Value check test
	struct boxed_value* expected_value;
} sensors_data;

static void* plugin_handle = NULL;

static sensors_data* my_data = NULL;

static void sensors_help(const char* help_msg_base)
{
    fprintf(stdout, help_msg_base,
        "[-p <sensor_plug-in_path>]",
        "  -p: Specify the plug-in to test. This is the full path of the sensor plug-in .so.\n");
}

/* Arguments -l, -e, -en, -s, -?, -nc are reserved, do not use here */
static const char short_opts[] = "p:";
static const struct option long_opts[] =
{
    {"plugin", required_argument, NULL, 'p'},
    {0, 0, 0, 0}
};

static int app_init_once(const char* plugin_name, sensors_data *data);

static int parse_test_type(sensors_data* data, const char* test_type_name)
{
    for (int i = 0; i < NUMTESTTYPES; i++)
    {
        if (strcmp(test_type_names[i], test_type_name) == 0)
        {
            data->test_type = i;
            return 0;
        }
    }

    BLTS_ERROR("Unknown test type \"%s\"\n", test_type_name);
    return -1;
}

static int parse_arg(sensors_data* data, const char* arg_tag, struct boxed_value* arg_value)
{
    int res = 0;

    if (strcmp(arg_tag, "test_type") == 0)
        res = parse_test_type(data, arg_value->str_val);
    else if (strcmp(arg_tag, "input") == 0)
        data->sensor_input_name = strdup(blts_config_boxed_value_get_string(arg_value));
    else if (strcmp(arg_tag, "duration") == 0)
        data->duration = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "max_noise") == 0)
        data->max_noise = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "range_min") == 0)
        data->range_min = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "range_max") == 0)
        data->range_max = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "range_deviation") == 0)
        data->range_deviation = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "input_is_polled") == 0)
        data->input_is_polled = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "axis_rotation_threshold") == 0)
        data->axis_rotation_threshold = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "axis_index") == 0)
        data->axis_index = blts_config_boxed_value_get_int(arg_value);
    else if (strcmp(arg_tag, "expected_value") == 0)
        data->expected_value = blts_config_boxed_value_dup(arg_value);
    else if (strcmp(arg_tag, "max_rate_difference_pct") == 0)
        data->max_rate_difference_pct = blts_config_boxed_value_get_int(arg_value);
    else if (strncmp(arg_tag, "plugin_", 7) == 0)
    {
        const char* plugin_var_name = arg_tag + 7;
        const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(plugin_var_name);
        if (var_spec->set_value)
        {
            var_spec->set_value(data->plugin_private_data, arg_value);
        }
        else
        {
            BLTS_ERROR("Plug-in variable %s is not settable\n", plugin_var_name);
            res = -1;
        }
    }
    else
    {
        BLTS_ERROR("Unknown test argument \"%s\"\n", arg_tag);
        res = -1;
    }

    return res;
}

static void* sensors_arg_handler(struct boxed_value* arg_tags, struct boxed_value* args, void* user_ptr)
{
    sensors_data* data = (sensors_data*)user_ptr;

    while (arg_tags && args)
    {
        parse_arg(data, blts_config_boxed_value_get_string(arg_tags), args);
        arg_tags = arg_tags->next;
        args = args->next;
    }

    return user_ptr;
}

/* Return NULL in case of an error */
static void* sensors_argument_processor(int argc, char **argv)
{
	int ret;
    int plugin_arg_found = 0;
    int c;

    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'p':
            plugin_arg_found = 1;
            break;
        }
    }

    if (!plugin_arg_found)
        return NULL;

    int case_num = 0;
    while (my_data->testcases[case_num].case_name != 0)
    {
        ret = blts_config_declare_variable_test_dynamic(my_data->testcases[case_num].case_name, sensors_arg_handler);
        if (ret)
            return NULL;
        case_num++;
    }

    return my_data;
}

static void update_input_min_max(const struct plugin_variable_spec* var_spec, struct boxed_value* sensor_values, int* min, int* max)
{
    for (int i = 0; i < var_spec->num_values; i++)
    {
        if (sensor_values[i].int_val < min[i])
            min[i] = sensor_values[i].int_val;
        if (sensor_values[i].int_val > max[i])
            max[i] = sensor_values[i].int_val;
    }
}

static int measure_input_min_max(sensors_data* data, int** min_out, int** max_out)
{
    int ret = 0;

    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    int* min = malloc(var_spec->num_values * sizeof(int));
    int* max = malloc(var_spec->num_values * sizeof(int));

    *min_out = min;
    *max_out = max;

    for (int i = 0; i < var_spec->num_values; i++)
    {
        min[i] = INT_MAX;
        max[i] = INT_MIN;
    }

    int timelimit = time(NULL) + data->duration;
    struct boxed_value* sensor_values = malloc(var_spec->num_values * sizeof(struct boxed_value));

    while (time(NULL) < timelimit)
    {
        if (var_spec->get_value(data->plugin_private_data, sensor_values) == -1)
        {
            BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
            ret = -1;
            goto done;
        }

        update_input_min_max(var_spec, sensor_values, min, max);
    }

done:
    free(sensor_values);

    return ret;
}

static int test_noise(sensors_data* data)
{
    int ret;
    int* min = NULL;
    int* max = NULL;

    BLTS_DEBUG("Measuring sensor input noise for %d seconds\n", data->duration);

    ret = measure_input_min_max(data, &min, &max);
    if (ret == 0)
    {
        const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
        for (int i = 0; i < var_spec->num_values; i++)
        {
            BLTS_TRACE("Noise on axis %d: %d (max noise %d, min value %d, max value %d)\n",
                i, max[i] - min[i], data->max_noise, min[i], max[i]);
            if (max[i] - min[i] > data->max_noise)
            {
                BLTS_ERROR("Noise exceeds maximum allowed value on axis %d (noise %d, maximum allowed %d)\n",
                    i, max[i] - min[i], data->max_noise);
                ret = -1;
                break;
            }
        }
    }

    free(max);
    free(min);

    return ret;
}

static int test_value(sensors_data* data)
{
    int ret = 0;

    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    assert(var_spec->num_values == 1);

    struct boxed_value sensor_value;
    if (var_spec->get_value(data->plugin_private_data, &sensor_value) == -1)
    {
        BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
        return -1;
    }

    if (var_spec->type == CONFIG_PARAM_INT)
    {
        int expected_value_int;
        char* endptr;
        errno = 0;
        expected_value_int = strtol(data->expected_value->str_val, &endptr, 10);
        if (errno == 0)
        {
            BLTS_TRACE("Expected value %d, input value %d\n", expected_value_int, sensor_value.int_val);
            ret = (sensor_value.int_val == expected_value_int) ? 0 : -1;
            if (ret == -1)
                BLTS_ERROR("Input value does not match expected value\n");
        }
        else
        {
            BLTS_ERROR("Expected value \"%s\" cannot be converted to an integer\n", data->expected_value->str_val);
            ret = -1;
        }
    }
    else if (var_spec->type == CONFIG_PARAM_FLOAT)
    {
        double expected_value_float;
        char* endptr;
        errno = 0;
        expected_value_float = strtod(data->expected_value->str_val, &endptr);
        if (errno == 0)
        {
            BLTS_TRACE("Expected value %f, input value %f\n", expected_value_float, sensor_value.float_val);
            ret = (sensor_value.float_val == expected_value_float) ? 0 : -1;
            if (ret == -1)
                BLTS_ERROR("Input value does not match expected value\n");
        }
        else
        {
            BLTS_ERROR("Expected value \"%s\" cannot be converted to a float\n", data->expected_value->str_val);
            ret = -1;
        }
    }
    else if (var_spec->type == CONFIG_PARAM_STRING)
    {
        BLTS_TRACE("Expected value %s, input value %s\n", data->expected_value->str_val, sensor_value.str_val);
        ret = (strcmp(sensor_value.str_val, data->expected_value->str_val) == 0) ? 0 : -1;
        if (ret == -1)
            BLTS_ERROR("Input value does not match expected value\n");
    }
    else
    {
        BLTS_ERROR("Unhandled variable type %d\n", var_spec->type);
        ret = -1;
    }

    return ret;
}

static int test_axis(sensors_data* data)
{
    int max_delta_axis, max_delta, prev_max_delta_axis = -1;
    struct boxed_value* sensor_values;
    int timelimit;
    int res = 0;
    int* min;
    int* max;

    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    if (data->axis_index < 0 || data->axis_index >= var_spec->num_values)
    {
        BLTS_ERROR("Invalid axis index %d\n", data->axis_index);
        return -1;
    }

    min = malloc(var_spec->num_values * sizeof(int));
    max = malloc(var_spec->num_values * sizeof(int));

    for (int i = 0; i < var_spec->num_values; i++)
    {
        min[i] = INT_MAX;
        max[i] = INT_MIN;
    }

    sensor_values = malloc(var_spec->num_values * sizeof(struct boxed_value));

    BLTS_DEBUG("Prepare the DUT for %c axis rotation/range test\n", 'X' + data->axis_index);

    sleep(5);

    BLTS_DEBUG("Rotate the %c axis 180 degrees\n", 'X' + data->axis_index);

    timelimit = time(NULL) + data->duration;
    while (time(NULL) < timelimit)
    {
        if (var_spec->get_value(data->plugin_private_data, sensor_values) == -1)
        {
            BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
            res = -1;
            goto done;
        }

        update_input_min_max(var_spec, sensor_values, min, max);

        max_delta_axis = 0;
        max_delta = max[0] - min[0];
        for (int i = 1; i < var_spec->num_values; i++)
        {
            if (max[i] - min[i] > max_delta)
            {
                max_delta_axis = i;
                max_delta = max[i] - min[i];
            }
        }

        if (max_delta > data->axis_rotation_threshold)
            break;
    }

    if (max_delta_axis == data->axis_index)
    {
        if (max_delta > data->axis_rotation_threshold)
        {
            BLTS_DEBUG("Rotation complete\n");
        }
        else
        {
            BLTS_ERROR("Axis range %d is less than required value %d\n", max_delta, data->axis_rotation_threshold);
            res = -1;
        }
    }
    else
    {
        BLTS_ERROR("Rotation detected on %c axis instead of %c axis\n",
            'X' + max_delta_axis, 'X' + data->axis_index);
        res = -1;
    }

done:
    free(sensor_values);
    free(max);
    free(min);

    return res;
}

static int test_range(sensors_data* data)
{
    int ret;
    int* min = NULL;
    int* max = NULL;

    BLTS_DEBUG("Measuring sensor input range for %d seconds\n", data->duration);

    ret = measure_input_min_max(data, &min, &max);
    if (ret == 0)
    {
        const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
        for (int i = 0; i < var_spec->num_values; i++)
        {
            if (abs(data->range_min - min[i]) > data->range_deviation || abs(data->range_max - max[i]) > data->range_deviation)
            {
                BLTS_ERROR("Measured range %d - %d does not match configured range %d - %d (+- %d)\n",
                    min[i], max[i], data->range_min, data->range_max, data->range_deviation);
                ret = -1;
            }
        }
    }

    free(max);
    free(min);

    return ret;
}

static int test_rate(sensors_data* data)
{
    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    int num_updates = 0;
    int observed_rate;
    struct boxed_value sensor_rate;
    struct boxed_value* sensor_values;
    struct boxed_value* prev_sensor_values;
    int timelimit;
    int ret = 0;

    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    const struct plugin_variable_spec* rate_spec = plugin_get_variable_spec("rate");
    if (rate_spec->get_value == NULL || rate_spec->get_value(data->plugin_private_data, &sensor_rate) == -1)
    {
        BLTS_ERROR("Cannot get sensor update rate\n");
        return -1;
    }

    sensor_values = malloc(var_spec->num_values * sizeof(struct boxed_value));
    prev_sensor_values = calloc(1, var_spec->num_values * sizeof(struct boxed_value));

    timelimit = time(NULL) + data->duration;
    while (time(NULL) < timelimit)
    {
        if (var_spec->get_value(data->plugin_private_data, sensor_values) == -1)
        {
            BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
            ret = -1;
            goto done;
        }

        if (data->input_is_polled)
        {
            // The input value is obtained with polling, so we don't know if it's
            // actually a new value. Assume that any new value is different from
            // the previous value, and only count changed values as updates.
            if (sensor_values[0].int_val != prev_sensor_values[0].int_val || sensor_values[1].int_val != prev_sensor_values[1].int_val || sensor_values[2].int_val != prev_sensor_values[2].int_val)
            {
                num_updates++;
                memcpy(prev_sensor_values, sensor_values, var_spec->num_values * sizeof(struct boxed_value));
            }
        }
    }

    observed_rate = num_updates / data->duration;
    BLTS_DEBUG("Observed update rate %d Hz, sensor rate %d Hz\n", observed_rate, sensor_rate.int_val);
    if (sensor_rate.int_val == 0)
    {
        // Special check for zero rate
        if (observed_rate != 0)
        {
            BLTS_ERROR("Observed rate does not match sensor rate\n");
            ret = -1;
            goto done;
        }
    }
    else
    {
        int rate_difference_pct = abs(sensor_rate.int_val - observed_rate) * 100 / sensor_rate.int_val;
        BLTS_DEBUG("Observed difference %d%%, max %d%%\n", rate_difference_pct, data->max_rate_difference_pct);
        if (rate_difference_pct > data->max_rate_difference_pct)
        {
            BLTS_ERROR("Observed update rate differs from sensor rate by %d %%, maximum allowed difference is %d %%\n",
                rate_difference_pct, data->max_rate_difference_pct);
            ret = -1;
            goto done;
        }
    }

done:
    free(sensor_values);

    return ret;
}

static int test_valuechange(sensors_data* data)
{
    int ret = 0;

    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    assert(var_spec->num_values == 1);

    struct boxed_value sensor_value_start;
    if (var_spec->get_value(data->plugin_private_data, &sensor_value_start) == -1)
    {
        BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
        return -1;
    }

    sleep(data->duration);

    struct boxed_value sensor_value_end;
    if (var_spec->get_value(data->plugin_private_data, &sensor_value_end) == -1)
    {
        BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
        return -1;
    }

    if (var_spec->type == CONFIG_PARAM_INT)
    {
        BLTS_TRACE("Start value %d, end value %d\n", sensor_value_start.int_val, sensor_value_end.int_val);
        ret = (sensor_value_start.int_val == sensor_value_end.int_val) ? -1 : 0;
        if (ret == -1)
            BLTS_ERROR("Input value did not change\n");
    }
    else if (var_spec->type == CONFIG_PARAM_FLOAT)
    {
        BLTS_TRACE("Start value %f, end value %f\n", sensor_value_start.float_val, sensor_value_end.float_val);
        ret = (sensor_value_start.float_val == sensor_value_end.float_val) ? -1 : 0;
        if (ret == -1)
            BLTS_ERROR("Input value did not change\n");
    }
    else if (var_spec->type == CONFIG_PARAM_STRING)
    {
        BLTS_TRACE("Start value \"%s\", end value \"%s\"\n", sensor_value_start.str_val, sensor_value_end.str_val);
        ret = (strcmp(sensor_value_start.str_val, sensor_value_end.str_val) == 0) ? -1 : 0;
        if (ret == -1)
            BLTS_ERROR("Input value did not change\n");
    }

    return ret;
}

static int test_novaluechange(sensors_data* data)
{
    int ret = 0;

    const struct plugin_variable_spec* var_spec = plugin_get_variable_spec(data->sensor_input_name);
    if (var_spec->get_value == NULL)
    {
        BLTS_ERROR("Plug-in variable %s is not gettable\n", data->sensor_input_name);
        return -1;
    }

    assert(var_spec->num_values == 1);

    struct boxed_value sensor_value_start;
    if (var_spec->get_value(data->plugin_private_data, &sensor_value_start) == -1)
    {
        BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
        return -1;
    }

    sleep(data->duration);

    struct boxed_value sensor_value_end;
    if (var_spec->get_value(data->plugin_private_data, &sensor_value_end) == -1)
    {
        BLTS_ERROR("Cannot get value for plug-in variable %s\n", data->sensor_input_name);
        return -1;
    }

    if (var_spec->type == CONFIG_PARAM_INT)
    {
        BLTS_TRACE("Start value %d, end value %d\n", sensor_value_start.int_val, sensor_value_end.int_val);
        ret = (sensor_value_start.int_val == sensor_value_end.int_val) ? 0 : -1;
        if (ret == -1)
            BLTS_ERROR("Input value changed\n");
    }
    else if (var_spec->type == CONFIG_PARAM_FLOAT)
    {
        BLTS_TRACE("Start value %f, end value %f\n", sensor_value_start.float_val, sensor_value_end.float_val);
        ret = (sensor_value_start.float_val == sensor_value_end.float_val) ? 0 : -1;
        if (ret == -1)
            BLTS_ERROR("Input value changed\n");
    }
    else if (var_spec->type == CONFIG_PARAM_STRING)
    {
        BLTS_TRACE("Start value \"%s\", end value \"%s\"\n", sensor_value_start.str_val, sensor_value_end.str_val);
        ret = (strcmp(sensor_value_start.str_val, sensor_value_end.str_val) == 0) ? 0 : -1;
        if (ret == -1)
            BLTS_ERROR("Input value changed\n");
    }

    return ret;
}

static int sensors_run_case(void* user_ptr, int test_num)
{
    int ret = 0;
    sensors_data* data = (sensors_data*)user_ptr;

    if (plugin_init_plugin(data->plugin_private_data) == -1)
        return -1;

    BLTS_DEBUG("Running test case %d\n", test_num);
//    BLTS_DEBUG("Config: test type %d input name %s duration %d\n",
//        data->test_type, data->sensor_input_name, data->duration);
    switch (data->test_type)
    {
    case INIT:
        // Already done above
        break;
    case NOISE:
        ret = test_noise(data);
        break;
    case TESTVALUE:
        ret = test_value(data);
        break;
    case RATE:
        ret = test_rate(data);
        break;
    case RANGE:
        ret = test_range(data);
        break;
    case AXIS:
        ret = test_axis(data);
        break;
    case VALUECHANGE:
        ret = test_valuechange(data);
        break;
    case NOVALUECHANGE:
        ret = test_novaluechange(data);
        break;
    default:
        BLTS_ERROR("Unhandled test type %d\n", data->test_type);
        ret = -1;
        break;
    }

    plugin_deinit_plugin(data->plugin_private_data);

    return ret;
}

/* user_ptr is what you returned from my_example_argument_processor */
static void sensors_teardown(void *user_ptr)
{
    BLTS_UNUSED_PARAM(user_ptr);
    if (my_data->plugin_private_data)
        plugin_deinit_plugin_data(my_data->plugin_private_data);
    if (plugin_handle)
        dlclose(plugin_handle);
    free(my_data);
}

#define LOAD_SYMBOL(var_name, symbol_type, symbol_name) \
{ \
    var_name = (symbol_type) dlsym(plugin_handle, #symbol_name); \
    const char *dlsym_error = dlerror(); \
    if (dlsym_error) \
    { \
        BLTS_ERROR("Cannot load symbol " #symbol_name ": %s\n", dlsym_error); \
        dlclose(plugin_handle); \
        return -1; \
    } \
}

static int app_init_once(const char* plugin_name, sensors_data *data)
{
    assert(plugin_name != NULL);
    assert(data != NULL);

    plugin_handle = dlopen(plugin_name, RTLD_LAZY);
    if (plugin_handle == NULL)
    {
        BLTS_ERROR("Cannot load plugin %s\n", plugin_name);
        return -1;
    }

    // Clear dlerror before trying to load symbols
    dlerror();

    LOAD_SYMBOL(plugin_init_plugin_data, init_plugin_data_t, init_plugin_data);
    LOAD_SYMBOL(plugin_deinit_plugin_data, deinit_plugin_data_t, deinit_plugin_data);
    LOAD_SYMBOL(plugin_init_plugin, init_plugin_t, init_plugin);
    LOAD_SYMBOL(plugin_deinit_plugin, deinit_plugin_t, deinit_plugin);
    LOAD_SYMBOL(plugin_sensors_cases, sensors_cases_t, sensors_cases);
    LOAD_SYMBOL(plugin_get_variable_spec, get_variable_spec_t, get_variable_spec);

    if (plugin_init_plugin_data(&data->plugin_private_data) != 0)
    {
        BLTS_ERROR("Plug-in data initialization failed\n");
    	return -1;
    }

    data->testcases = plugin_sensors_cases;

    return 0;
}

static blts_cli_testcase empty_cases[] =
{
	BLTS_CLI_END_OF_LIST
};

static blts_cli sensors_cli =
{
    .test_cases = empty_cases,
    .log_file = "blts_sensors_log.txt",
    .blts_cli_help = sensors_help,
    .blts_cli_process_arguments = sensors_argument_processor,
    .blts_cli_teardown = sensors_teardown
};

int main(int argc, char **argv)
{
    /* We have to parse the command line early to get the name of the plug-in
     * so that we can load the plug-in and get the list of test cases
     */

    /* blts_cli_main opens log file, but we need logging before it is called.
     * Open the log file temporarily here.
     */
    blts_log_open(sensors_cli.log_file,
	BLTS_LOG_FLAG_FILE | BLTS_LOG_FLAG_STDOUT);
    blts_log_set_level(LEVEL_DEBUG);

    my_data = malloc(sizeof(*my_data));
    memset(my_data, 0, sizeof(*my_data));

    const char* plugin_name = NULL;
    int i;
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            if (++i >= argc)
            {
                // Delay error reporting to the argument processing function
                // to get complete help message
                break;
            }

            plugin_name = argv[i];
            break;
        }
    }

    if (plugin_name != NULL)
    {
        if (app_init_once(plugin_name, my_data) != 0)
        {
            free(my_data);
            return 1;
        }
        sensors_cli.test_cases = (blts_cli_testcase*) my_data->testcases;
        i = 0;
        while (sensors_cli.test_cases[i].case_name != NULL)
            sensors_cli.test_cases[i++].case_func = sensors_run_case;
    }

    blts_log_close();

    return blts_cli_main(&sensors_cli, argc, argv);
}
