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
#include <glib.h>
#include "test_geoclue_utils.h"
#include "geoclue-test-provider-client.h"

#define DELAY_USEC_UNIT (1000000UL)


static  DBusGConnection *connection = NULL;
static  DBusGProxy *proxy = NULL;
/*
The White House
    1600 Pennsylvania Avenue, NW
    Washington, DC 20500
*/

GcTestLocation location_white_house = {	{38.89859, -77.035971, 0,
	GEOCLUE_POSITION_FIELDS_LATITUDE | \
	GEOCLUE_POSITION_FIELDS_LONGITUDE | \
        GEOCLUE_POSITION_FIELDS_ALTITUDE},\
	{"US", "United States", "NW", "Washington", "Washington, DC", "20500",
		"1600 Pennsylvania Avenue"},
	{GEOCLUE_ACCURACY_LEVEL_STREET, 0,0}
};


TestProviderData test_providers_data[PROVIDER_NUM] =
{
	/*test provider*/
	{"org.freedesktop.Geoclue.Providers.Test",
	"/org/freedesktop/Geoclue/Providers/Test", "Test", "GeoClue Test\
		Provider",
	"org.freedesktop.Geoclue.Position;org.freedesktop.Geoclue.Address"},
	
 	/*manual provider*/
 	{"org.freedesktop.Geoclue.Providers.Manual",
	"/org/freedesktop/Geoclue/Providers/Manual","Manual", "Manual provider",
	"org.freedesktop.Geoclue.Address"},

 	/*example provider*/	
 	{"org.freedesktop.Geoclue.Providers.Example",
	"/org/freedesktop/Geoclue/Providers/Example","Example", "Example\
	provider",\
	"org.freedesktop.Geoclue.Position"},
	
	/*hostip provider*/
	{"org.freedesktop.Geoclue.Providers.Hostip",
	"/org/freedesktop/Geoclue/Providers/Hostip","Hostip", "Hostip\
	provider",\
	"org.freedesktop.Geoclue.Position;org.freedesktop.Geoclue.Address"} ,
	
 	/*yahoo provider*/
	{"org.freedesktop.Geoclue.Providers.Yahoo",
	"/org/freedesktop/Geoclue/Providers/Yahoo","Yahoo", "Geocode provider that\
	uses the Yahoo! Maps web services API","org.freedesktop.Geoclue.Geocode"},
	
 	/*geonames provider*/	
	{"org.freedesktop.Geoclue.Providers.Geonames",
	"/org/freedesktop/Geoclue/Providers/Geonames","Geonames", "Geonames\
	provider","org.freedesktop.Geoclue.Geocode;org.freedesktop.Geoclue.\
	ReverseGeocode"},
	
 	/*localnet provider*/
	{"org.freedesktop.Geoclue.Providers.Localnet",
	"/org/freedesktop/Geoclue/Providers/Localnet","Localnet", "provides Address\
	based on current gateway mac address and a local address file (which can be\
	updated through D-Bus)","org.freedesktop.Geoclue.Address"},
	
 	/*plazes provider*/	
	{"org.freedesktop.Geoclue.Providers.Plazes",
	"/org/freedesktop/Geoclue/Providers/Plazes","Plazes", "Plazes.com based\
	provider, uses gateway mac address to\
	locate","org.freedesktop.Geoclue.Position;org.freedesktop.Geoclue.Address"}
};



void
geoclue_test_provider_setup ()
{
	GError* error = NULL;
	connection = dbus_g_bus_get ( DBUS_BUS_SESSION,&error );
	g_assert ( connection != NULL );

	proxy = dbus_g_proxy_new_for_name ( connection, GEOCLUE_TEST_SERVICE,
GEOCLUE_TEST_PATH, GEOCLUE_TEST_INTERFACE );
	g_assert ( proxy != NULL );
}

void
geoclue_test_provider_tear_down ()
{
	g_object_unref ( proxy );
}



