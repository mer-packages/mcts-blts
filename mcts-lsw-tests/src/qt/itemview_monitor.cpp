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

#include "itemview_monitor.h"

ItemViewMonitor::ItemViewMonitor(SwClientItemView * itemView) {
	this->thisView = itemView;
}

ItemViewMonitor::~ItemViewMonitor() {
	delete this->thisView;
}

void ItemViewMonitor::connectSignals() {
	connect(this->thisView, SIGNAL(ItemsAdded(QList<SwClientItem*> )), this,
			SLOT(
					onItemAddedSignalEmitted(QList<SwClientItem*> )));
	connect(this->thisView, SIGNAL(ItemsChanged(QList<SwClientItem*> )), this,
			SLOT(
					onItemsChangedSignalEmitted(QList<SwClientItem*> )));
	connect(this->thisView, SIGNAL(ItemsRemoved(ArrayOfSwItemId)), this, SLOT(
			onItemsRemovedSignalEmitted(ArrayOfSwItemId)));
}

//Static function
ItemViewMonitor* ItemViewMonitor::createViewMonitor(SwClientService * service,
		QString query) {
	ItemViewMonitor * monitor = 0;
	SwClientItemView * itemView = 0;
	itemView = service->openView(query);

	if (NULL != itemView) {
		qDebug("OK, the instance of item_view is created!");
		monitor = new ItemViewMonitor(itemView);
		qDebug("Going to connect the item-related signals...");
		monitor->connectSignals();
	}
	return monitor;
}

void ItemViewMonitor::startMonitorView() {
	this->thisView->startView();
}

void ItemViewMonitor::onItemAddedSignalEmitted(QList<SwClientItem *> itemsAdded) {
	foreach (SwClientItem *item, itemsAdded)
		{
			qDebug("Receiving a new item!");
			qDebug() << QString("\tService: %1").arg(item->getServiceName());
			qDebug() << QString("\tUUID: %1").arg(item->getSwUUID());
			qDebug() << QString("\tDate: %1").arg(
					item->getDateTime().toString());
			qDebug() << QString("\tType: %1").arg(item->getItemType());
			qDebug() << QString("\tCached: %1").arg(item->getCached());
			qDebug("\tProps:");
			QHash<QString, QString> propsHash = item->getSwItemProps();
			QHash<QString, QString>::iterator i;
			for (i = propsHash.begin(); i != propsHash.end(); ++i) {
				qDebug() << QString("\t\t%1 - %2").arg(i.key(), i.value());
			}
			qDebug("---------------------------------\n");
		}
}

void ItemViewMonitor::onItemsChangedSignalEmitted(
		QList<SwClientItem *> itemsChanged) {
	foreach (SwClientItem *item, itemsChanged)
		{
			qDebug("Receiving a changed item!");
			qDebug() << QString("\tService: %1").arg(item->getServiceName());
			qDebug() << QString("\tUUID: %1").arg(item->getSwUUID());
			qDebug() << QString("\tDate: %1").arg(
					item->getDateTime().toString());
			qDebug() << QString("\tType: %1").arg(item->getItemType());
			qDebug() << QString("\tCached: %1").arg(item->getCached());
			qDebug("\tProps:");
			QHash<QString, QString> propsHash = item->getSwItemProps();
			QHash<QString, QString>::iterator i;
			for (i = propsHash.begin(); i != propsHash.end(); ++i) {
				qDebug() << QString("\t\t%1 - %2").arg(i.key(), i.value());
			}
			qDebug("---------------------------------\n");
		}
}

void ItemViewMonitor::onItemsRemovedSignalEmitted(ArrayOfSwItemId itemsRemoved) {
	foreach (SwItemId itemID, itemsRemoved)
		{
			qDebug("Receiving a removed item!");
			qDebug() << QString("\tUUID: %1").arg(itemID.uuid);
		}
}

void ItemViewMonitor::machel() {

}

void ItemViewMonitor::stopMonitorView() {
	this->thisView->stopView();
}
void ItemViewMonitor::refreshMonitorView() {
	this->thisView->refreshView();
}
void ItemViewMonitor::closeMonitorView() {
	this->thisView->closeView();
}
