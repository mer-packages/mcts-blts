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
#include <geoclue/geoclue-address.h>
#include "test_geoclue_def.h"
#include "test_geoclue_utils.h"

GcTestAddress* test_address_async;
GcTestAccuracy* test_address_async_accuracy;

static void test_get_address_async_country ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned country with expected result*/
	test_address_check_details ( test_address_async,
	                             GEOCLUE_ADDRESS_KEY_COUNTRY );
}


static void test_get_address_async_country_code ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned country code with expected result*/
	test_address_check_details ( test_address_async,
	                             GEOCLUE_ADDRESS_KEY_COUNTRYCODE );
}


static void test_get_address_async_region ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned region with expected result*/
	test_address_check_details ( test_address_async, GEOCLUE_ADDRESS_KEY_REGION
	                           );
}


static void test_get_address_async_locality ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned locality with expected result*/
	test_address_check_details ( test_address_async,
	                             GEOCLUE_ADDRESS_KEY_LOCALITY );
}

static void test_get_address_async_area ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned area with expected result*/
	test_address_check_details ( test_address_async, GEOCLUE_ADDRESS_KEY_AREA );
}


static void test_get_address_async_postal_code()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned postalcode with expected result*/
	test_address_check_details ( test_address_async,
	                             GEOCLUE_ADDRESS_KEY_POSTALCODE );

}
static void test_get_address_async_street()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address_async != NULL );
	/*check returned street with expected result*/
	test_address_check_details ( test_address_async, GEOCLUE_ADDRESS_KEY_STREET
	                           );
}


void test_get_address_async_accuracy_level ()
{

	/*Wait for obtaining potition*/
	wait();
	/*check returned accuracy_level with expected result*/
	g_assert ( test_address_async_accuracy != NULL );
	test_accuracy_check_details ( test_address_async_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_LEVEL );
}

void test_get_address_async_accuracy_horizontal ()
{

	/*Wait for obtaining potition*/
	wait();
	/*check returned horizontal_accurary with expected result*/
	g_assert ( test_address_async_accuracy != NULL );
	test_accuracy_check_details ( test_address_async_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL );
}

void test_get_address_async_accuracy_vertical ()
{

	/*Wait for obtaining potition*/
	wait();
	/*check returned vertical_accuracy with expected result*/
	g_assert ( test_address_async_accuracy != NULL );
	test_accuracy_check_details ( test_address_async_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_VERTICAL );
}


void callback_test_address_async ( GeoclueAddress *address,int
                                   timestamp,GHashTable *details,GeoclueAccuracy
                                   *accuracy,GError
                                   *error,gpointer userdata )
{
	/*assert no error occurs*/
	if ( error )
	{
		g_test_message ( "Error in get address in async mode: %s.\n",
		                 error->message );
		g_error_free ( error );
		g_assert_not_reached();
	}

	/*setup test address and accuracy in async mode*/
	g_assert ( details != NULL );
	test_address_async = test_address_record ( details );
	g_assert ( test_address_async != NULL );
	test_address_async_accuracy = test_accuracy_record ( accuracy );

	/*notify other test cases*/
	notify();

	/*quit from mainloop_test_address_async*/
	g_main_loop_quit ( ( GMainLoop * ) userdata );
}

static void test_get_address_async()
{
	GeoclueAddress *address = create_geoclue_address ( PROVIDER_TEST );
	g_assert ( address != NULL );

	/*prepare main loop*/
	GMainLoop *mainloop_test_address_async = g_main_loop_new ( NULL, FALSE );
	g_assert ( mainloop_test_address_async != NULL );

	/*get position asynchronously*/
	geoclue_address_get_address_async ( address,
	                                    ( GeoclueAddressCallback )
	                                    callback_test_address_async,
	                                    mainloop_test_address_async );
	g_print ( "===INFO:Get address asynchronously, in main loop now...===\n" );
	g_main_loop_run ( mainloop_test_address_async );

	/*unref objects*/
	g_main_loop_unref ( mainloop_test_address_async );
	g_object_unref ( address );
}

int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );

	test_setup();
	/*add test cases*/
	g_test_add_func ( "/geoclue/geoclue_address/new/test_get_address_async",
	                  test_get_address_async );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async",test_get_address_async );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_country_code" ,
	    test_get_address_async_country_code );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_country",test_get_address_async_country );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_region",test_get_address_async_region );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_locality",test_get_address_async_locality );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_area",test_get_address_async_area );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_postal_code",test_get_address_async_postal_code
	);
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_street",test_get_address_async_street );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_accuracy_level",
	    test_get_address_async_accuracy_level ) ;
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_accuracy_horizontal",
	    test_get_address_async_accuracy_horizontal );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address_async/\
	    test_get_address_async_accuracy_vertical",
	    test_get_address_async_accuracy_vertical );

	/*run test cases*/
	ret=g_test_run();

	test_tear_down();

	return ret;

}
