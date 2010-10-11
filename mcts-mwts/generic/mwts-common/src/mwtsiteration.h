/* mwtsiteration.h --
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

#ifndef INCLUDED_MWTS_ITERATION_H
#define INCLUDED_MWTS_ITERATION_H

class MwtsMeasure;

#include <QStringList>

class MwtsStep
{
public:
	MwtsStep(QString sName, bool bPassed);

	QString	m_sName;
	bool	m_bPassed;
};

class MwtsIteration
{
public:
	bool IsPassed();
	void AddMeasure(MwtsMeasure* pMeasure);
	void AddStep(MwtsStep* pStep);
	void AddError(QString sError);

//protected:
	QList<MwtsMeasure*> m_listMeasures;
	QStringList m_listErrors;
	QList<MwtsStep*> m_listSteps;

};


#endif //#define INCLUDED_MWTS_ITERATION_H

