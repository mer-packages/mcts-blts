/* blts_sensor_variable.h -- Sensor API variable spec declaration

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

#ifndef BLTS_SENSOR_VARIABLE_H
#define BLTS_SENSOR_VARIABLE_H

struct plugin_variable_spec {
    const char* name; /* Name of the variable */
    int num_values; /* Number of values provided by the variable */
    enum config_param_type type; /* Type of the variable */
    int (*get_value)(void* data, struct boxed_value value[]); /* Pointer to a function for getting the value of the variable */
    int (*set_value)(void* data, struct boxed_value value[]); /* Pointer to a function for setting the value of the variable */
};

#endif // BLTS_SENSOR_VARIABLE_H
