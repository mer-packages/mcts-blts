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
#include <geoclue/geoclue-master.h>
#include "test_geoclue_utils.h"

GcTestPosition* test_position_async;
GcTestPosition* test_position_async_accuracy;

static void test_create_position_async_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned country with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_LATITUDE );
}


static void test_create_position_async_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned country code with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_LONGITUDE );
}


static void test_create_position_async_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned region with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_ALTITUDE );
}


static void test_create_position_async_fields_latitude()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned locality with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_LATITUDE );
}

static void test_create_position_async_fields_longitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_LONGITUDE );
}

static void test_create_position_async_fields_altitude ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_position_async != NULL );
	
	/*check returned area with expected result*/
	test_position_check_details ( test_position_async,
		TEST_POSITION_KEY_FIELDS_ALTITUDE );
}


static void test_create_position_async_accuracy_level ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned accuracy_level with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_LEVEL );
}

static void test_create_position_async_accuracy_horizontal ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned horizontal_accurary with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL );
}

static void test_create_position_async_accuracy_vertical ()
{

	/*Wait for obtaining potition*/
	wait();
	
	/*check returned vertical_accuracy with expected result*/
	g_assert ( test_position_async_accuracy != NULL );
	test_accuracy_check_details ( test_position_async_accuracy,
		TEST_ACCURACY_KEY_ACCURACY_VERTICAL );
}


static void callback_create_position ( GeoclueMasterClient *client,
                                       GeocluePosition *position,
                                       GError *error,
                                       gpointer userdata )
{
	/*assert no error occurs*/
	if ( error ) {
		g_test_message ( "Error getting position: %s\n", error->message );
		g_assert_not_reached();
	}

	g_assert ( position != NULL );
	g_assert ( GEOCLUE_IS_POSITION ( position ) );
	
	/*get coordinate value from geoclue_position*/
	int timestamp;
	double lat,lon,alt;;
	GeoclueAccuracy *accuracy = NULL;
	GeocluePositionFields fields=geoclue_position_get_position (
		position,&timestamp,&lat,&lon,&alt,&accuracy,&error );
	if ( error ) {
		g_test_message ( "Error in geocode_master_client: %s.\n", error->message
			);
		g_assert_not_reached();
	}

	/*setup test position*/
	test_position_async = test_position_record ( &lat, &lon, &alt, fields );
	g_assert ( test_position_async != NULL );
	test_position_async_accuracy = test_accuracy_record ( accuracy );
	
	/*notify other test cases*/
	notify();
	
	/*quit from mainloop_test_position_async*/
	g_main_loop_quit ( ( GMainLoop * ) userdata );
}

static void  test_geoclue_master_client_create_position_async()
{
	GeoclueMasterClient* client = create_geoclue_master_client();
	g_assert ( client != NULL );
	
	/*prepare main loop*/
	GMainLoop *mainloop_test_loop = g_main_loop_new ( NULL, FALSE );
	g_assert ( mainloop_test_loop != NULL );
	
	/*get position asynchronously*/
	geoclue_master_client_create_position_async ( client, (
		CreatePositionCallback ) callback_create_position,  mainloop_test_loop
		);
	g_main_loop_run ( mainloop_test_loop );

	/*unref objects*/
	g_main_loop_unref ( mainloop_test_loop );
	g_object_unref ( client );

}


int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );
	test_setup();

	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		test_geoclue_master_client_create_position_async",
		test_geoclue_master_client_create_position_async );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_latitude",test_create_position_async_latitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_longitude",test_create_position_async_longitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_altitude",test_create_position_async_altitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_fields_latitude",
		test_create_position_async_fields_latitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_fields_longitude",
		test_create_position_async_fields_longitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_fields_altitude",
		test_create_position_async_fields_altitude );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_accuracy_level",
		test_create_position_async_accuracy_level );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_accuracy_horizontal",
		test_create_position_async_accuracy_horizontal );
	g_test_add_func (
		"/geoclue/geoclue_master_client/create_position_async/\
		create_position_async_accuracy_vertical",
		test_create_position_async_accuracy_vertical );
	
	/*run test cases*/
	ret=g_test_run();
	test_tear_down();

	return ret;
}
