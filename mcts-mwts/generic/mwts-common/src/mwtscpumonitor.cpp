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
#include "MwtsCommon"

/** Constructs the CPU monitor object and starts CPU monitoring process*/
MwtsCpuMonitor::MwtsCpuMonitor(MwtsMonitorLogger* logger)
	: MwtsAbstractMonitor(logger)
{
	QStringList params;
    //n - causes the headers not to be reprinted regularly.
    //1 - updates in every second
    params << QString("-n") << QString::number(1);
	m_pMpstatProcess=new QProcess();
	//it might be that float precision is needed (note: no mpstat on meego)
	m_pMpstatProcess->start("/usr/bin/vmstat", params);
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
        if (-1 != line.indexOf("procs")) //this is first line
		{	
			continue;
		}

        else if(-1 != line.indexOf("st") ) // this is header line
		{
			// check the header positions. locations change a little
			// on different implementations so lets figure them out            
            // r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
			QStringList headers=line.split(" ", QString::SkipEmptyParts);
			QHash<QString, int> hash;
			for(int j=0; j<headers.size(); j++)
			{
				hash[headers[j]] = j;
			}
            m_nIowaitIndex=hash["wa"];
            m_nUserIndex=hash["us"];
            //if (!m_nUserIndex) // can be also ..
            //	m_nUserIndex=hash["%user"];
            m_nSysIndex=hash["sy"];
            m_nIdleIndex=hash["id"];
			continue;
		}
		else
		{
			QStringList values=line.split(' ', QString::SkipEmptyParts);
			if(values.size()<5)
			{// empty line
				continue;
			}
			int cpu_user=values.at(m_nUserIndex).toInt();
            int cpu_sys=values.at(m_nSysIndex).toInt();
            int cpu_iowait=values.at(m_nIowaitIndex).toInt();
            int cpu_idle=values.at(m_nIdleIndex).toInt();
            int cpu_total=100-cpu_idle;

			m_pLogger->Write("CpuUsage", cpu_total);
			m_pLogger->Write("CpuIowait", cpu_iowait);

			//added cpu usage measurement to .csv file for testrunner-lite
			g_pResult->AddSeriesMeasure("cpu_usage", QString::number(cpu_total), "%");
            //added cpu io wait measurement to .csv file for testrunner-lite
			g_pResult->AddSeriesMeasure("cpu_io_wait", QString::number(cpu_iowait), "%");
		}
	}


}
/*
procs -----------memory---------- ---swap-- -----io---- --system-- -----cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 3  0    428  13812     56 436412    0    0     3     7  184  224  5  1 93  1  0
 2  0    428  13928     56 436412    0    0     0     0 2086  650 100  0  0  0  0
 3  0    428  13928     56 436412    0    0     0     0 2074  571 100  0  0  0  0
 2  0    428  13928     56 436412    0    0     0     0 2078  576 100  0  0  0  0
 2  0    428  13944     56 436412    0    0     0     0 2067  577 100  0  0  0  0
 5  0    428  13960     56 436412    0    0     0     0 2151  798 100  1  0  0  0
*/
