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
#include "service_parent.h"
#include "lsw_testcases.h"

TcServiceParent::TcServiceParent() :
	QObject(), mClient(new SwClient(QDBusConnection::sessionBus(), this)) {

}

int TcServiceParent::checkServices() {
	QStringList* services = mClient->getServices();

	/*SwClientService * cs = client->getService("xxxx");

	 if(NULL == cs){

	 }*/

	if (NULL != services && services->size() > 0) {
		int result = SUCCESS_RETURN;
		foreach (QString sName, *services)
			{
				//for (int i = 0; i < services->size(); i++) {
				qDebug() << "The service " << sName << "is available!";
				SwClientService * clientService = mClient->getService(sName);
				if (NULL != clientService) {
					qDebug() << "The instance of the service " << sName
							<< " is available. and its' display name is"
							<< clientService->getDisplayName();
				} else {
					result = FAILURE_RETURN;
					qDebug() << "Fail! The instance of the service " << sName
							<< " is not available.";
				}
			}
		return result;
	} else {
		qDebug("fail to invoke the method 'getServices'");
		return FAILURE_RETURN;
	}
}
