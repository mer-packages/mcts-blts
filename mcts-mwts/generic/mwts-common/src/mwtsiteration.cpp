/* mwtsiteration.cpp --
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
#include "mwtsiteration.h"
#include <MwtsCommon>

MwtsStep::MwtsStep(QString sName, bool bPassed)
{
	m_sName=sName;
	m_bPassed=bPassed;
}

/** Tells whether this iteration is passed or not.*/
bool MwtsIteration::IsPassed()
{	
	if(m_listErrors.size()>0)
		return false;
	for(int i=0; i<m_listSteps.size(); i++)
		if(!m_listSteps.at(i)->m_bPassed)
			return false;
	// todo : check measurements and compare to limits
	return true;
}

/** Adds measurement value to this iteration.*/
void MwtsIteration::AddMeasure(MwtsMeasure* pMeasure)
{
	MWTS_ENTER;
	m_listMeasures.append(pMeasure);
}

/** Adds a new step to iteration */
void MwtsIteration::AddStep(MwtsStep* pStep)
{
	MWTS_ENTER;
	m_listSteps.append(pStep);
}

/** Adds error string to this iteration*/
void MwtsIteration::AddError(QString sError)
{
	MWTS_ENTER;
	m_listErrors.append(sError);
}

