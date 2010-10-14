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

#include "common_sw_service.h"
#include <QFile>
#include <QTextStream>

CommonSwService::CommonSwService(SwClientService *service) {
	this->this_service = service;
}

CommonSwService::~CommonSwService() {
	delete this->this_service;
}

CommonSwService* CommonSwService::createCommonSwService(
		SwClientService *service) {
	CommonSwService * commonService = NULL;
	if (NULL != service) {
		commonService = new CommonSwService(service);
		commonService->connectServiceSignals();
	}
	return commonService;
}

void CommonSwService::onItemHidden(SwClientService *service,
		const QString &uuid) {
	qDebug()
			<< "OK! Receiving a signal 'ItemHidden' with a new service, and a uuid:"
			<< uuid;
	QFile f(ITEM_HIDDEN_SIGNAL_FILE);
	if (f.open(QIODevice::WriteOnly)) {
		QTextStream t(&f);
		t << ITEM_HIDDEN_SIGNAL;
		f.close();
	}
	if (NULL != service) {
		disconnect(this->this_service,
				SIGNAL(ItemHidden(SwClientService *, const QString &)), this,
				SLOT(onItemHidden(SwClientService *, const QString &)));
		this->this_service = service;
		connect(this->this_service,
				SIGNAL(ItemHidden(SwClientService *, const QString &)), this,
				SLOT(onItemHidden(SwClientService *, const QString &)));
	} else {
		qDebug() << "The new service is null";
	}
}

void CommonSwService::onUserChanged(SwClientService *service) {
	qDebug() << "OK! Receiving a signal 'UserChanged' with a new service";
	QFile f(USER_CHANGED_SIGNAL_FILE);
	if (f.open(QIODevice::WriteOnly)) {
		QTextStream t(&f);
		t << USER_CHANGED_SIGNAL;
		f.close();
	}
	if (NULL != service) {
		disconnect(this->this_service, SIGNAL(UserChanged(SwClientService *)),
				this, SLOT(onUserChanged(SwClientService *)));
		this->this_service = service;
		connect(this->this_service, SIGNAL(UserChanged(SwClientService *)),
				this, SLOT(onUserChanged(SwClientService *)));
	} else {
		qDebug() << "The new service is null";
	}
}

void CommonSwService::onDynCapsChanged(SwClientService *service,
		QStringList newDynCaps) {
	qDebug() << "OK! Receiving a signal 'DynCapsChanged' with a new service";
	QFile f(DYN_CAPS_SIGNAL_FILE);
	if (f.open(QIODevice::WriteOnly)) {
		QTextStream t(&f);
		t << DYN_CAPS_SIGNAL;
		f.close();
	}
	if (NULL != service) {
		disconnect(this->this_service,
				SIGNAL(DynCapsChanged(SwClientService *, QStringList)), this,
				SLOT(onDynCapsChanged(SwClientService *,
								QStringList)));
		this->this_service = service;
		connect(this->this_service,
				SIGNAL(DynCapsChanged(SwClientService *, QStringList)), this,
				SLOT(onDynCapsChanged(SwClientService *,
								QStringList)));
	} else {
		qDebug() << "The new service is null";
	}
	if (newDynCaps.size() > 0) {
		qDebug() << "The new dynamic capabilities are: ";
		for (int i = 0; i < newDynCaps.size(); i++) {
			qDebug() << newDynCaps.at(i);
		}
	} else {
		qDebug("No dynamic capabilities are available");
	}
}

void CommonSwService::connectServiceSignals() {
	connect(this->this_service, SIGNAL(UserChanged(SwClientService *)), this,
			SLOT(onUserChanged(SwClientService *)));
	connect(this->this_service,
			SIGNAL(DynCapsChanged(SwClientService *, QStringList)), this,
			SLOT(onDynCapsChanged(SwClientService *,QStringList)));
	connect(this->this_service,
			SIGNAL(ItemHidden(SwClientService *, const QString &)), this,
			SLOT(onItemHidden(SwClientService *, const QString &)));
}
