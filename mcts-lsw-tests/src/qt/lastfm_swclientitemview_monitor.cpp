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
#include <QCoreApplication>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include <libsocialweb-qt/swclientdbustypes.h>
#include "lsw_testcases.h"
#include "itemview_monitor.h"

#define ICON_FILE_PATH QString("/usr/share/libsocialweb/services/%1.png")

const QString LASTFM_SERVICE_NAME("lastfm");
const QString LASTFM_QUERY("feed");
const SwParams EMPTY_PARAMS;

int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);
	qDebug() << "Going to invoke the method 'getService' for the "
			<< LASTFM_SERVICE_NAME << " instance of 'SwClientService'";

	qDebug() << (ICON_FILE_PATH.arg(LASTFM_SERVICE_NAME));

	SwClient* client = new SwClient();
	SwClientService * service = client->getService(LASTFM_SERVICE_NAME);

	ItemViewMonitor* monitor = ItemViewMonitor::createViewMonitor(service,
			LASTFM_QUERY);
	monitor->startMonitorView();

	return a.exec();
}
