/*
* geoclue-test.c - Test address provider
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


/** Geoclue test provider**/

#include <string.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus.h>

#include <geoclue/gc-provider.h>
#include <geoclue/gc-iface-address.h>
#include <geoclue/gc-iface-position.h>

typedef struct _GeoclueTestPosition
{
	GeocluePositionFields fields;
	gdouble latitude;
	gdouble longitude;
	gdouble altitude;
	GeoclueAccuracy* position_accuracy;
} GeoclueTestPosition;


typedef struct
{
	GcProvider parent;
	GMainLoop *loop;
	guint event_id;
	int timestamp;
	GHashTable *address;
	GeoclueTestPosition* position;
	GeoclueAccuracy *accuracy;
} GeoclueTest;

typedef struct
{
	GcProviderClass parent_class;
} GeoclueTestClass;


#define GEOCLUE_TYPE_TEST (geoclue_test_get_type ())
#define GEOCLUE_TEST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj),GEOCLUE_TYPE_TEST,\
	GeoclueTest))
#define GEOCLUE_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),\
	GEOCLUE_TYPE_TEST, GeoclueTest))
#define GEOCLUE_IS_TEST(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj),\
	GEOCLUE_TYPE_TEST))
#define GEOCLUE_IS_TEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),\
	GEOCLUE_TYPE_TEST))

static void geoclue_test_address_init ( GcIfaceAddressClass *iface );
static void geoclue_test_position_init ( GcIfacePositionClass  *iface );

G_DEFINE_TYPE_WITH_CODE ( GeoclueTest, geoclue_test, GC_TYPE_PROVIDER,
                          G_IMPLEMENT_INTERFACE ( GC_TYPE_IFACE_ADDRESS,
                                                  geoclue_test_address_init )
                          G_IMPLEMENT_INTERFACE ( GC_TYPE_IFACE_POSITION,
                                                  geoclue_test_position_init ) )

static gboolean
geoclue_test_set_address ( GeoclueTest *test,
                           int valid_until,
                           GHashTable *address,
                           GError **error );

static gboolean
geoclue_test_set_address_fields ( GeoclueTest *test,
                                  int valid_until,
                                  char *country_code,
                                  char *country,
                                  char *region,
                                  char *locality,
                                  char *area,
                                  char *postalcode,
                                  char *street,
                                  GError **error );


static gboolean
geoclue_test_set_position ( GeoclueTest *test,
                            double latitude,
                            double longitude,
                            double altitude,
                            int accuracy_level,
                            int horizontal_accuracy,
                            int vertical_accuracy,
                            GError **error );

#include "geoclue-test-glue.h"


static GeoclueAccuracyLevel
get_accuracy_for_address ( GHashTable *address )
{
	if ( g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_STREET ) ) {
		return GEOCLUE_ACCURACY_LEVEL_STREET;
	}
	else if ( g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_POSTALCODE ) ){
		return GEOCLUE_ACCURACY_LEVEL_POSTALCODE;
	}
	else if ( g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_LOCALITY ) ){
		return GEOCLUE_ACCURACY_LEVEL_LOCALITY;
	}
	else if ( g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_REGION ) ){
		return GEOCLUE_ACCURACY_LEVEL_REGION;
	}
	else if ( g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_COUNTRY ) ||
	          g_hash_table_lookup ( address, GEOCLUE_ADDRESS_KEY_COUNTRYCODE )
		){
		return GEOCLUE_ACCURACY_LEVEL_COUNTRY;
	}
	return GEOCLUE_ACCURACY_LEVEL_NONE;
}


static gboolean
get_status ( GcIfaceGeoclue *gc,
             GeoclueStatus  *status,
             GError        **error )
{
	*status = GEOCLUE_STATUS_AVAILABLE;
	return TRUE;
}

static void
shutdown ( GcProvider *provider )
{
	GeoclueTest *test;
	test = GEOCLUE_TEST ( provider );
	g_main_loop_quit ( test->loop );
}

