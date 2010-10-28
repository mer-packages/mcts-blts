/* blts_sensor_plugin.h -- Sensor plug-in API declarations

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

#ifndef BLTS_SENSOR_PLUGIN_H
#define BLTS_SENSOR_PLUGIN_H

#include <blts_params.h>
#include <blts_cli_frontend.h>

#include <blts_sensor_variable.h>

/* Allocates and initializes plug-in-specific data. The plugin_data
 * argument should point to the allocated data structure when the
 * function returns.
 */
int init_plugin_data(void** plugin_data);

/* Deallocates the plug-in-specific data structure.
 */
int deinit_plugin_data(void* plugin_data);

/* Initializes the plug-in, e.g. opens the sensor device.
 */
int init_plugin(void* plugin_data);

/* Deinitializes the plug-in, e.g. closes the sensor device.
 */
int deinit_plugin(void* plugin_data);

/* Returns the plug-in variable spec for the variable with the requested name.
 */
const struct plugin_variable_spec* get_variable_spec(const char* var_name);

/* Contains the test case definitions for the plug-in. The case_func member
 * of each blts_cli_testcase instance is set by the front-end when the plug-in
 * is loaded, and can be set as NULL by the plug-in.
 */
extern blts_cli_testcase sensors_cases[];

#endif // BLTS_SENSOR_PLUGIN_H
