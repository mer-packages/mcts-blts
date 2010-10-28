/* sensor-plugin-example.c -- Sensor plug-in example for test development

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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include <blts_cli_frontend.h>
#include <blts_log.h>
#include <blts_params.h>

#include <blts_sensor_plugin.h>

// Local declarations
struct plugin_data {
    int max_value;
    int seed_value;
};

static int get_value(void* data, struct boxed_value value[]);
static int set_max_value(void* data, struct boxed_value value[]);
static int set_seed_value(void* data, struct boxed_value value[]);
static int get_seed_value(void* data, struct boxed_value value[]);

// Test case data
blts_cli_testcase sensors_cases[] =
{
    /* Test case name, test case function, timeout in ms
     * It is possible to use same function for multiple cases.
     * Zero timeout = infinity */
    /* The case_func member is set by the plug-in loader */
    { "Example init test", NULL, 60000 },
    { "Example range test 1", NULL, 60000 },
    { "Example range test 2", NULL, 60000 },
    { "Example noise test", NULL, 60000 },
    { "Example no value change test", NULL, 60000 },
    { "Example value change test", NULL, 60000 },
    { "Example value check test", NULL, 60000 },

    BLTS_CLI_END_OF_LIST
};

// Plug-in API data
struct plugin_variable_spec plugin_variable_specs[] =
{
    { "value", 1, CONFIG_PARAM_INT, get_value, NULL },
    { "seed_value", 1, CONFIG_PARAM_INT, get_seed_value, set_seed_value },
    { "max_value", 3, CONFIG_PARAM_INT, NULL, set_max_value },
    { NULL, 0, CONFIG_PARAM_NONE, NULL, NULL }
};

// Plug-in API functions
const struct plugin_variable_spec* get_variable_spec(const char* var_name)
{
    int i = 0;
    while (plugin_variable_specs[i].name != NULL)
    {
        if (strcmp(plugin_variable_specs[i].name, var_name) == 0)
            break;
        i++;
    }

    if (plugin_variable_specs[i].name == NULL)
        return NULL;

    return &plugin_variable_specs[i];
}

int init_plugin_data(void** data)
{
    *data = calloc(1, sizeof(struct plugin_data));

    return 0;
}

int deinit_plugin_data(void* data)
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    free(private_data);

    return 0;
}

int init_plugin(void* data)
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    srand(private_data->seed_value);

    return 0;
}

int deinit_plugin(void* data)
{
    return 0;
}

// Plug-in variable setters and getters
static int set_max_value(void* data, struct boxed_value value[])
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    private_data->max_value = blts_config_boxed_value_get_int(&value[0]);

    return 0;
}

static int set_seed_value(void* data, struct boxed_value value[])
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    private_data->seed_value = blts_config_boxed_value_get_int(&value[0]);

    return 0;
}

static int get_seed_value(void* data, struct boxed_value value[])
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    value[0].type = CONFIG_PARAM_INT;
    value[0].int_val = private_data->seed_value;

    return 0;
}

static int get_value(void* data, struct boxed_value value[])
{
    struct plugin_data* private_data = (struct plugin_data*)data;

    value[0].type = CONFIG_PARAM_INT;
    value[0].int_val = rand() % (private_data->max_value + 1);

    return 0;
}