GcTestPosition*
test_position_record ( double* latitude,  double* longitude, double* altitude,
	GeocluePositionFields fields )
{
	GValueArray* test_location = g_value_array_new ( 6 );
	g_assert ( test_location != NULL );

	GValue v_lat = {0};
	g_value_init ( &v_lat, G_TYPE_DOUBLE );
	g_value_set_double ( &v_lat, *latitude );

	GValue v_log = {0};
	g_value_init ( &v_log, G_TYPE_DOUBLE );
	g_value_set_double ( &v_log, *longitude );

	GValue v_alt = {0};
	g_value_init ( &v_alt, G_TYPE_DOUBLE );
	g_value_set_double ( &v_alt, *altitude );

	GValue v_fields_lat = {0};
	g_value_init ( &v_fields_lat, G_TYPE_DOUBLE );
	g_value_set_double ( &v_fields_lat, fields &
		GEOCLUE_POSITION_FIELDS_LATITUDE );

	GValue v_fields_log = {0};
	g_value_init ( &v_fields_log, G_TYPE_DOUBLE );
	g_value_set_double ( &v_fields_log, fields &
		GEOCLUE_POSITION_FIELDS_LONGITUDE );

	GValue v_fields_alt = {0};
	g_value_init ( &v_fields_alt, G_TYPE_DOUBLE );
	g_value_set_double ( &v_fields_alt, fields &
		GEOCLUE_POSITION_FIELDS_ALTITUDE );

	g_value_array_insert ( test_location, TEST_POSITION_KEY_LATITUDE, &v_lat );
	g_value_array_insert ( test_location, TEST_POSITION_KEY_LONGITUDE, &v_log );
	g_value_array_insert ( test_location, TEST_POSITION_KEY_ALTITUDE, &v_alt );
	g_value_array_insert ( test_location, TEST_POSITION_KEY_FIELDS_LATITUDE,
		&v_fields_lat );
	g_value_array_insert ( test_location, TEST_POSITION_KEY_FIELDS_LONGITUDE,
		&v_fields_log );
	g_value_array_insert ( test_location, TEST_POSITION_KEY_FIELDS_ALTITUDE,
		&v_fields_alt );

	return test_location;
}

void
test_position_free ( GcTestPosition* test_position )
{
	g_value_array_free ( test_position );
}

double
test_position_get_property ( GcTestPosition* test_position, TestPositionProperty
	property )
{
	return g_value_get_double ( g_value_array_get_nth ( test_position, property
) );
}

GcTestAddress*
test_address_record ( GHashTable* address_details )
{
	g_assert ( address_details != NULL );
	return ( GHashTable* ) geoclue_address_details_copy ( address_details );
}

void
test_address_free ( GcTestAddress* test_address )
{
	g_hash_table_unref ( test_address );
}

gchar*
test_address_get_property ( GcTestAddress* test_address, TestAddressProperty
	property )
{
	return g_strdup ( g_hash_table_lookup ( test_address, property ) );
}

GcTestAccuracy*
test_accuracy_record ( GeoclueAccuracy* accuracy )
{
	GValueArray* test_accuracy = NULL;
	if ( accuracy ) {
		test_accuracy = g_value_array_new ( 3 );
		g_assert ( test_accuracy != NULL );

		GeoclueAccuracyLevel level;
		double   horizontal_accuracy, vertical_accuracy;
		geoclue_accuracy_get_details ( accuracy, &level, &horizontal_accuracy,
			&vertical_accuracy );

		GValue v_level = {0};
		g_value_init ( &v_level, G_TYPE_DOUBLE );
		g_value_set_double ( &v_level, level );

		GValue v_horizontal = {0};
		g_value_init ( &v_horizontal, G_TYPE_DOUBLE );
		g_value_set_double ( &v_horizontal, horizontal_accuracy );

		GValue v_vertical = {0};
		g_value_init ( &v_vertical, G_TYPE_DOUBLE );
		g_value_set_double ( &v_vertical, vertical_accuracy );

		g_value_array_insert ( test_accuracy, TEST_ACCURACY_KEY_ACCURACY_LEVEL,
			&v_level );
		g_value_array_insert ( test_accuracy,
			TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL, &v_horizontal );
		g_value_array_insert ( test_accuracy,
			TEST_ACCURACY_KEY_ACCURACY_VERTICAL, &v_vertical );
	}
	return test_accuracy;
}

