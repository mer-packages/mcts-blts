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
#include <libsocialweb-qt/swclient.h>
#include "lsw_testcases.h"

int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);
	qDebug("Going to invoke the method 'isOnline'");

	//qDebug() << SwClient::GetSwServiceName();

	SwClient* client = new SwClient();
	bool isOn = client->isOnline();

	if (true == isOn) {
		qDebug("The current status is online!");
	} else {
		qDebug("The current status is off-line!");
	}

	return SUCCESS_RETURN;
}

