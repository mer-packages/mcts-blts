/*
 * Copyright (C) 2010, Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with self library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.

 * Authors:
 *         Mu, Qin  <qin.mu@intel.com>
 */
#ifndef _TEST_GEOCLUE_UTILS_H
#define _TEST_GEOCLUE_UTILS_H

#include <glib.h>
#include <geoclue/geoclue-types.h>

G_BEGIN_DECLS


#include <geoclue/geoclue-types.h>
#include <geoclue/geoclue-address.h>
#include <geoclue/geoclue-position.h>
#include <geoclue/geoclue-geocode.h>
#include <geoclue/geoclue-master.h>
#include "test_geoclue_def.h"

/*test position utilites*/
GcTestPosition* test_position_record ( double* latitude,  double* longitude, double* altitude, GeocluePositionFields fields );
void test_position_free ( GcTestPosition* test_position );
double test_position_get_property ( GcTestPosition* test_position, TestPositionProperty property );
void test_position_check_details ( GcTestPosition* test_position, TestPositionProperty property ) ;
void test_position_check_details_full ( GcTestPosition* real, TestPositionProperty property, GcTestPosition* expected );

/*test address utilites*/
GcTestAddress*  test_address_record ( GHashTable* address_details );
void test_address_free ( GcTestAddress* test_address );
gchar* test_address_get_property ( GcTestAddress* test_address, TestAddressProperty property );
void test_address_check_details ( GcTestAddress* test_address, TestAddressProperty property );
void test_address_check_details_full ( GcTestAddress* real, TestAddressProperty property, GcTestAddress* expected );
void compare_address_details ( GHashTable* source, GHashTable* target );
void insert_address_key_and_value ( char *key, char *value, GHashTable *target );

/*test accuracy utilities*/
GcTestAccuracy* test_accuracy_record ( GeoclueAccuracy* accuracy );
void test_accuracy_free ( GcTestAccuracy* test_accuracy );
double test_accuracy_get_property ( GcTestAccuracy* test_accuracy, TestAccuracyProperty property );
void test_accuracy_check_details ( GcTestAccuracy* test_accuracy, TestAccuracyProperty property );
void test_accuracy_check_details_full ( GcTestAccuracy* real, TestAccuracyProperty property, GcTestAccuracy* expected );

/*test location utilities*/
GcTestAccuracy* test_location_to_accuray ( GcTestLocation* location );
GcTestPosition* test_location_to_position ( GcTestLocation* location );
GcTestAddress* test_location_to_address ( GcTestLocation* location );

/*test sychronization utilites*/
void wait();
void notify();

/*test setup and teardown*/
void test_setup();
void test_tear_down();

/*test provider setup and configure utilites*/
void geoclue_test_provider_setup ();
void geoclue_test_provider_tear_down ();
gboolean geoclue_test_provider_set_position ( GcTestPosition* position, GcTestAccuracy* accuracy );
gboolean geoclue_test_provider_set_address ( GcTestAddress* address );

/*geoclue object creation utilities*/
GeocluePosition* create_geoclue_position ( TestProviderKey key );
GeoclueAddress* create_geoclue_address ( TestProviderKey key );
GeoclueGeocode* create_geoclue_geocode ( TestProviderKey key );
GeoclueProvider* create_geoclue_provider ( TestProviderKey key, TestProviderType type );
GeoclueMasterClient* create_geoclue_master_client();

G_END_DECLS

#endif
