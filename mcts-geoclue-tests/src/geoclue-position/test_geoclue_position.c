/*
#  Copyright (C) 2010, Intel Corporation
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation; either version 2 of the License,
#  or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA.
#
#   Authors:
#        Mu, Qin  <qin.mu@intel.com>
*/


#include <glib.h>
#include <geoclue/geoclue-position.h>
#include "test_geoclue_def.h"
#include "test_geoclue_utils.h"

GcTestPosition* test_position;
GcTestAccuracy* test_position_accuracy;

static void test_get_position_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned country with expected result*/
	test_position_check_details ( test_position, TEST_POSITION_KEY_LATITUDE );
}


static void test_get_position_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned country code with expected result*/
	test_position_check_details ( test_position, TEST_POSITION_KEY_LONGITUDE );
}


static void test_get_position_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned region with expected result*/
	test_position_check_details ( test_position, TEST_POSITION_KEY_ALTITUDE );
}


static void test_get_position_fields_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned locality with expected result*/
	test_position_check_details ( test_position,
		TEST_POSITION_KEY_FIELDS_LATITUDE );
}

static void test_get_position_fields_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position,
		TEST_POSITION_KEY_FIELDS_LONGITUDE );
}

static void test_get_position_fields_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position,
		TEST_POSITION_KEY_FIELDS_ALTITUDE );
}


static void test_get_position_accuracy_level ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned accuracy_level with expected result*/
	g_assert ( test_position_accuracy != NULL );
	test_accuracy_check_details ( test_position_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_LEVEL );
}

static void test_get_position_accuracy_horizontal ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned horizontal_accurary with expected result*/
	g_assert ( test_position_accuracy != NULL );
	test_accuracy_check_details ( test_position_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL );
}

static void test_get_position_accuracy_vertical ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned vertical_accuracy with expected result*/
	g_assert ( test_position_accuracy != NULL );
	test_accuracy_check_details ( test_position_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_VERTICAL );
}

static void  test_get_position()
{
	/*create position from geoclue test provider*/
	GeocluePosition *pos = create_geoclue_position ( PROVIDER_TEST );
	g_assert ( pos != NULL );
	
	/*get position*/
	int timestamp;
	double lat,lon,alt;;
	GeoclueAccuracy *accuracy = NULL;
	GError *error = NULL;

	GeocluePositionFields fields=geoclue_position_get_position (
		pos,&timestamp,&lat,&lon,&alt,&accuracy,&error );

	if ( error ) {
		g_test_message ( "Error in get_position: %s.\n", error->message );
		g_error_free ( error );
		g_object_unref ( pos );
		g_assert ( NULL );
	}

	/*record returned position and accuracy data*/
	test_position = test_position_record ( &lat,&lon,&alt, fields );
	g_assert ( test_position != NULL );
	test_position_accuracy = test_accuracy_record ( accuracy );
	g_assert ( test_position_accuracy != NULL );

	/*notify other test cases*/
	notify();
	
	/*unref objects*/
	geoclue_accuracy_free ( accuracy );
	g_object_unref ( pos );
}


int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );
	test_setup();

	g_test_add_func (
		"/geoclue/geoclue_position/new/test_get_position",test_get_position );
	g_test_add_func (
"/geoclue/geoclue_position/get_position/\
			test_get_position",test_get_position
);
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/test_get_position_latitude",
		test_get_position_latitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/test_get_position_longitude",
		test_get_position_longitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/test_get_position_altitude",
		test_get_position_altitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
		test_get_position_fields_latitude",
		test_get_position_fields_latitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
		test_get_position_fields_longitude",
		test_get_position_fields_longitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
		test_get_position_fields_altitude",
		test_get_position_fields_altitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
			test_get_position_accuracy_level",
		test_get_position_accuracy_level );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
			test_get_position_accuracy_horizontal",
		test_get_position_accuracy_horizontal );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position/\
			test_get_position_accuracy_vertical",
		test_get_position_accuracy_vertical );
	
	/*run test cases*/
	ret=g_test_run();
	test_tear_down();
	return ret;
}
