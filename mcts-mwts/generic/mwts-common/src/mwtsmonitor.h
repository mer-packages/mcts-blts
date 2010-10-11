/* mwtsmonitor.cpp --
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

#ifndef _INCLUDED_MWTS_MONITOR_H
#define _INCLUDED_MWTS_MONITOR_H


#include <MwtsCommon>
#include "mwtsmemorymeasurement.h"

class MwtsMemoryMonitor;
class MwtsCpuMonitor;
class MwtsMonitorLogger;
class MwtsMonitorThread;
class MwtsEndurance;

/** Class for monitoring system resources*/

class MWTSCOMMON_EXPORT MwtsMonitor : public QObject
{
	Q_OBJECT
public:
	static MwtsMonitor* instance(); // get singleton instance
	virtual ~MwtsMonitor();

	void Start();
	void Stop();
	void NextIteration();
	void WriteResults();
private:
	MwtsMonitor();
	static MwtsMonitor* inst;
	MwtsMonitorThread* m_pThread;
	MwtsEndurance* m_pEndurance;
	int m_nEnduranceStep;
	int m_nIteration;
	QList<double> m_listMemoryUsages;
	MwtsMemoryMeasurement m_memoryMeasurement;

};

#endif //#ifndef _INCLUDED_MWTS_MONITOR_H
