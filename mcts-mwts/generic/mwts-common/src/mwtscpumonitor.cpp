/* mwtscpumonitor.cpp --
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

#include "mwtscpumonitor.h"

/** Constructs the CPU monitor object and starts CPU monitoring process*/
MwtsCpuMonitor::MwtsCpuMonitor(MwtsMonitorLogger* logger)
	: MwtsAbstractMonitor(logger)
{
	QStringList params;
	params << QString::number(1);
	m_pMpstatProcess=new QProcess();
	m_pMpstatProcess->start("/usr/bin/mpstat", params);

}

/** Writes result to logger */
void MwtsCpuMonitor::WriteResult()
{
	QString str = m_pMpstatProcess->readAllStandardOutput();

	QStringList lines = str.split("\n");
	int i;
	for(i=0; i<lines.size(); i++)
	{
		QString line=lines[i];
		if (-1 != line.indexOf("Linux")) //this is first line
		{
			continue;
		}

		else if(-1 != line.indexOf("%idle") ) // this is header line
		{
			// check the header positions. locations change a little
			// on different implementations so lets figure them out
			// 02:04:04 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest   %idle
			QStringList headers=line.split(" ", QString::SkipEmptyParts);
			QHash<QString, int> hash;
			for(int j=0; j<headers.size(); j++)
			{
				hash[headers[j]] = j;
			}
			m_nIowaitIndex=hash["%iowait"];
			m_nUserIndex=hash["%usr"];
			if (!m_nUserIndex) // can be also ..
				m_nUserIndex=hash["%user"];
			m_nSysIndex=hash["%sys"];
			m_nIdleIndex=hash["%idle"];
			continue;
		}
		else
		{
			QStringList values=line.split(' ', QString::SkipEmptyParts);
			if(values.size()<5) // empty line
				continue;
			double cpu_user=values.at(m_nUserIndex).toDouble();
			double cpu_sys=values.at(m_nSysIndex).toDouble();
			double cpu_iowait=values.at(m_nIowaitIndex).toDouble();
			double cpu_idle=values.at(m_nIdleIndex).toDouble();
			double cpu_total=100.0-cpu_idle;

			m_pLogger->Write("CpuUsage", cpu_total);
			m_pLogger->Write("CpuIowait", cpu_iowait);
		}
	}


}
/*
Linux 2.6.31-20-generic (tumi-dt) 	03/10/2010 	_i686_	(4 CPU)
02:04:04 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest   %idle
02:04:05 PM  all   15.95    0.00    5.06    0.00    0.00    0.00    0.00    0.00   78.99
02:04:06 PM  all   10.90    0.00    7.02    0.00    0.00    0.00    0.00    0.00   82.08
02:04:07 PM  all    0.80    0.00    1.60    0.00    0.00    0.00    0.00    0.00   97.59
02:04:08 PM  all    1.19    0.00    2.14    0.00    0.00    0.00    0.00    0.00   96.67
02:04:09 PM  all    2.36    0.00    6.28    0.00    0.00    0.00
*/
