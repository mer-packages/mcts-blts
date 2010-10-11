/* mwtsload.cpp --
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
#include "mwtsload.h"

/**
  Constructor of MwtsLoad base class
*/

MwtsLoad::MwtsLoad()
{
	MWTS_ENTER;
	m_pProcess=new QProcess();
	m_nAmount=0;
}

MwtsLoad::~MwtsLoad()
{
// todo: fix hanging of destructor
//	Stop();
//	delete m_pProcess;
}

/**
  Sets amount of load.
  @param nAmount amount of load wanted. Unit is dependent of sub class implementation
*/
void MwtsLoad::SetAmount(int nAmount)
{
	MWTS_ENTER;
	m_nAmount=nAmount;
}

/**
  Generates the load. Starts the load process. Runs Exec() which is implemented in sub classes.
*/
bool MwtsLoad::Start()
{
	MWTS_ENTER;
	return this->Exec(); // implemented in subclasses
}
/**
  Stops the load. Kills the process generating the load. This is automatically called by the destructor.
*/
void MwtsLoad::Stop()
{
	MWTS_ENTER;
	if(m_pProcess->pid() != 0)
	{
		m_pProcess->kill();
	}
}


/* execution implementations */

/**
  Load execution implementation of CPU load class
*/
bool MwtsCpuLoad::Exec()
{
	MWTS_ENTER;
	QStringList params;
	params << (QString) m_nAmount;
	m_pProcess->start("/usr/bin/cpuload", params);
	// todo check that it really runs
	return true;
}

/**
  Load execution implementation of memory load class
*/
bool MwtsMemLoad::Exec()
{
	MWTS_ENTER;
	QStringList params;
	params << (QString) m_nAmount;
	m_pProcess->start("/usr/bin/memload", params);
	// todo check that it really runs
	return true;
}



