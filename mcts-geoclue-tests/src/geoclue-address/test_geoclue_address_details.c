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

GHashTable* test_address_details;

static void test_address_details_new ()
{
	GHashTable* address_details = geoclue_address_details_new();
	g_assert ( g_hash_table_size ( address_details ) == 0 );
	g_hash_table_unref ( address_details );
}

static void test_address_details_copy ()
{
	g_assert ( test_address_details != NULL );

	/*check the copied address details with source*/
	GHashTable* copy_details = geoclue_address_details_copy (
	                               test_address_details );
	compare_address_details ( test_address_details, copy_details );

	g_hash_table_unref ( copy_details );
}
/*require geoclue-0.12
static void test_address_details_insert () {
    g_assert(test_address_details != NULL);

    //check the inserted address details with source
    GHashTable* new_details = geoclue_address_details_new();
	g_hash_table_foreach (test_address_details,
	                      (GHFunc)insert_address_key_and_value,
	                      new_details);
    compare_address_detail(test_address_details, new_details);

    g_hash_table_unref(new_details);
}
*/
/* require geoclue-0.12
static void test_address_details_set_country_from_code() {
    g_assert(test_address_details != NULL);

    //check the country with expected value
    GHashTable* new_details = geoclue_address_details_new();
    char* country_code = test_address_get_propery(test_address_details,
TEST_ADDRESS_KEY_COUNTRY_CODE);
    char* country = test_address_get_propery(test_address_details,
TEST_ADDRESS_KEY_COUNTRY);

    geoclue_address_details_insert(new_details, GEOCLUE_ADDRESS_KEY_COUNTRYCODE,
country_code);
    geoclue_address_details_set_country_from_code(new_details);
    g_assert_cmpstr (g_hash_table_lookup(new_details,
GEOCLUE_ADDRESS_KEY_COUNTRY), ==, country);

    g_hash_table_unref(new_details);
}
*/

static void
test_address_details_get_accuracy_level()
{
	g_assert ( test_address_details != NULL );

	/*check the accrary_level with expected value*/
	GeoclueAccuracyLevel level = geoclue_address_details_get_accuracy_level (
	                                 test_address_details );
	g_assert_cmpint ( level, ==, TEST_LOCATION.accuracy.accuracy_level );
}



/*get test address details*/
static void test_address_details_setup()
{
	test_address_details = test_location_to_address ( &TEST_LOCATION );
}

static void test_address_details_tear_down()
{
	if ( test_address_details )
	{
		g_hash_table_unref ( test_address_details );
	}
}

int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );

	test_address_details_setup();

	/*add test cases*/
	g_test_add_func (
	    "/geoclue/geoclue_address_details/new/\
	    test_address_details_new",
	    test_address_details_new );
	g_test_add_func (
	    "/geoclue/geoclue_address_details/copy/\
	    test_address_details_new",
	    test_address_details_new );
	g_test_add_func (
	    "/geoclue/geoclue_address_details/copy/\
	    test_address_details_copy",
	    test_address_details_copy );
	/*g_test_add_func("/geoclue/geoclue_address_details/
		test_address_details_insert",test_address_details_insert);
	g_test_add_func("/geoclue/geoclue_address_details/
		test_address_details_set_country_from_code",
		test_address_details_set_country_from_code);*/
	g_test_add_func (
	    "/geoclue/geoclue_address_details/\
	    get_accuracy_level/test_address_details_new",
	    test_address_details_new );
	g_test_add_func (
	    "/geoclue/geoclue_address_details/get_accuracy_level/\
	    test_address_details_get_accuracy_level",
	    test_address_details_get_accuracy_level );

	/*run test cases*/
	ret=g_test_run();

	test_address_details_tear_down();

	return ret;

}