gboolean
validity_ended ( GeoclueTest *test )
{
	test->event_id = 0;
	g_hash_table_remove_all ( test->address );
	geoclue_accuracy_set_details ( test->accuracy,
	                               GEOCLUE_ACCURACY_LEVEL_NONE, 0, 0 );

	gc_iface_address_emit_address_changed ( GC_IFACE_ADDRESS ( test ),
	                                        test->timestamp,
	                                        test->address,
	                                        test->accuracy );
	return FALSE;
}

/* Position interface implementation */

static gboolean
geoclue_test_get_position ( GcIfacePosition        *iface,
                            GeocluePositionFields  *fields,
                            int                    *timestamp,
                            double                 *latitude,
                            double                 *longitude,
                            double                 *altitude,
                            GeoclueAccuracy       **accuracy,
                            GError                **error )
{
	GeoclueTest *test = GEOCLUE_TEST ( iface );
	if ( timestamp ) {
		*timestamp = test->timestamp;
	}
	if ( fields ){
		*fields = test->position->fields;
	}
	if ( latitude ){
		*latitude = test->position->latitude;
	}
	if ( longitude ){
		*longitude = test->position->longitude;
	}
	if ( altitude ){
		*altitude = test->position->altitude;
	}
	if ( accuracy ){
		*accuracy = geoclue_accuracy_copy ( test->position->position_accuracy );
	}
	return TRUE;

}


static void
geoclue_test_set_address_common ( GeoclueTest *test,
                                  int valid_for,
                                  GHashTable *address )
{
	if ( test->event_id > 0 ){
		g_source_remove ( test->event_id );
	}
	test->timestamp = time ( NULL );
	g_hash_table_destroy ( test->address );
	test->address = address;
	geoclue_accuracy_set_details ( test->accuracy,
	                               get_accuracy_for_address ( address ),
	                               0, 0 );
	gc_iface_address_emit_address_changed ( GC_IFACE_ADDRESS ( test ),
	                                        test->timestamp,
	                                        test->address,
	                                        test->accuracy );
	if ( valid_for > 0 ){
		test->event_id = g_timeout_add ( valid_for * 1000,
		                                 ( GSourceFunc ) validity_ended,
		                                 test );
	}
}

static gboolean
geoclue_test_set_address ( GeoclueTest *test,
                           int valid_for,
                           GHashTable *address,
                           GError **error )
{
	geoclue_test_set_address_common ( test,
	                                  valid_for,
	                                  geoclue_address_details_copy ( address )
		);
	return TRUE;
}

static gboolean
geoclue_test_set_address_fields ( GeoclueTest *test,
                                  int valid_for,
                                  char *country_code,
                                  char *country,
                                  char *region,
                                  char *locality,
                                  char *area,
                                  char *postalcode,
                                  char *street,
                                  GError **error )
{
	GHashTable *address;
	address = geoclue_address_details_new ();
	if ( country_code && ( strlen ( country_code ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_COUNTRYCODE ),
		                      g_strdup ( country_code ) );
	}
	if ( country && ( strlen ( country ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_COUNTRY ),
		                      g_strdup ( country ) );
	}
	if ( region && ( strlen ( region ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_REGION ),
		                      g_strdup ( region ) );
	}
	if ( locality && ( strlen ( locality ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_LOCALITY ),
		                      g_strdup ( locality ) );
	}
	if ( area && ( strlen ( area ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_AREA ),
		                      g_strdup ( area ) );
	}
	if ( postalcode && ( strlen ( postalcode ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_POSTALCODE ),
		                      g_strdup ( postalcode ) );
	}
	if ( street && ( strlen ( street ) > 0 ) ){
		g_hash_table_insert ( address,
		                      g_strdup ( GEOCLUE_ADDRESS_KEY_STREET ),
		                      g_strdup ( street ) );
	}
	geoclue_test_set_address_common ( test,
	                                  valid_for,
	                                  address );
	return TRUE;
}

