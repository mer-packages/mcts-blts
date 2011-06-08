/* mwtsmemorymeasurement.cpp --
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
#include "mwtsmemorymonitor.h"
#include "mwtsmonitorlogger.h"
#include "MwtsCommon"

MwtsMemoryMonitor::MwtsMemoryMonitor(MwtsMonitorLogger* logger)
	: MwtsAbstractMonitor(logger)
{

}

/** Writes memory consumption results to logger */
void MwtsMemoryMonitor::WriteResult()
{
	if(!m_pLogger)
		return;
	measurement.Update();

	//write used and free memory measurements to .result file
	m_pLogger->Write("UsedMem", measurement.GetUsed()/1024.0);
	m_pLogger->Write("FreeMem", measurement.GetFree()/1024.0);

	//added used memory measurement to .csv file for testrunner-lite
	g_pResult->AddSeriesMeasure("mem_used", QString::number(measurement.GetUsed()/1024.0), "MB");
	//added free memory measurement to .csv file for testrunner-lite
	g_pResult->AddSeriesMeasure("mem_free", QString::number(measurement.GetFree()/1024.0), "MB");
}

