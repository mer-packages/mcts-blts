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


static void
test_geoclue_master_get_default()
{
	GeoclueMaster* master = geoclue_master_get_default();
	g_assert ( master != NULL );
	g_assert ( GEOCLUE_IS_MASTER ( master ) );
	g_object_unref ( master );
}


static void
test_geoclue_master_create_client ()
{
	GeoclueMaster *master = geoclue_master_get_default();
	g_assert ( master != NULL );
	gchar* path;
	GError* error = NULL;
	GeoclueMasterClient* client = geoclue_master_create_client ( master, &path,
		&error );
	
	/*assert no error occurs*/
	if ( error ) {
		g_test_message ( "Error in geocode_master_client: %s.\n", error->message
	);
		g_assert_not_reached();
	}
	g_assert ( client != NULL );
	g_assert ( GEOCLUE_IS_MASTER_CLIENT ( client ) );
	g_assert ( path != NULL );

	g_object_unref ( master );
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
		"/geoclue/geoclue_master/test_geoclue_master_get_default",
		test_geoclue_master_get_default );
	g_test_add_func (
		"/geoclue/geoclue_master/test_geoclue_master_create_client",
		test_geoclue_master_create_client );
	
	/*run test cases*/
	ret=g_test_run();
	test_tear_down();
	return ret;
}