void
test_accuracy_free ( GcTestAccuracy* test_accuracy )
{
	g_hash_table_unref ( ( GHashTable* ) test_accuracy );
}

double
test_accuracy_get_property ( GcTestAccuracy* test_accuracy, TestAccuracyProperty
property )
{
	return g_value_get_double ( g_value_array_get_nth ( test_accuracy, property
		) );
}

void
insert_address_key_and_value ( char *key, char *value, GHashTable *target )
{
	g_hash_table_insert ( target,key,value );
}

gboolean
geoclue_test_provider_set_position ( GcTestPosition* position, GcTestAccuracy*
accuracy )
{
	g_assert ( position != NULL );
	GError* error = NULL;
	return org_freedesktop_Geoclue_Test_set_position ( proxy,
	        test_position_get_property ( position, TEST_POSITION_KEY_LATITUDE ),
	        test_position_get_property ( position, TEST_POSITION_KEY_LONGITUDE
				),
	        test_position_get_property ( position, TEST_POSITION_KEY_ALTITUDE ),
	        test_accuracy_get_property ( accuracy,
				TEST_ACCURACY_KEY_ACCURACY_LEVEL ),
	        test_accuracy_get_property ( accuracy,
				TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL ),
	        test_accuracy_get_property ( accuracy,
				TEST_ACCURACY_KEY_ACCURACY_VERTICAL ),
	        &error );
}


gboolean
geoclue_test_provider_set_address ( GHashTable* address )
{
	g_assert ( address != NULL );
	GError* error = NULL;
	return org_freedesktop_Geoclue_Test_set_address ( proxy,
	        0,
	        address,
	        &error );
}

static void
check_address_key_and_value ( char *key, char *value, GHashTable *target )
{
	char* target_value = g_hash_table_lookup ( target, key );
	g_assert ( target_value != NULL );
	g_assert_cmpstr ( value, ==, target_value );
}

void
compare_address_details ( GHashTable* source, GHashTable* target )
{
	g_hash_table_foreach ( source,
	                       ( GHFunc ) check_address_key_and_value,
	                       target );
}

void
test_accuracy_check_details ( GcTestAccuracy* test_accuracy,
	TestAccuracyProperty property )
{

	GcTestAccuracy* expected_accuracy = test_location_to_accuray (
		&TEST_LOCATION );
	test_accuracy_check_details_full ( test_accuracy,property,expected_accuracy
);
}

void
test_accuracy_check_details_full ( GcTestAccuracy* real, TestAccuracyProperty
	property, GcTestAccuracy* expected )
{
	double real_value = test_accuracy_get_property ( real, property );
	double expected_value = test_accuracy_get_property ( expected, property );
	g_assert_cmpfloat ( real_value, ==, expected_value );
}

void
test_position_check_details ( GcTestPosition* test_position,
	TestPositionProperty property )
{
	GcTestPosition* expected_position = test_location_to_position (
		&TEST_LOCATION );
	test_position_check_details_full ( test_position, property,
		expected_position );
}

void
test_position_check_details_full ( GcTestPosition* real, TestPositionProperty
	property, GcTestPosition* expected )
{
	g_assert ( real != NULL && expected != NULL );
	double real_value = test_position_get_property ( real, property );
	double expected_value = test_position_get_property ( expected, property );
	g_assert_cmpfloat ( real_value, ==, expected_value );
}

void
test_address_check_details ( GcTestAddress* test_address, TestAddressProperty
	property )
{
	GcTestAddress* expected_address = test_location_to_address ( &TEST_LOCATION
		);
	test_address_check_details_full ( test_address, property, expected_address
		);
}

void
test_address_check_details_full ( GcTestAddress* real, TestAddressProperty
property, GcTestAddress* expected )
{
	g_assert ( real != NULL && expected != NULL );
	char*  real_value = test_address_get_property ( real, property );
	char*  expected_value = test_address_get_property ( expected, property );
	g_assert_cmpstr ( real_value, ==, expected_value );
}

void wait()
{
	g_usleep ( DELAY_USEC_UNIT * 2 );
}
void notify()
{
}

