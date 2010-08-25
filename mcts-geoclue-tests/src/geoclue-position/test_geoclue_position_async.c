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
#include "test_geoclue_utils.h"

GcTestPosition* test_position_async;
GcTestPosition* test_position_async_accuracy;

static void test_get_position_async_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned country with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_LATITUDE );
}


static void test_get_position_async_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned country code with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_LONGITUDE );
}


static void test_get_position_async_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned region with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_ALTITUDE );
}


static void test_get_position_async_fields_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned locality with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_LATITUDE );
}

static void test_get_position_async_fields_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_LONGITUDE );
}

static void test_get_position_async_fields_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_ALTITUDE );
}


static void test_get_position_async_accuracy_level ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned accuracy_level with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_LEVEL );
}

static void test_get_position_async_accuracy_horizontal ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned horizontal_accurary with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL );
}

static void test_get_position_async_accuracy_vertical ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned vertical_accuracy with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_VERTICAL );
}

static void callback_test_position_async ( GeocluePosition      *pos,
        GeocluePositionFields fields,
        int                   timestamp,
        double                latitude,
        double                longitude,
        double                altitude,
        GeoclueAccuracy      *accuracy,
        GError               *error,
        gpointer              userdata )
{
	/*assert no error occurs*/
	if ( error ) {
		g_print ( "Error getting position: %s\n", error->message );
		g_test_message ( "Error getting position: %s\n", error->message );
		g_assert_not_reached();
	}

	/*setup test position*/
	test_position_async = test_position_record ( &latitude, &longitude,
		&altitude, fields );
	g_assert ( test_position_async != NULL );
	test_position_async_accuracy = test_accuracy_record ( accuracy );

	/*notify other test cases*/
	notify();

	/*quit from mainloop_test_position_async*/
	g_main_loop_quit ( ( GMainLoop * ) userdata );
}

static void  test_get_position_async()
{
	/*create position from geoclue test provider*/
	GeocluePosition *pos = create_geoclue_position ( PROVIDER_TEST );
	g_assert ( pos != NULL );
	
	/*prepare main loop*/
	GMainLoop *mainloop_test_position_async = g_main_loop_new ( NULL, FALSE );
	g_assert ( mainloop_test_position_async != NULL );
	
	/*get position asynchronously*/
	geoclue_position_get_position_async ( pos, ( GeocluePositionCallback )
		callback_test_position_async,  mainloop_test_position_async );
	g_main_loop_run ( mainloop_test_position_async );

	/*unref objects*/
	g_main_loop_unref ( mainloop_test_position_async );
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
		"/geoclue/geoclue_position/new/test_get_position_async",
		test_get_position_async );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/test_get_position_async",
		test_get_position_async );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_latitude" , test_get_position_async_latitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_longitude" ,test_get_position_async_longitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_altitude", test_get_position_async_altitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_fields_latitude",
		test_get_position_async_fields_latitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_fields_longitude",
		test_get_position_async_fields_longitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_fields_altitude",
		test_get_position_async_fields_altitude );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_accuracy_level",
		test_get_position_async_accuracy_level );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_accuracy_horizontal",
		test_get_position_async_accuracy_horizontal );
	g_test_add_func (
		"/geoclue/geoclue_position/get_position_async/\
		test_get_position_async_accuracy_vertical",
		test_get_position_async_accuracy_vertical );
	
	/*run test cases*/
	ret=g_test_run();
	test_tear_down();

	return ret;
}
