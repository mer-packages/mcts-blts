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


static void foreach_glist_cb(gpointer data, gpointer user_data){
	GnomeKeyringNetworkPasswordData *item = data;
	printf("OK!, getting password: \n%s\n", item->user);
}

int main(int argc, char *argv[]) {
	g_type_init();

	/*gnome_keyring_find_network_password(NULL, NULL, "ws.audioscrobbler.com",
	 NULL, NULL, NULL, 0, found_password_cb, NULL, NULL);*/

	GList * list = NULL;
	GnomeKeyringResult result = gnome_keyring_find_network_password_sync(NULL,
			NULL, "ws.audioscrobbler.com", NULL, NULL, NULL, 0, &list);

	if (result == GNOME_KEYRING_RESULT_OK && NULL != list) {
		g_list_foreach(list, foreach_glist_cb, NULL);
	} else {
		if (result != GNOME_KEYRING_RESULT_NO_MATCH) {
			printf("Error! getting password: %s",
					gnome_keyring_result_to_message(result));
		} else {
			printf("Error! the password is NULL: %s",
					gnome_keyring_result_to_message(result));
		}
	}
}

#endif /* _READING_USER_ACCOUNT */
