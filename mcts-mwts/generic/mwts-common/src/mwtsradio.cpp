/* mwtsradio.cpp -- radio access mode change
 *
 * This file is part of MCTS 
 * 
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
 * 
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com; 
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * version 2.1 as published by the Free Software Foundation. 
 * 
 * This library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
 * 02110-1301 USA 
 *
 */

#include "mwtsradio.h"
#include <MwtsCommon>
#include <QDBusConnection>
#include <QDBusInterface>

/** Changes radio access technology mode*/
bool MwtsRadio::ChangeMode(int mode)
{
	MWTS_ENTER;

	QString sMode;

	switch (mode)
	{
	case RADIO_ANY:
		sMode="any";
		break;
	case RADIO_GSM:
		sMode="gsm";
		break;
	case RADIO_UMTS:
		sMode="umts";
		break;
	case RADIO_LTE:
		sMode="lte";
		break;
	default:
		qCritical() << "Invalid radio technology preference";
		return false;
	}

	QDBusMessage reply;

	qDebug() << "Opening Manager interface";
	QDBusInterface manager("org.ofono", "/", "org.ofono.Manager", QDBusConnection::systemBus());

	qDebug() << "Querying modems";
	reply = manager.call("GetModems");

	QString modemPath=reply.arguments()[0].toString();
	
	qDebug() << "Opening RadioSettings interface";
	QDBusInterface radioSettings("org.ofono", modemPath, "org.ofono.RadioSettings", QDBusConnection::systemBus());

	qDebug() << "Calling TechnologyPreference";
	radioSettings.call("TechnologyPreference", sMode);

	MWTS_LEAVE;
	return true;
}


