/*
 * Copyright (c) 2010, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Created on: 2010-9-29
 *      Author: Tang, Shao-Feng (shaofeng.tang@intel.com)
 */

#ifndef _READING_USER_ACCOUNT
#define _READING_USER_ACCOUNT

#include <glib.h>
#include <gnome-keyring.h>
#include <stdio.h>
#include <unistd.h>

const char
		* USAGE =
				"Usage:\n  \
    -s  the service name. for now, only the service\
 'lastfm', 'twitter', and 'vimeo' are supported\n\
	-p  reading password\n";

static void foreach_pwd_glist_cb(gpointer data, gpointer user_data) {
	GnomeKeyringNetworkPasswordData *item = data;
	printf("OK!, getting password: \n%s\n", item->password);
}

static void foreach_glist_cb(gpointer data, gpointer user_data) {
	GnomeKeyringNetworkPasswordData *item = data;
	printf("OK!, getting user name: \n%s\n", item->user);
}

static GHashTable * service_url_hash = NULL;

static void print_key_value(gpointer key, gpointer value, gpointer user_data) {
	printf("%s ---> %s\n", key, value);
}

static void display_hash_table(GHashTable *table) {
	g_hash_table_foreach(table, print_key_value, NULL);
}

static char * getTheServiceUrl(char *service_name) {
	if (NULL == service_url_hash) {
		service_url_hash = g_hash_table_new(g_str_hash, g_str_equal);

		g_hash_table_insert(service_url_hash, "lastfm", "ws.audioscrobbler.com");
		g_hash_table_insert(service_url_hash, "twitter", "api.twitter.com");
		g_hash_table_insert(service_url_hash, "vimeo", "vimeo.com");
	}
	display_hash_table(service_url_hash);
	return (char *) g_hash_table_lookup(service_url_hash, service_name);
}

int main(int argc, char *argv[]) {
	char *service_name = NULL;
	int getPassword = 0;
	int c = -1;

	opterr = 0;

	while ((c = getopt(argc, argv, "s:hp")) != -1)
		switch (c) {
		case 's':
			service_name = optarg;
			break;
		case 'p':
			getPassword = 1;
			break;
		case 'h':
			fprintf(stderr, USAGE, optopt);
			return -1;
		case '?':
			if (optopt == 's')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return -1;
		default:
			abort();
		}

	g_type_init();
	char * service_url = NULL;
	if (NULL != service_name) {
		service_url = getTheServiceUrl(service_name);
	}

	if (NULL == service_url) {
		fprintf(
				stderr,
				"Unknown service '%s', for now, only the service\
 'lastfm', 'twitter', and 'vimeo' are supported\n",
				service_name);
		g_hash_table_destroy(service_url_hash);
		return -1;
	}

	GList * list = NULL;
	GnomeKeyringResult result = gnome_keyring_find_network_password_sync(NULL,
			NULL, service_url, NULL, NULL, NULL, 0, &list);

	if (result == GNOME_KEYRING_RESULT_OK && NULL != list) {
		if (0 == getPassword) {
			g_list_foreach(list, foreach_glist_cb, NULL);
		} else {
			g_list_foreach(list, foreach_pwd_glist_cb, NULL);
		}
	} else {
		if (result != GNOME_KEYRING_RESULT_NO_MATCH) {
			printf("Error! getting user name: %s",
					gnome_keyring_result_to_message(result));
		} else {
			printf("Error! the user name is NULL: %s",
					gnome_keyring_result_to_message(result));
		}
	}

	g_hash_table_destroy(service_url_hash);
}

#endif /* _READING_USER_ACCOUNT */
