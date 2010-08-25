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

#ifndef _TEST_GEOCLUE_DEF_H
#define _TEST_GEOCLUE_DEF_H

#include <glib.h>
#include <geoclue/geoclue-types.h>

G_BEGIN_DECLS

typedef enum _TestProviderKey
{
	PROVIDER_TEST = 0,
	PROVIDER_MANUAL,
	PROVIDER_EXAMPLE,
	PROVIDER_HOSTIP,
	PROVIDER_YAHOO,
	PROVIDER_GEONAMES,
	PROVIDER_LOCALNET,
	PROVIDER_PLAZES,
	PROVIDER_NUM
}TestProviderKey;

typedef enum _TestProviderType
{
	PROVIDER_TYPE_POSITION = 0,
	PROVIDER_TYPE_ADDRESS,
	PROVIDER_TYPE_GEOCODE,
	PROVIDER_TYPE_VELOCITY
}TestProviderType;

typedef enum _ProviderDataKey
{
	PROVIDER_DATA_NAME = 0,
	PROVIDER_DATA_DESCRIPTION,
	PROVIDER_DATA_SERVICE,
	PROVIDER_DATA_PATH
}ProviderDataKey;

typedef struct _TestProviderData
{
	char *service;
	char *path;
	char* name;
	char* description;
	char* interface;
}TestProviderData;

extern TestProviderData test_providers_data[PROVIDER_NUM];


typedef enum _TestPositionProperty
{
	/*position properties*/
	TEST_POSITION_KEY_LATITUDE = 0,
	TEST_POSITION_KEY_LONGITUDE,
	TEST_POSITION_KEY_ALTITUDE,
	TEST_POSITION_KEY_FIELDS_LATITUDE,
	TEST_POSITION_KEY_FIELDS_LONGITUDE,
	TEST_POSITION_KEY_FIELDS_ALTITUDE
}TestPositionProperty;

typedef enum _TestAccuracyProperty
{
	/*accuracy properties*/
	TEST_ACCURACY_KEY_ACCURACY_LEVEL,
	TEST_ACCURACY_KEY_ACCURACY_HORIZONTAL,
	TEST_ACCURACY_KEY_ACCURACY_VERTICAL
}TestAccuracyProperty;


typedef gchar* TestAddressProperty;

/*address properties*/
/*
typedef enum _TestAddressProperty {
    TEST_ADDRESS_KEY_COUNTRY,
    TEST_ADDRESS_KEY_COUNTRY_CODE,
    TEST_ADDRESS_KEY_REGION,
    TEST_ADDRESS_KEY_LOCALITY,
    TEST_ADDRESS_KEY_AREA,
    TEST_ADDRESS_KEY_POSTAL_CODE,
    TEST_ADDRESS_KEY_STREET,
    TEST_LOCATION_PROPERTY_NUM
}TestAddressProperty;
*/



typedef struct _PositionData
{
	//GeocluePositionFields fields;
	gdouble latitude;
	gdouble longitude;
	gdouble altitude;
	gint fields;
} PositionData;

typedef struct _AddressData
{
	char *country_code;
	char *country;
	char *region;
	char *locality;
	char *area;
	char *postalcode;
	char *street;
}AddressData;

typedef struct _AccuracyData
{
	gint accuracy_level;
	gdouble horizontal_accuracy;
	gdouble vertical_accuracy;
}AccuracyData;



typedef GValueArray GcTestPosition;
typedef GHashTable GcTestAddress;
typedef GeoclueAccuracy GcTestAccuracy;


typedef struct _GcTestLocation
{
	PositionData position;
	AddressData address;
	AccuracyData accuracy;
}GcTestLocation;


/*
The White House
    1600 Pennsylvania Avenue, NW
    Washington, DC 20500
*/
GcTestLocation location_white_house ;
#define TEST_LOCATION location_white_house

G_END_DECLS

#endif


