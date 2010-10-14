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

#ifndef ITEMVIEW_MONITOR_H_
#define ITEMVIEW_MONITOR_H_

#include <QObject>
#include <QtDBus>
#include <libsocialweb-qt/swclient.h>
#include <libsocialweb-qt/swclientitem.h>

#define CACHE_FILE_NAME QString("/tmp/qt.lastfm.items.cache")

class ItemViewMonitor: public QObject {
Q_OBJECT
public:
	~ItemViewMonitor();
	static ItemViewMonitor* createViewMonitor(SwClientService * service,
			QString query);
	void startMonitorView();
	void stopMonitorView();
	void refreshMonitorView();
	void closeMonitorView();

private slots:
	void onItemAddedSignalEmitted(QList<SwClientItem *> itemsAdded);
	void onItemsChangedSignalEmitted(QList<SwClientItem *> itemsChanged);
	void onItemsRemovedSignalEmitted(ArrayOfSwItemId itemsRemoved);

private:
	ItemViewMonitor(SwClientItemView * itemView);
	void connectSignals();
	SwClientItemView * thisView;
	QHash<QString, SwClientItem*> thisMap;
	void machel();
};
#endif /* ITEMVIEW_MONITOR_H_ */
