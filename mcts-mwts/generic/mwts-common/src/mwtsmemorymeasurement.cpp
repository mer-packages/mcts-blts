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
#include "mwtsmemorymeasurement.h"

MwtsMemoryMeasurement::MwtsMemoryMeasurement()
{
}
/** Updates memory measurement values.
Remember to call this before GetFree or GetUsed
*/
void MwtsMemoryMeasurement::Update()
{
	QFile file("/proc/meminfo");
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qCritical()<<"Failed to open /proc/meminfo";
		return;
	}
	QTextStream in(&file);
	QString line = in.readLine();
	while (!line.isNull())
	{
		QStringList data=line.split(" ", QString::SkipEmptyParts);
		QString name=data[0].split(":")[0];
		values[name] = data[1].toInt();
		line = in.readLine();
	}
}
/** Gets amount of free memory in kilobytes*/
int MwtsMemoryMeasurement::GetFree()
{
	int free=values["MemFree"];
	int cached=values["Cached"];
	return free+cached;
}

/** Gets amount of used memory in kilobytes*/
int MwtsMemoryMeasurement::GetUsed()
{
	int total=values["MemTotal"];
	int free=values["MemFree"];
	int cached=values["Cached"];
	return total-free-cached;
}


/* /proc/meminfo data
MemTotal:         489944 kB
MemFree:          214416 kB
Buffers:               0 kB
Cached:            87704 kB
SwapCached:            0 kB
Active:           114836 kB
Inactive:          52368 kB
Active(anon):     105384 kB
Inactive(anon):        0 kB
Active(file):       9452 kB
Inactive(file):    52368 kB
Unevictable:       31016 kB
Mlocked:           31016 kB
SwapTotal:             0 kB
SwapFree:              0 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:        110532 kB
Mapped:           116348 kB
Shmem:             14424 kB
Slab:              13256 kB
SReclaimable:       4864 kB
SUnreclaim:         8392 kB
KernelStack:        1424 kB
PageTables:         3220 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:      244972 kB
Committed_AS:     534104 kB
VmallocTotal:     385024 kB
VmallocUsed:       13272 kB
VmallocChunk:     364332 kB
*/