void test_setup ()
{
	geoclue_test_provider_setup();
	geoclue_test_provider_set_position ( test_location_to_position (
		&TEST_LOCATION ),test_location_to_accuray (
		&TEST_LOCATION ));
	geoclue_test_provider_set_address ( test_location_to_address (
		&TEST_LOCATION ) );
}

void test_tear_down ()
{
	geoclue_test_provider_tear_down ();
}

GcTestPosition* test_location_to_position ( GcTestLocation* location )
{
	g_assert ( location != NULL );
	GcTestPosition* position = test_position_record (
		&location->position.latitude,
	                           &location->position.longitude,
	                           &location->position.altitude,
	                           location->position.fields );
	return position;
}

GcTestAddress* test_location_to_address ( GcTestLocation* location )
{
	g_assert ( location != NULL );
	GHashTable* address_details = g_hash_table_new ( g_str_hash, g_str_equal );

	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_COUNTRYCODE,
		location->address.country_code );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_COUNTRY,
		location->address.country );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_REGION,
		location->address.region );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_LOCALITY,
		location->address.locality );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_AREA,
		location->address.area );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_POSTALCODE,
		location->address.postalcode );
	g_hash_table_insert ( address_details, GEOCLUE_ADDRESS_KEY_STREET,
		location->address.street );
	return address_details;
}

GcTestAccuracy* test_location_to_accuray ( GcTestLocation* location )
{
	g_assert ( location != NULL );

	GcTestAccuracy* accuracy = test_accuracy_record ( geoclue_accuracy_new (
		location->accuracy.accuracy_level,
	                           location->accuracy.horizontal_accuracy,
	                           location->accuracy.vertical_accuracy )
	                                                );
	return accuracy;
}



GeocluePosition*
create_geoclue_position ( TestProviderKey key )
{
	return GEOCLUE_POSITION ( create_geoclue_provider ( key,
		PROVIDER_TYPE_POSITION ) );
}

GeoclueAddress*
create_geoclue_address ( TestProviderKey key )
{
	return GEOCLUE_ADDRESS ( create_geoclue_provider ( key,
		PROVIDER_TYPE_ADDRESS ) );
}
GeoclueGeocode*
create_geoclue_geocode ( TestProviderKey key )
{
	return GEOCLUE_GEOCODE ( create_geoclue_provider ( key,
		PROVIDER_TYPE_GEOCODE ) );
}

GeoclueMasterClient* create_geoclue_master_client()
{
	GeoclueMaster* master = geoclue_master_get_default ();
	g_assert ( master != NULL );

	GError * error = NULL;
	GeoclueMasterClient* client = geoclue_master_create_client ( master, NULL,
		&error );
	g_assert ( client != NULL );
	
	/* Set provider requirements*/
	geoclue_master_client_set_requirements ( client,
	        GEOCLUE_ACCURACY_LEVEL_STREET,
	        0, FALSE,
	        GEOCLUE_RESOURCE_ALL,
	        &error );
	
	/*assert no error occurs*/
	if ( error ){
		g_test_message ( "Error in geoclue_master_client_set_requirements:\
%s.\n", error->message );
		g_assert_not_reached();
	}
	g_object_unref ( master );
	return client;

}

GeoclueProvider* create_geoclue_provider ( TestProviderKey key, TestProviderType
	type )
{
	GeoclueProvider* provider = NULL;
	char* service = test_providers_data[key].service;
	char* path = test_providers_data[key].path;
	if ( service != NULL &&  path != NULL ){
		switch ( type )
		{
			case PROVIDER_TYPE_POSITION:
				provider = GEOCLUE_PROVIDER ( geoclue_position_new ( service,
								path ) );
				break;
			case PROVIDER_TYPE_ADDRESS:
				provider = GEOCLUE_PROVIDER ( geoclue_address_new ( service,
								path ) );
				break;
			case PROVIDER_TYPE_GEOCODE:
				provider = GEOCLUE_PROVIDER ( geoclue_geocode_new ( service,
								path ) );
				break;
			default:
				break;
		}

	}
	return provider;
}






