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
#include <glib-object.h>
#include <geoclue/geoclue-geocode.h>
#include "test_geoclue_utils.h"

GcTestPosition*  test_position_async = NULL;
GcTestAccuracy* test_position_accuracy_async = NULL;

void test_geocode_address2position_async_latitude () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned altitude with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_LATITUDE);
}

void test_geocode_address2position_async_longitude () {

    /*Wait for obtaining potition*/
    wait();
     g_assert(test_position_async != NULL);
    
	 /*check returned longitude with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_LONGITUDE);
}
void test_geocode_address2position_async_altitude () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned altitude with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_ALTITUDE);
}

void test_geocode_address2position_async_fields_latitude () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned altitude field with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_FIELDS_LATITUDE);
}
void test_geocode_address2position_async_fields_longitude () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned longitude field with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_FIELDS_LONGITUDE);
}
void test_geocode_address2position_async_fields_altitude () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned altitude field with expected result*/
    test_position_check_details (test_position_async,
		TEST_POSITION_KEY_FIELDS_ALTITUDE);
}
void test_geocode_address2position_async_accuracy_level () {

    /*Wait for obtaining potition*/
    wait();
    g_assert(test_position_async != NULL);
    
	/*check returned accuracy_level with expected result*/
    g_assert (test_position_accuracy_async != NULL);
    test_accuracy_check_details (test_position_accuracy_async,
		TEST_ACCURACY_KEY_ACCURACY_LEVEL);
}

void test_geocode_address2position_async_accuracy_horizontal () {

    /*Wait for obtaining potition*/
    wait();
    
	/*check returned horizontal_accurary with expected result*/
    g_assert (test_position_accuracy_async != NULL);
    test_accuracy_check_details (test_position_accuracy_async,
		TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL);
}

void test_geocode_address2position_async_accuracy_vertical () {

    /*Wait for obtaining potition*/
    wait();
    
	/*check returned vertical_accuracy with expected result*/
    g_assert (test_position_accuracy_async != NULL);
    test_accuracy_check_details (test_position_accuracy_async,
		TEST_ACCURACY_KEY_ACCURACY_VERTICAL);
}

static void callback_test_geocode_address2position_async (GeoclueGeocode     
*pcode,
		   GeocluePositionFields fields,
//		   int                   timestamp,  //it's bug in latest geoclue, modified by jinliang.
		   double                latitude,
		   double                longitude,
		   double                altitude,
		   GeoclueAccuracy      *accuracy,
		   GError               *error,
		   gpointer              userdata)
{
   /*assert no error occurs*/
	if (error) {
		g_print ("Error getting position: %s\n", error->message);
        	g_test_message("Error getting position: %s\n", error->message);
		g_assert_not_reached();
	} 

    /*setup test position*/
    test_position_async = test_position_record (&latitude, &longitude,
		&altitude, TEST_LOCATION.position.fields); // bugs here, if using fields);  modified by jinliang.
    g_assert(test_position_async != NULL);
    test_position_accuracy_async = test_accuracy_record(accuracy);
    
     /*notify other test cases*/
    notify();

    /*quit from mainloop_test_position_async*/
   g_main_loop_quit ((GMainLoop *)userdata);
}


static void  test_geocode_address_to_position_async()
{
    /*create geoclue from geoclue test provider*/
	GeoclueGeocode *geocode= create_geoclue_geocode(PROVIDER_YAHOO);
	 g_assert(geocode != NULL);
 
	/*prepare main loop*/
     GMainLoop *mainloop_test_geocode_async = g_main_loop_new (NULL, FALSE);
     g_assert (mainloop_test_geocode_async != NULL);
     /*get position asynchronously*/

	GHashTable* address_details = test_location_to_address(&TEST_LOCATION);
	geoclue_geocode_address_to_position_async(geocode,address_details,
			(GeoclueGeocodeCallback)
			callback_test_geocode_address2position_async,\
			mainloop_test_geocode_async);
	g_main_loop_run(mainloop_test_geocode_async);	

    /*unref objects*/
	g_hash_table_destroy(address_details);
	g_main_loop_unref(mainloop_test_geocode_async);
	g_object_unref(geocode);

}


int main(int argc,char **argv)
{
	int ret = 1;
	
	g_type_init();
	g_test_init(&argc,&argv,NULL);
	g_assert(argc==1);

   
	g_test_add_func("/geoclue/geoclue_geocode/new/\
		test_geocode_address_to_position_async",
		test_geocode_address_to_position_async);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address_to_position_async",
		test_geocode_address_to_position_async);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_latitude",
		test_geocode_address2position_async_latitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_longitude",
		test_geocode_address2position_async_longitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_altitude",
		test_geocode_address2position_async_altitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_fields_latitude",
		test_geocode_address2position_async_fields_latitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_fields_longitude",
		test_geocode_address2position_async_fields_longitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_fields_altitude",
		test_geocode_address2position_async_fields_altitude);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_accuracy_level",
		test_geocode_address2position_async_accuracy_level);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_accuracy_horizontal",
		test_geocode_address2position_async_accuracy_horizontal);
	
	g_test_add_func("/geoclue/geoclue_geocode/address_to_position_async/\
		test_geocode_address2position_async_accuracy_vertical",
		test_geocode_address2position_async_accuracy_vertical);
		
	/*run test cases*/
		test_setup();
		ret=g_test_run();
		test_tear_down();
		return ret;	
	}
