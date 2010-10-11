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

#include "stable.h"
#include "mwtsmonitor.h"
#include "mwtsmonitorlogger.h"
#include "mwtscpumonitor.h"
#include "mwtsmemorymonitor.h"
#include "mwtsendurance.h"
#include "mwtsstatistics.h"


/************ MwtsMonitorThread *************/

/** thread class for the monitor*/
class MwtsMonitorThread : public QThread
{
public:
	void run();
};

/** Runs the monitor thread */
void MwtsMonitorThread::run()
{
	MWTS_ENTER;

	MwtsMonitorLogger* logger = new MwtsMonitorLogger();
	MwtsMemoryMonitor* memMonitor = new MwtsMemoryMonitor(logger);
	MwtsCpuMonitor* cpuMonitor = new MwtsCpuMonitor(logger);

	// start the event loop
	exec();

	MWTS_LEAVE;
	delete logger;
	delete memMonitor;
	delete cpuMonitor;

}




/************ MwtsMonitor *************/

MwtsMonitor* MwtsMonitor::inst = NULL;
/** Static method that returns pointer to singleton monitor object */
MwtsMonitor* MwtsMonitor::instance()
{
	if(!inst)
		inst=new MwtsMonitor;
	return inst;
}

/** Constructs and setus up the monitor */
MwtsMonitor::MwtsMonitor()
{
	m_pEndurance=NULL;
	m_pThread=NULL;
	QString sStep=getenv("MWTS_ENDURANCE");
	if(sStep=="")
	{
		m_pEndurance=NULL;
	}
	else
	{
		m_pEndurance=new MwtsEndurance;
		m_nEnduranceStep=sStep.toInt();
		m_pEndurance->GatherData("Initial");
	}
	m_nIteration=0;
}

MwtsMonitor::~MwtsMonitor()
{
	this->Stop();
}

/** Starts the monitoring thread*/
void MwtsMonitor::Start()
{
	m_pThread=new MwtsMonitorThread();
	m_pThread->start(QThread::LowPriority);
}
void MwtsMonitor::Stop()
{
	if(m_pThread)
	{
		qDebug() << "Quitting monitor thread";
		m_pThread->quit();
		m_pThread->wait(10000);
		if(!m_pThread->isFinished())
		{
			qWarning() << "Terminating monitor thread";
			m_pThread->terminate();
			m_pThread->wait(10000);
		}
		delete m_pThread;
		m_pThread=NULL;
	}
	if(m_pEndurance)
	{
		m_pEndurance->GatherData("Final");
		delete m_pEndurance;
		m_pEndurance = NULL;
	}
}

/** Starts next iteration */
void MwtsMonitor::NextIteration()
{
	m_nIteration++;
	if(m_pEndurance && m_nIteration%m_nEnduranceStep==0)
	{
		QString sStep = "Iteration " + QString::number(m_nIteration);
		m_pEndurance->GatherData(sStep);
	}
	m_memoryMeasurement.Update();
	m_listMemoryUsages.append(m_memoryMeasurement.GetUsed()/1024.0);
}

/** Writes monitor summary to result file */
void MwtsMonitor::WriteResults()
{
	QString str;
	MwtsStatistics stat(m_listMemoryUsages);
	str.sprintf("Memory usage: %.2lf+/-%.2lf MB",
		    stat.Mean(),
		    stat.Stdev());
	g_pResult->Write(str);
	str.sprintf("Memory diff: %.2lf MB",
		    m_listMemoryUsages.last() - m_listMemoryUsages.first());
	g_pResult->Write(str);
}