static gboolean
geoclue_test_set_position ( GeoclueTest *test,
                            double latitude,
                            double longitude,
                            double altitude,
                            int accuracy_level,
                            int horizontal_accuracy,
                            int vertical_accuracy,
                            GError **error )
{
	test->timestamp = time ( NULL );
	test->position->fields = GEOCLUE_POSITION_FIELDS_NONE;

	if ( -90 <= latitude && 90 >= latitude ){
		test->position->latitude = latitude;
		test->position->fields |=  GEOCLUE_POSITION_FIELDS_LATITUDE;
	}


	if ( -180 <= longitude && 180 >= longitude ){
		test->position->longitude = longitude;
		test->position->fields |=  GEOCLUE_POSITION_FIELDS_LONGITUDE;
	}

	if ( 0 <= altitude ){
		test->position->altitude = altitude;
		test->position->fields |=  GEOCLUE_POSITION_FIELDS_ALTITUDE;
	}

	if ( accuracy_level >= 0 && horizontal_accuracy >=0 && vertical_accuracy >=0
		){
		test->position->position_accuracy =   geoclue_accuracy_new (
			accuracy_level, horizontal_accuracy, vertical_accuracy );
	}
	gc_iface_position_emit_position_changed ( GC_IFACE_POSITION ( test ),
	        test->position->fields,
	        time ( NULL ),
	        test->position->latitude,
	        test->position->longitude,
	        test->position->altitude,
	        test->position->position_accuracy );

	return TRUE;
}


static void
finalize ( GObject *object )
{
	GeoclueTest *test;
	test = GEOCLUE_TEST ( object );
	g_hash_table_destroy ( test->address );
	geoclue_accuracy_free ( test->accuracy );
	( ( GObjectClass * ) geoclue_test_parent_class )->finalize ( object );
}

static void
geoclue_test_class_init ( GeoclueTestClass *klass )
{
	GObjectClass *o_class = ( GObjectClass * ) klass;
	GcProviderClass *p_class = ( GcProviderClass * ) klass;
	o_class->finalize = finalize;
	p_class->get_status = get_status;
	p_class->shutdown = shutdown;
	dbus_g_object_type_install_info ( geoclue_test_get_type (),
	                                  &dbus_glib_geoclue_test_object_info );
}

static void
geoclue_test_init ( GeoclueTest *test )
{
	gc_provider_set_details ( GC_PROVIDER ( test ),
	                          "org.freedesktop.Geoclue.Providers.Test",
	                          "/org/freedesktop/Geoclue/Providers/Test",
	                          "Test", "GeoClue Test Provider" );
	test->address = geoclue_address_details_new ();
	test->accuracy =
	    geoclue_accuracy_new ( GEOCLUE_ACCURACY_LEVEL_NONE, 0, 0 );
	test->position = g_slice_new ( GeoclueTestPosition );
}

static gboolean
geoclue_test_get_address ( GcIfaceAddress   *gc,
                           int              *timestamp,
                           GHashTable      **address,
                           GeoclueAccuracy **accuracy,
                           GError          **error )
{
	GeoclueTest *test = GEOCLUE_TEST ( gc );

	if ( timestamp )	{
		*timestamp = test->timestamp;
	}
	if ( address )	{
		*address = geoclue_address_details_copy ( test->address );
	}
	if ( accuracy )	{
		*accuracy = geoclue_accuracy_copy ( test->accuracy );
	}
	return TRUE;
}

static void
geoclue_test_address_init ( GcIfaceAddressClass *iface )
{
	iface->get_address = geoclue_test_get_address;
}

static void
geoclue_test_position_init ( GcIfacePositionClass  *iface )
{
	iface->get_position = geoclue_test_get_position;
}

int
main ( int    argc,
       char **argv )
{
	GeoclueTest *test;
	g_type_init ();
	test = g_object_new ( GEOCLUE_TYPE_TEST, NULL );
	test->loop = g_main_loop_new ( NULL, TRUE );
	g_main_loop_run ( test->loop );
	g_main_loop_unref ( test->loop );
	g_object_unref ( test );

	return 0;
}
