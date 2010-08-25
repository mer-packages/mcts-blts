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


static TestProviderData address_provider_info;
static TestProviderData position_provider_info;


static void
check_provider_info_property ( TestProviderData *provider_info, TestProviderKey
key, ProviderDataKey property )
{
	g_assert ( provider_info != NULL );
	TestProviderData expected_info = test_providers_data[key];
	switch ( property )
	{
		case PROVIDER_DATA_NAME:
			g_assert_cmpstr ( provider_info->name, ==, expected_info.name );
			break;
		case PROVIDER_DATA_DESCRIPTION:
			g_assert_cmpstr ( provider_info->description, ==,
								expected_info.description );
			break;
		case PROVIDER_DATA_SERVICE:
			g_assert_cmpstr ( provider_info->service, ==, expected_info.service
);
			break;
		case PROVIDER_DATA_PATH:
			g_assert_cmpstr ( provider_info->path, ==, expected_info.path );
			break;
		default:
			break;
	}
}

static void
test_geoclue_master_client_get_address_provider_info_name()
{
	/*Wait for obtaining address provider info*/
	wait();
	check_provider_info_property ( &address_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_NAME );
}

static void
test_geoclue_master_client_get_address_provider_info_description()
{
	/*Wait for obtaining address provider info*/
	wait();
	check_provider_info_property ( &address_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_DESCRIPTION );
}
static void
test_geoclue_master_client_get_address_provider_info_service()
{
	/*Wait for obtaining address provider info*/
	wait();
	check_provider_info_property ( &address_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_SERVICE );
}
static void
test_geoclue_master_client_get_address_provider_info_path()
{
	/*Wait for obtaining address provider info*/
	wait();
	check_provider_info_property ( &address_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_PATH );
}

static void
test_geoclue_master_client_get_position_provider_info_name()
{
	/*Wait for obtaining position provider info*/
	wait();
	check_provider_info_property ( &position_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_NAME );
}

static void
test_geoclue_master_client_get_position_provider_info_description()
{
	/*Wait for obtaining position provider info*/
	wait();
	check_provider_info_property ( &position_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_DESCRIPTION );
}
static void
test_geoclue_master_client_get_position_provider_info_service()
{
	/*Wait for obtaining position provider info*/
	wait();
	check_provider_info_property ( &position_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_SERVICE );
}
static void
test_geoclue_master_client_get_position_provider_info_path()
{
	/*Wait for obtaining position provider info*/
	wait();
	check_provider_info_property ( &position_provider_info, PROVIDER_TEST,
		PROVIDER_DATA_PATH );
}




static void
test_geoclue_master_client_get_address_provider_info()
{
	GeoclueMasterClient* client = create_geoclue_master_client();
	g_assert ( client != NULL );

	gchar *name;
	gchar *description;
	gchar *service;
	gchar *path;
	GError* error = NULL;
	geoclue_master_client_get_address_provider ( client, &name, &description,
&service, &path, &error );
	
	/*assert no error occurs*/
	if ( error ) {
		g_test_message ( "Error in geocode_master_client: %s.\n", error->message
			);
		g_assert_not_reached();
	}
	/*record test provider info*/
	address_provider_info.service = g_strdup ( service );
	address_provider_info.path = g_strdup ( path );
	address_provider_info.name = g_strdup ( name );
	address_provider_info.description = g_strdup ( description );
	
	/*notify other test cases*/
	notify();
	
	/*unref object*/
	g_object_unref ( client );
}

static void
test_geoclue_master_client_get_position_provider_info()
{
	GeoclueMasterClient* client = create_geoclue_master_client();
	g_assert ( client != NULL );

	gchar *name, *description, *service, *path;
	GError* error = NULL;
	geoclue_master_client_get_position_provider ( client, &name, &description,
&service, &path, &error );
	
	/*assert no error occurs*/
	if ( error ) {
		g_test_message ( "Error in geoclue_master_client_get_position_provider:\
			%s.\n", error->message );
		g_assert_not_reached();
	}
	g_print ( "provider info : %s, %s, %s, %s\n", service, description, name,
		path );

	/*record test provider info*/
	position_provider_info.service = g_strdup ( service );
	position_provider_info.path = g_strdup ( path );
	position_provider_info.name = g_strdup ( name );
	position_provider_info.description = g_strdup ( description );
	
	/*notify other test cases*/
	notify();
	
	/*unref object*/
	g_object_unref ( client );
}


int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );
	test_setup();

	g_test_add_func ("/geoclue/geoclue_master_client/get_address_provider_info/\
		test_geoclue_master_client_get_address_provider_info",
		test_geoclue_master_client_get_address_provider_info );
	g_test_add_func ("/geoclue/geoclue_master_client/get_address_provider_info/\
		test_geoclue_master_client_get_address_provider_info_name",
		test_geoclue_master_client_get_address_provider_info_name );
	g_test_add_func ("/geoclue/geoclue_master_client/get_address_provider_info/\
		test_geoclue_master_client_get_address_provider_info_description",
		test_geoclue_master_client_get_address_provider_info_description );
	g_test_add_func ("/geoclue/geoclue_master_client/get_address_provider_info/\
		test_geoclue_master_client_get_address_provider_info_service",
		test_geoclue_master_client_get_address_provider_info_service );
	g_test_add_func ("/geoclue/geoclue_master_client/get_address_provider_info/\
		test_geoclue_master_client_get_address_provider_info_path",
		test_geoclue_master_client_get_address_provider_info_path );
	g_test_add_func (
		"/geoclue/geoclue_master_client/get_position_provider_info/\
		test_geoclue_master_client_get_position_provider_info",
		test_geoclue_master_client_get_position_provider_info );
	g_test_add_func (
		"/geoclue/geoclue_master_client/get_position_provider_info/\
		test_geoclue_master_client_get_position_provider_info_name",
		test_geoclue_master_client_get_position_provider_info_name );
	g_test_add_func (
		"/geoclue/geoclue_master_client/get_position_provider_info/\
		test_geoclue_master_client_get_position_provider_info_description",
		test_geoclue_master_client_get_position_provider_info_description );
	g_test_add_func (
		"/geoclue/geoclue_master_client/get_position_provider_info/\
		test_geoclue_master_client_get_position_provider_info_service",
		test_geoclue_master_client_get_position_provider_info_service );
	g_test_add_func (
		"/geoclue/geoclue_master_client/get_position_provider_info/\
		test_geoclue_master_client_get_position_provider_info_path",
		test_geoclue_master_client_get_position_provider_info_path );

	/*run test cases*/
	ret=g_test_run();
	test_tear_down();
	return ret;
}
