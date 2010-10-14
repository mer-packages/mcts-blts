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
 *  Created on: 2010-9-8
 *      Author: Tang, Shao-Feng (shaofeng.tang@intel.com)
 *
 *  Usage:
 *
 *  -s  the name of the testing service, available service list: flickr, lastfm, twitter, viemo ('flickr' is default)
 */

#ifndef _TESTCASE_LIBSOCIALWEB_KEYSTORE
#define _TESTCASE_LIBSOCIALWEB_KEYSTORE

#define KEY_FILE_LENGTH 30
#define BUFFER_LENGTH 1024

#include <glib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsocialweb-keystore/sw-keystore.h>

int read_key_secret_from_keys(const char *service, const char **key,
		const char **secret) {
	FILE *fp;
	int i = 0;
	char tmp_str[BUFFER_LENGTH];
	char FILE_NAME[KEY_FILE_LENGTH] = "/usr/share/libsocialweb/keys/";
	strcat(FILE_NAME, service);
	fprintf(stdout, "Reading keys of the service '%s' from %s\n", service,
			FILE_NAME);
	fp = fopen(FILE_NAME, "r");
	if (!fp) {
		fprintf(stderr, "Fail to open the corresponding keys '%s'\n", FILE_NAME);
		return;
	}

	for (i = 0; i < 2 && fscanf(fp, "%s", &tmp_str) > 0; i++) {
		//fprintf(stdout, "Line%d: %s\n", i, tmp_str);
		if (0 == i) {
			char *tmpkey;
			tmpkey = (char *) malloc(strlen(tmp_str) + 1);
			strcpy(tmpkey, tmp_str);
			*key = tmpkey;
		} else if (1 == i) {
			char *tmpsecret;
			tmpsecret = (char *) malloc(strlen(tmp_str) + 1);
			strcpy(tmpsecret, tmp_str);
			*secret = tmpsecret;
		}
	}
	fclose(fp);
}

void release_key_secret(const char **fkey, const char **fsecret) {
	if (NULL != *fkey) {
		free((void *) *fkey);
		fkey = NULL;
	}
	if (NULL != *fsecret) {
		free((void *) *fsecret);
		fsecret = NULL;
	}
}

/*
 * gboolean sw_keystore_get_key_secret (const char *service,
 *                                      const char **key,
 *                                      const char **secret);
 */
int test_sw_keystore_get_key_secret(const char *service) {
	const char *key = NULL, *secret = NULL;

	if (!sw_keystore_get_key_secret(service, &key, &secret)) {
		fprintf(stderr, "Fail to get the API_KEY and secret\n");
		return 1;
	} else {
		fprintf(stdout,
				"The API_KEY of the service '%s' from key-store API is %s\n",
				service, key);
		fprintf(stdout,
				"The secret of the service '%s' from key-store API is %s\n",
				service, secret);

		//read the key and secret from /usr/share/libsocialweb/keys/<service_name>
		const char *fkey = NULL, *fsecret = NULL;
		int result = 0;
		read_key_secret_from_keys(service, &fkey, &fsecret);

		fprintf(stdout, "The API_KEY of the service '%s' in key_file is %s\n",
				service, fkey);
		fprintf(stdout, "The secret of the service '%s' in key_file is %s\n",
				service, fsecret);

		if (0 != strcmp(key, fkey)) {
			fprintf(
					stderr,
					"ERROR! the API_KEYs are different.\nFrom LSW-Keystore, it is %s\nIn keys file, it is %s\n",
					key, fkey);
			result = 1;
		} else if (NULL != secret && NULL != fsecret && 0 != strcmp(secret,
				fsecret)) {
			fprintf(
					stderr,
					"ERROR! the secrets are different.\nFrom LSW-Keystore, it is %s\nIn keys file, it is %s\n",
					secret, fsecret);
			result = 1;
		} else if (NULL == secret && NULL == fsecret) {
			fprintf(
					stdout,
					"OK! The API_KEYs are same. and No secret was setting for the service '%s'\n",
					service);
		} else if (NULL == secret) {
			fprintf(
					stderr,
					"ERROR! no secret is available from key-store API, but 1 secret is in the corresponding key file of the service '%s'\n",
					service);
			result = 1;
		} else if (NULL == fsecret) {
			fprintf(
					stderr,
					"ERROR! no secret is available from key file of the service '%s', but 1 secret is in the corresponding key-store API\n",
					service);
			result = 1;
		} else {
			fprintf(
					stdout,
					"OK! The API_KEYs and secrets are same for the service '%s'\n",
					service);
		}

		release_key_secret(&fkey, &fsecret);
		return result;
	}
}

/*
 * const char* sw_keystore_get_key (const char *service)
 */
int test_sw_keystore_get_key(const char *service) {
	const char *key = NULL;
	int result = 0;
	key = sw_keystore_get_key(service);

	if (NULL == key) {
		fprintf(
				stderr,
				"ERROR! the API_KEYs is unavailable from key-store API for the service '%s'.\n",
				service);
		result = 1;
	} else {
		const char *fkey = NULL, *fsecret = NULL;
		read_key_secret_from_keys(service, &fkey, &fsecret);

		if (0 != strcmp(key, fkey)) {
			fprintf(
					stderr,
					"ERROR! the API_KEYs are different.\nFrom LSW-Keystore, it is %s\nIn keys file, it is %s\n",
					key, fkey);
			result = 1;
		} else {
			fprintf(stdout, "OK! the API_KEYs are same.for the service '%s'\n",
					service);
		}
		release_key_secret(&fkey, &fsecret);
	}

	return result;
}

const char
		* USAGE =
				"Usage:\n  \
    -s  the name of the testing service, available service list: flickr, lastfm, twitter, viemo ('flickr' is default)\n";
int main(int argc, char *argv[]) {
	char *svalue = NULL;
	int index = 0;
	int c = -1;

	opterr = 0;

	while ((c = getopt(argc, argv, "s:h")) != -1)
		switch (c) {
		case 's':
			svalue = optarg;
			break;
		case 'h':
			fprintf(stderr, USAGE, optopt);
			return 1;
		case '?':
			if (optopt == 's')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	if (NULL == svalue)
		svalue = "flickr";
	printf("service = %s\n", svalue);

	for (index = optind; index < argc; index++)
		printf("Non-option argument %s\n", argv[index]);

	g_type_init();

	int result = test_sw_keystore_get_key_secret(svalue);
	if (0 == result) {
		result = test_sw_keystore_get_key(svalue);
	}
	return result;
}

#endif /* _TESTCASE_LIBSOCIALWEB_KEYSTORE */
