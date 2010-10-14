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
 *  -k  the api_key of the testing service (optional)
 *  -c  the secret of the testing service (optional)
 *  -f  the flag for not comparing the api_key and secret with the file '/usr/share/libsocialweb/keys/<service_name>'
 */

#ifndef _TESTCASE_LIBSOCIALWEB_KEYFOB_SYNC
#define _TESTCASE_LIBSOCIALWEB_KEYFOB_SYNC

#include <glib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsocialweb-keyfob/sw-keyfob.h>
#include <libsocialweb-keystore/sw-keystore.h>
#include <rest/oauth-proxy.h>
#include <rest/oauth2-proxy.h>
#include <rest-extras/flickr-proxy.h>

/*
 const char * LASTFM_URL = "http://ws.audioscrobbler.com/2.0/";
 const char * FLICKR_URL = "http://flickr.com/";
 const char * TWITTER_URL = "https://api.twitter.com/";
 const char * VIEMO_URL = "http://vimeo.com/api/v2/%s/";
 const char* LASTFM = "lastfm";
 */
const char* FLICKR = "flickr";
const char* VIEMO = "viemo";
const char* TWITTER = "twitter";
/*
 * some one told me the auth2 will be used for "facebook",
 * but for time being, the service 'facebook' is not available in MeeGo 1.1 yet
 */
const char* FACEBOOK = "facebook";

const char * SERVICE_URLS[] = { "http://ws.audioscrobbler.com/2.0/",
		"https://api.twitter.com/", "http://flickr.com/",
		"http://vimeo.com/api/v2/%s/" };

const char * SERVICES[] = { "lastfm", "twitter", "flickr", "viemo" };

static const char * getServiceURL(const char* serviceName) {
	const char* url = NULL;
	int i = 0;
	for (i = 0; NULL != SERVICES[i]; i++) {
		if (0 == strcmp(serviceName, SERVICES[i])) {
			url = SERVICE_URLS[i];
			break;
		}
	}
	return url;
}

int invoking_oauth_auth_sync(const char* serviceName,
		const char* end_point_url, const char *key, const char *secret) {
	printf("Key = %s; secret = %s\n", key, secret);
	OAuthProxy *proxy = NULL;
	proxy = (OAuthProxy *) oauth_proxy_new(key, secret, end_point_url, FALSE);
	if (NULL != proxy) {
		printf("Successfully create the OAuthProxy object\n");

		if (sw_keyfob_oauth_sync(proxy)) {
			printf("Successfully invoking the sync method, return TRUE\n");
			const char *token = NULL, *token_secret = NULL;
			token = oauth_proxy_get_token(proxy);
			token_secret = oauth_proxy_get_token_secret(proxy);
			printf("Token = %s; TokenSecret = %s\n", token, token_secret);
			return 0;
		} else {
			printf("Successfully invoking the sync method, return False\n");
			return 1;
		}
	}
}

int test_generic_key_fob_auth_sync(const char* serviceName,
		const char* end_point_url, const char *thekey, const char *theSecret) {
	const char *key = NULL, *secret = NULL;
	if (NULL != thekey && NULL != theSecret) {
		return invoking_oauth_auth_sync(serviceName, end_point_url, thekey,
				theSecret);
	} else if (sw_keystore_get_key_secret(serviceName, &key, &secret)) {
		return invoking_oauth_auth_sync(serviceName, end_point_url, key, secret);
	} else {
		fprintf(stderr, "Fail to get the API_KEY and secret\n");
	}
	return 1;
}

/*
 * For now, the auth2 is unavailable yet
 */
int test_generic_key_fob_auth2_sync(const char* serviceName,
		const char* end_point_url) {
	//for now, we don't know where to get the client_id
	return 1;
}

int invoking_flickr_auth_sync(const char *key, const char *secret) {
	printf("Key = %s; secret = %s\n", key, secret);
	FlickrProxy *proxy = (FlickrProxy *) flickr_proxy_new(key, secret);
	if (NULL != proxy) {
		printf("Successfully create the FlickrProxy object\n");

		if (sw_keyfob_flickr_sync(proxy)) {
			printf("Successfully invoking the sync method, return TRUE\n");

			const char *token = NULL;
			token = flickr_proxy_get_token(proxy);
			printf("Token = %s\n", token);
			return 0;
		} else {
			printf("Successfully invoking the sync method, return False\n");
		}
	}
	return 1;
}

int test_flickr_key_fob_sync(const char *thekey, const char *theSecret) {
	const char *key = NULL, *secret = NULL;

	if (NULL != thekey && NULL != theSecret) {
		return invoking_flickr_auth_sync(thekey, theSecret);
	} else if (sw_keystore_get_key_secret("flickr", &key, &secret)) {
		return invoking_flickr_auth_sync(key, secret);
	} else {
		fprintf(stderr, "Fail to get the API_KEY and secret\n");
	}
	return 1;
}

const char
		* USAGE =
				"Usage:\n  \
    -s  the name of the testing service, available service list: flickr, lastfm, twitter, viemo ('flickr' is default)\n\
	-x  setting the service, to run keyfob_auth\n\
	-k  setting the api_key of the service\n\
	-e  setting the shared_secret of the service\n";

int main(int argc, char *argv[]) {
	char *svalue = NULL, *special_key = NULL, *special_secret = NULL;
	int special_service = 0;
	int index = 0;
	int c = -1;

	opterr = 0;

	while ((c = getopt(argc, argv, "s:hxk:e:")) != -1)
		switch (c) {
		case 's':
			svalue = optarg;
			break;
		case 'x':
			special_service = 1;
			break;
		case 'k':
			special_key = optarg;
			break;
		case 'e':
			special_secret = optarg;
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

	const char * service_url = (const char *) getServiceURL(svalue);
	printf("service_url = %s\n", service_url);

	g_type_init();
	int result = 0;
	if (0 == strcmp(svalue, TWITTER) || special_service) {
		result = test_generic_key_fob_auth_sync(svalue, service_url,
				special_key, special_secret);
	} else if (0 == strcmp(svalue, FLICKR)) {
		result = test_flickr_key_fob_sync(special_key, special_secret);
	} else if (0 == strcmp(svalue, FACEBOOK)) {
		fprintf(stderr,
				"For time being, the service 'Facebook' is unavailable yet!\n");
	} else {
		fprintf(stderr,
				"For time being, the service '%s' is not available yet!\n",
				svalue);
	}
	return result;
}

#endif /* _TESTCASE_LIBSOCIALWEB_KEYFOB_SYNC */
