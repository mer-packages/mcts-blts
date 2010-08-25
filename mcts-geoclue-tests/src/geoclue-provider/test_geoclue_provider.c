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

/*get status from test provider*/
static void
check_geoclue_provider_status ( GeoclueProvider* provider,GeoclueStatus
expected_status )
{
	g_assert ( provider != NULL );

	GeoclueStatus status;
	GError* error = NULL;
	gboolean ret = geoclue_provider_get_status ( provider, &status, &error );
	if ( error ) {
		g_test_message ( "Error in getting provider status!: %s.\n",
			error->message );
		g_error_free ( error );
		g_assert_not_reached();
	}
	g_assert_cmpint ( status, ==, expected_status );
	g_assert ( ret == TRUE );
}

static void
check_geoclue_provider_info ( GeoclueProvider* provider, TestProviderKey key )
{
	g_assert ( provider != NULL );
	gchar* name;
	gchar* description;
	GError* error = NULL;
	gboolean ret = geoclue_provider_get_provider_info ( provider, &name,
&description, &error );
	if ( error ) {
		g_test_message ( "Error in getting provider info!: %s.\n",
			error->message );
		g_error_free ( error );
		g_assert_not_reached();
	}
	TestProviderData expected_info = test_providers_data[key];
	g_assert_cmpstr ( name, ==, expected_info.name );
	g_assert_cmpstr ( description, ==, expected_info.description );
	g_assert ( ret == TRUE );
}

static void
check_geoclue_provider_set_option ( GeoclueProvider* provider, GHashTable*
options )
{
	g_assert ( provider != NULL );
	GError* error = NULL;

	gboolean ret = geoclue_provider_set_options ( provider, options, &error );
	if ( error ) {
		g_test_message ( "Error in getting provider info!: %s.\n",
			error->message );
		g_error_free ( error );
		g_assert_not_reached();
	}
	g_assert ( ret == TRUE );
}

static void
test_geoclue_provider_get_status_position_example ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_EXAMPLE,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_position_test ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_TEST,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_position_plazes ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_PLAZES,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_position_hostip ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_HOSTIP,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_address_manual ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_MANUAL,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_address_test ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_TEST,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_address_localnet ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_LOCALNET,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}


static void
test_geoclue_provider_get_status_geocode_geonames ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_GEONAMES,
		PROVIDER_TYPE_GEOCODE );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_status_geocode_yahoo ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_YAHOO,
		PROVIDER_TYPE_GEOCODE );
	check_geoclue_provider_status ( provider, GEOCLUE_STATUS_AVAILABLE );
	g_object_unref ( provider );
}


static void
test_geoclue_provider_get_provider_info_position_example ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_EXAMPLE,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_info ( provider, PROVIDER_EXAMPLE );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_position_test ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_TEST,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_info ( provider, PROVIDER_TEST );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_position_plazes ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_PLAZES,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_info ( provider, PROVIDER_PLAZES );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_position_hostip ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_HOSTIP,
		PROVIDER_TYPE_POSITION );
	check_geoclue_provider_info ( provider, PROVIDER_HOSTIP );
	g_object_unref ( provider );
}


static void
test_geoclue_provider_get_provider_info_address_manual ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_MANUAL,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_info ( provider, PROVIDER_MANUAL );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_address_test ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_TEST,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_info ( provider, PROVIDER_TEST );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_address_localnet ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_LOCALNET,
		PROVIDER_TYPE_ADDRESS );
	check_geoclue_provider_info ( provider, PROVIDER_LOCALNET );
	g_object_unref ( provider );
}


static void
test_geoclue_provider_get_provider_info_geocode_geonames ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_GEONAMES,
		PROVIDER_TYPE_GEOCODE );
	check_geoclue_provider_info ( provider, PROVIDER_GEONAMES );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_get_provider_info_geocode_yahoo ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_YAHOO,
		PROVIDER_TYPE_GEOCODE );
	check_geoclue_provider_info ( provider, PROVIDER_YAHOO );
	g_object_unref ( provider );
}

static void
test_geoclue_provider_set_options_test ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_TEST,
		PROVIDER_TYPE_ADDRESS );
	GHashTable* options = g_hash_table_new ( g_str_hash,g_str_equal );
	g_hash_table_insert ( options, "update", "on" );
	g_hash_table_insert ( options, "mode", "auto" );
	check_geoclue_provider_set_option ( provider, options );
	g_object_unref ( provider );
	g_hash_table_destroy ( options );
}

static void
test_geoclue_provider_set_options_example ()
{
	GeoclueProvider* provider = create_geoclue_provider ( PROVIDER_EXAMPLE,
		PROVIDER_TYPE_POSITION );
	GHashTable* options = g_hash_table_new ( g_str_hash,g_str_equal );
	g_hash_table_insert ( options, "update", "on" );
	g_hash_table_insert ( options, "mode", "auto" );
	check_geoclue_provider_set_option ( provider, options );
	g_object_unref ( provider );
	g_hash_table_destroy ( options );
}



int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );
	
	/*geoclue_position_get_status*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_position_example",
		test_geoclue_provider_get_status_position_example );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_position_test",
		test_geoclue_provider_get_status_position_test );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_position_plazes",
		test_geoclue_provider_get_status_position_plazes );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_position_hostip",
		test_geoclue_provider_get_status_position_hostip );
	
	/*geoclue_address_get_status*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_address_manual",
		test_geoclue_provider_get_status_address_manual );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_address_test",
		test_geoclue_provider_get_status_address_test );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_address_localnet",
		test_geoclue_provider_get_status_address_localnet );
	
	/*geoclue_geocode_get_status*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_geocode_geonames",
		test_geoclue_provider_get_status_geocode_geonames );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_status/\
		test_geoclue_provider_get_status_geocode_yahoo",
		test_geoclue_provider_get_status_geocode_yahoo );

	/*geoclue_position_get_provider_info*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_position_example",
		test_geoclue_provider_get_provider_info_position_example );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_position_test",
		test_geoclue_provider_get_provider_info_position_test );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_position_plazes",
		test_geoclue_provider_get_provider_info_position_plazes );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_position_hostip",
		test_geoclue_provider_get_provider_info_position_hostip );
	
	/*geoclue_address_get_provider_info*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_address_manual",
		test_geoclue_provider_get_provider_info_address_manual );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_address_test",
		test_geoclue_provider_get_provider_info_address_test );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_address_localnet",
		test_geoclue_provider_get_provider_info_address_localnet );
	
	/*geoclue_geocode_get_provider_info*/
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_geocode_geonames",
		test_geoclue_provider_get_provider_info_geocode_geonames );
	g_test_add_func (
		"/geoclue/geoclue_provider/get_provider_info/\
		test_geoclue_provider_get_provider_info_geocode_yahoo",
		test_geoclue_provider_get_provider_info_geocode_yahoo );

	g_test_add_func (
		"/geoclue/geoclue_provider/set_options/\
		test_geoclue_provider_set_options_example",
		test_geoclue_provider_set_options_example );
	g_test_add_func (
		"/geoclue/geoclue_provider/set_options/\
		test_geoclue_provider_set_options_test",
		test_geoclue_provider_set_options_test );


	/*run test cases*/
	ret=g_test_run();
	return ret;
}

