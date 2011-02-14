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

#ifndef _SETTING_USER_ACCOUNT
#define _SETTING_USER_ACCOUNT

#include <glib.h>
#include <gnome-keyring.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <glib-object.h>

/*
 * Callback from the keyring lookup in refresh_credentials.
 */
static void keyring_op_done_cb(GnomeKeyringResult result, guint new_id,
		gpointer user_data) {
	if (result == GNOME_KEYRING_RESULT_OK) {
		printf("OK!, setting password, the new id is %d\n", new_id);

	} else {
		printf("Error! setting password: %s", gnome_keyring_result_to_message(
				result));
	}
	exit(0);
}

static void foreach_glist_cb(gpointer data, gpointer user_data) {
	GnomeKeyringNetworkPasswordData *item = data;
	//printf("OK!, getting password: %s; id: %d\n", item->user, item->item_id);
	gnome_keyring_item_delete_sync(NULL, item->item_id);
}

const char * USAGE =
		"Usage:\n  \
	-s  the service name,  for now, only the service\
 'lastfm', 'twitter', and 'vimeo' are supported\n\
    -u  user name on the service\n\
	-p  password for the user\n";

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
	char *userName = "moblintest2";
	char *password = "";
	char *service_name = NULL;
	int c = -1;

	opterr = 0;

	while ((c = getopt(argc, argv, "s:u:p:h")) != -1)
		switch (c) {
		case 's':
			service_name = optarg;
			break;
		case 'u':
			userName = optarg;
			break;
		case 'p':
			password = optarg;
			break;
		case 'h':
			fprintf(stderr, USAGE, optopt);
			return -1;
		case '?':
			if (optopt == 's' || optopt == 'u' || optopt == 'p')
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

	char * service_url = getTheServiceUrl(service_name);

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
		g_list_foreach(list, foreach_glist_cb, NULL);
	}

	gnome_keyring_set_network_password(NULL, userName, NULL,
			service_url, NULL, NULL, NULL, 0, password,
			keyring_op_done_cb, NULL, NULL);

	GMainLoop *loop;

	loop = g_main_loop_new(NULL, TRUE);
	g_main_loop_run(loop);
	g_main_loop_unref(loop);
        
        return 0;
}

#endif /* _SETTING_USER_ACCOUNT */
