/*
 *  Copyright (c) 2010, Intel Corporation.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU Lesser General Public License,
 *  version 2.1, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Created on: 2010-9-24
 *      Author: Tang, Shao-Feng (shaofeng.tang@intel.com)
 *
 */

#include <QtCore>
#include <QApplication>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include "lsw_testcases.h"
#include "common_sw_service.h"
#include <unistd.h>

const char
		* USAGE =
				"Usage:\n  \
    -s  the name of the service, such as: lastfm, flickr, twitter. The default is 'lastfm'\n";

int main(int argc, char *argv[]) {
	char *service_name = NULL;
	int c = -1;

	opterr = 0;

	while ((c = getopt(argc, argv, "s:h")) != -1)
		switch (c) {
		case 's':
			service_name = optarg;
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

	QApplication a(argc, argv);

	if(NULL == service_name){
		service_name = "lastfm";
	}

	QString qserviceName(service_name);
	qDebug() << "Going to invoke the method 'getService' for the "
			<< qserviceName << " instance of 'SwClientService'";

	SwClient* client = new SwClient();
	SwClientService * service = client->getService(qserviceName);

	qDebug() << "listenning the 'ItemHidden' signals";
	CommonSwService* common_service = CommonSwService::createCommonSwService(
			service);

	return a.exec();
}
