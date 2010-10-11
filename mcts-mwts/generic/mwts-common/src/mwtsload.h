/* mwtsload.h --
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

#ifndef _INCLUDED_MWTS_LOAD_H
#define _INCLUDED_MWTS_LOAD_H

#include <MwtsCommon>
#include <QObject>
#include <QProcess>

/** Base class for creating different kinds of load */
class MWTSCOMMON_EXPORT MwtsLoad
{
public:
	MwtsLoad();
	virtual ~MwtsLoad();

	bool Start();
	void Stop();

	void SetAmount(int nAmount);
protected:
	/** Pure virtual external application execute function
	This is implemented in subclasses to do actual execution*/
	virtual bool Exec() = 0;

	QProcess* m_pProcess;

protected:
	int m_nAmount;

};

/** Class for creating CPU load */
class MWTSCOMMON_EXPORT MwtsCpuLoad : public MwtsLoad
{
protected:
	bool Exec();
};

/** Class for creating memory load */
class MWTSCOMMON_EXPORT MwtsMemLoad : public MwtsLoad
{
protected:
	bool Exec();
};



#endif // #ifndef _INCLUDED_MWTS_LOAD_H

