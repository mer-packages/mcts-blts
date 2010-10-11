/* mwtsendurance.cpp --
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
#include "mwtsendurance.h"

/** Creates the object and initializes endurance reporting*/
MwtsEndurance::MwtsEndurance()
{
	// setenv("DISPLAY", ":0.0", 1); 
}

/** Gathers endurance data using some endurance data gathering tool.*/
void MwtsEndurance::GatherData(QString sStepName)
{
	QProcess process;

	QStringList params;
	QString casename=g_pTest->CaseName();
	casename.replace(" ", "_");
	sStepName.replace(" ", "_");
	params << casename << sStepName;
	process.setWorkingDirectory("/home/user/MyDocs");


	// todo: gathering the endurance data
	qWarning() << "Gathering endurance data not implemented yet!";

}
