/* mwtsglobal.cpp --
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

#include "stable.h"
#include "MwtsCommon"

/** Global pointer to log currently in use */
MwtsLog* g_pLog=NULL;

/** Global pointer to main application */
MwtsApp* g_pApp=NULL;

/** Global pointer to main window. Created if UI enabled. */
QMainWindow* g_pMainWindow=NULL;

/** Global pointer to currently used test object */
MwtsTest* g_pTest=NULL;

/** Global pointer to currently used result object*/
MwtsResult* g_pResult=NULL;

/** Global pointer to CPU load object which is always available */
MwtsCpuLoad* g_pCpuLoad=NULL;

/** Global pointer to memory load class which is always available*/
MwtsMemLoad* g_pMemLoad=NULL;

/** Global pointer to throughput measuring object which is always available */
MwtsIPerfThroughput* g_pIPerfThroughput=NULL;

/** Global pointer to current configuration */
MwtsConfig* g_pConfig=NULL;

/** Global pointer to time measurement object */
QTime* g_pTime=NULL;

/** Global pointer to monitor object */
MwtsMonitor* g_pMonitor=NULL;

