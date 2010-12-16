/* mwtsradio.h -- radio access mode change
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

#ifndef _INCLUDED_MWTS_RADIO_H
#define _INCLUDED_MWTS_RADIO_H

/** Class for handling radio access technology */

#define RADIO_ANY	1
#define RADIO_GSM	2
#define RADIO_UMTS	3
#define RADIO_LTE	4

#include <QObject>

// ofono-qt missing currently from MeeGo images. You can enable mode change by
// - compiling and installing ofono-qt from gitorious:
//   git://gitorious.org/meego-cellular/ofono-qt.git
// - uncomment following line, recompile and install mwts-common:
// #define OFONO_QT_EXISTS

#ifdef OFONO_QT_EXISTS
	#include <ofono-qt/ofonoradiosettings.h>
#endif


class MwtsRadio : public QObject
{
	Q_OBJECT
public:
	static MwtsRadio* instance();
private:
	static MwtsRadio* inst;


public:
	bool ChangeMode(int mode);

protected slots:
	void onRadioModeChanged();

protected:
	MwtsRadio();
#ifdef OFONO_QT_EXISTS
	OfonoRadioSettings* m_radioSettings;
#endif
};


#endif // #ifndef _INCLUDED_MWTS_RADIO_H
