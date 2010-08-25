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
#include "test_geoclue_utils.h"

void static
test_accuracy_new_get_details()
{
	/*create a new accuracy*/
	GeoclueAccuracy* new_accuracy = geoclue_accuracy_new (
	                              TEST_LOCATION.accuracy.accuracy_level,	    
                             	  TEST_LOCATION.accuracy.horizontal_accuracy,\
	                              TEST_LOCATION.accuracy.vertical_accuracy );
	g_assert ( new_accuracy != NULL );
	
	/*get accuracy detail*/
	GeoclueAccuracyLevel level;
	double   horizontal_accuracy, vertical_accuracy;
	geoclue_accuracy_get_details ( new_accuracy, &level, &horizontal_accuracy,
	                               &vertical_accuracy );
	
	/*compare details of new created accuracy with expected values*/
	g_assert_cmpint ( level, ==, TEST_LOCATION.accuracy.accuracy_level );
	g_assert_cmpfloat ( horizontal_accuracy, ==,
	                    TEST_LOCATION.accuracy.horizontal_accuracy );
	g_assert_cmpfloat ( vertical_accuracy, ==,
	                    TEST_LOCATION.accuracy.vertical_accuracy );

	geoclue_accuracy_free ( new_accuracy );
}

static void
test_accuracy_set_get_details ()
{
	/*create a new accuracy*/
	GeoclueAccuracy* new_accuracy = geoclue_accuracy_new (
	                                    GEOCLUE_ACCURACY_LEVEL_NONE, 0,0 );
	g_assert ( new_accuracy != NULL );
	
	/*insert accuracy details*/
	geoclue_accuracy_set_details ( new_accuracy,
	                               TEST_LOCATION.accuracy.accuracy_level,
	                               TEST_LOCATION.accuracy.horizontal_accuracy,
	                               TEST_LOCATION.accuracy.vertical_accuracy );
	/*get accuracy detail*/
	GeoclueAccuracyLevel level;
	double   horizontal_accuracy, vertical_accuracy;
	geoclue_accuracy_get_details ( new_accuracy, &level, &horizontal_accuracy,
	                               &vertical_accuracy );
	
	/*compare details of new created accuracy with expected values*/
	g_assert_cmpint ( level, ==, TEST_LOCATION.accuracy.accuracy_level );
	g_assert_cmpfloat ( horizontal_accuracy, ==,
	                    TEST_LOCATION.accuracy.horizontal_accuracy );
	g_assert_cmpfloat ( vertical_accuracy, ==,
	                    TEST_LOCATION.accuracy.vertical_accuracy );

	geoclue_accuracy_free ( new_accuracy );
}

static void
test_accuracy_copy ()
{
	/*create a new accuracy*/
	GeoclueAccuracy* source_accuracy = geoclue_accuracy_new (
		TEST_LOCATION.accuracy.accuracy_level,	                                
     	TEST_LOCATION.accuracy.horizontal_accuracy,	                            
        TEST_LOCATION.accuracy.vertical_accuracy );
	g_assert ( source_accuracy != NULL );

	/*get a copied accuracy*/
	GeoclueAccuracy* copied_accuracy = geoclue_accuracy_copy ( source_accuracy
	                                                         );
	g_assert ( copied_accuracy != NULL );

	/*get accuracy detail*/
	GeoclueAccuracyLevel level;
	double   horizontal_accuracy, vertical_accuracy;
	geoclue_accuracy_get_details ( copied_accuracy, &level,
	                               &horizontal_accuracy, &vertical_accuracy );

	/*compare details of copied accuracy with expected values*/
	g_assert_cmpint ( level, ==, TEST_LOCATION.accuracy.accuracy_level );
	g_assert_cmpfloat ( horizontal_accuracy, ==,
	                    TEST_LOCATION.accuracy.horizontal_accuracy );
	g_assert_cmpfloat ( vertical_accuracy, ==,
	                    TEST_LOCATION.accuracy.vertical_accuracy );

	geoclue_accuracy_free ( source_accuracy );
	geoclue_accuracy_free ( copied_accuracy );
}


int main ( int argc,char **argv )
{
	int ret = 1;

	g_type_init();
	g_test_init ( &argc,&argv,NULL );
	g_assert ( argc==1 );

	test_setup();

	/*add test cases*/
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/new/test_accuracy_new_get_details",
	    test_accuracy_new_get_details );
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/get_details/test_accuracy_new_get_details",
	    test_accuracy_new_get_details );
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/get_details/test_accuracy_new_get_details",
	    test_accuracy_set_get_details );
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/set_details/test_accuracy_set_get_details",
	    test_accuracy_new_get_details );
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/set_details/test_accuracy_set_get_details",
	    test_accuracy_set_get_details );
	g_test_add_func (
	    "/geoclue/geoclue_accuracy/copy/test_accuracy_copy",test_accuracy_copy
	);
	/*run test cases*/
	ret=g_test_run();

	test_tear_down();

	return ret;

}
