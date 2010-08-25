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

GcTestAddress* test_address = NULL;
GcTestAccuracy* test_address_accuracy = NULL;


static void test_get_address_country ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned country with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_COUNTRY );
}


static void test_get_address_country_code ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned country code with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_COUNTRYCODE
	                           );
}


static void test_get_address_region ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned region with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_REGION );
}


static void test_get_address_locality ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned locality with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_LOCALITY );
}

static void test_get_address_area ()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned area with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_AREA );
}


static void test_get_address_postal_code()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned postalcode with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_POSTALCODE );

}
static void test_get_address_street()
{
	/*wait for obtaining address*/
	wait();
	g_assert ( test_address != NULL );
	/*check returned street with expected result*/
	test_address_check_details ( test_address, GEOCLUE_ADDRESS_KEY_STREET );
}


void test_get_address_accuracy_level ()
{

	/*Wait for obtaining address*/
	wait();
	/*check returned accuracy_level with expected result*/
	g_assert ( test_address_accuracy != NULL );
	test_accuracy_check_details ( test_address_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_LEVEL );
}

void test_get_address_accuracy_horizontal ()
{

	/*Wait for obtaining address*/
	wait();
	/*check returned horizontal_accurary with expected result*/
	g_assert ( test_address_accuracy != NULL );
	test_accuracy_check_details ( test_address_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL );
}


void test_get_address_accuracy_vertical ()
{

	/*Wait for obtaining address*/
	wait();
	/*check returned vertical_accuracy with expected result*/
	g_assert ( test_address_accuracy != NULL );
	test_accuracy_check_details ( test_address_accuracy,
	                              TEST_ACCURACY_KEY_ACCURACY_VERTICAL );
}

static void test_get_address()
{
	GeoclueAddress *address = create_geoclue_address ( PROVIDER_TEST );
	g_assert ( address != NULL );

	GHashTable *details = NULL;
	GError *error = NULL;
	int timestamp;
	GeoclueAccuracy* accuracy = NULL;

	geoclue_address_get_address ( address,&timestamp,&details,&accuracy,&error
	                            );

	/*assert no error occurs*/
	if ( error )
	{
		g_test_message ( "Error in get address: %s.\n", error->message );
		g_assert_not_reached();
	}

	/*setup test address details and accuracy*/
	g_assert ( details != NULL );
	test_address = test_address_record ( details );
	g_assert ( test_address != NULL );
	test_address_accuracy = test_accuracy_record ( accuracy );

	/*notify other test cases*/
	notify();

	/*unref objects*/
	g_hash_table_destroy ( details );
	geoclue_accuracy_free ( accuracy );
	g_object_unref ( address );
}

/*

static void test_geoclue_address_new_null_service()
{
	GeoclueAddress *address = geoclue_address_new(NULL,
		"/org/freedesktop/Geoclue/Providers/Hostip");
	g_assert(address == NULL);
}

static void test_geoclue_address_new_null_path()
{
	GeoclueAddress *address =
		geoclue_address_new("org.freedesktop.Geoclue.Providers.Hostip", NULL);
	g_assert(address == NULL);
}
*/

int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();

	g_test_init ( &argc,&argv,NULL );

	g_assert ( argc==1 );
	test_setup();

	/*add test cases*/
	g_test_add_func (
	    "/geoclue/geoclue_address/new/test_get_address",test_get_address );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address",test_get_address
	);
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_country_code",
	    test_get_address_country_code );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_country",
	    test_get_address_country );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_region",
	    test_get_address_region );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_locality",
	    test_get_address_locality );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_area",
	    test_get_address_area );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_postal_code",
	    test_get_address_postal_code );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_street",
	    test_get_address_street );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/test_get_address_accuracy_level",
	    test_get_address_accuracy_level );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/\
	    test_get_address_accuracy_horizontal",
	    test_get_address_accuracy_horizontal );
	g_test_add_func (
	    "/geoclue/geoclue_address/get_address/\
	    test_get_address_accuracy_vertical",
	    test_get_address_accuracy_vertical );

	/*negtive test cases*/
	/*g_test_add_func("/geoclue/geoclue_address/negative/
	test_geoclue_address_new_null_service",test_geoclue_address_new_null_service)
	;
	g_test_add_func("/geoclue/geoclue_address/negative/
	test_geoclue_address_new_null_path",test_geoclue_address_new_null_path);*/

	/*run test cases*/
	test_tear_down();
	ret=g_test_run();

	return ret;


}
