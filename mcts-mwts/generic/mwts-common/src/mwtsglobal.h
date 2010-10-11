/* mwtsglobal.h --
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
 * */

#ifndef _INCLUDED_MWTS_GLOBAL_H
#define _INCLUDED_MWTS_GLOBAL_H

#include <QString>
#include <QWidget>
#include <QTime>
#include <QMainWindow>
#include <QDebug>

class MwtsLog;
class MwtsApp;
class MwtsResult;
class MwtsTest;
class MwtsMemLoad;
class MwtsCpuLoad;
class MwtsIPerfThroughput;
class MwtsConfig;


extern MwtsLog* g_pLog;
extern MwtsApp* g_pApp;
extern MwtsResult* g_pResult;
extern QMainWindow* g_pMainWindow;
extern MwtsTest* g_pTest;
extern MwtsCpuLoad* g_pCpuLoad;
extern MwtsMemLoad* g_pMemLoad;
extern MwtsCpuLoad* g_pCpuLoad;
extern MwtsIPerfThroughput* g_pIPerfThroughput;
extern MwtsConfig* g_pConfig;
extern QTime* g_pTime;


#define MWTS_DEBUG(str)		g_pLog->Debug(str)
#define MWTS_WARNING(str)	g_pLog->Warning(str)
#define MWTS_ERROR(str)		g_pLog->Error(str)

#define MWTS_ENTER	qDebug() <<  "Entered " << __PRETTY_FUNCTION__
#define MWTS_LEAVE	qDebug() <<  "Leaving " << __PRETTY_FUNCTION__


#endif // #ifndef _INCLUDED_MWTS_GLOBAL_H

