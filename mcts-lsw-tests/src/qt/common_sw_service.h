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

#ifndef COMMON_SW_SERVICE_H_
#define COMMON_SW_SERVICE_H_

#include <QObject>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include <libsocialweb-qt/swclientservice.h>

#define USER_CHANGED_SIGNAL_FILE QString("/tmp/qt.user.changed.signal")
#define USER_CHANGED_SIGNAL QString("UserChanged signal")

#define DYN_CAPS_SIGNAL_FILE QString("/tmp/qt.dyn.caps.signal")
#define DYN_CAPS_SIGNAL QString("DynCapsChanged signal")

#define ITEM_HIDDEN_SIGNAL_FILE QString("/tmp/qt.item.hidden.signal")
#define ITEM_HIDDEN_SIGNAL QString("ItemHidden signal")

class CommonSwService: public QObject {
Q_OBJECT
public:
	~CommonSwService();
	static CommonSwService* createCommonSwService(SwClientService *service);

private slots:
	void onUserChanged(SwClientService *service);
	void onDynCapsChanged(SwClientService *service, QStringList newDynCaps);
	void onItemHidden(SwClientService *service, const QString &uuid);

private:
	CommonSwService(SwClientService *service);
	SwClientService * this_service;
	void connectServiceSignals();
};
#endif /* COMMON_SW_SERVICE_H_ */
