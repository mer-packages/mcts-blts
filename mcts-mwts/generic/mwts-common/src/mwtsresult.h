/* mwtsresult.h --
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

#ifndef _INCLUDED_MWTS_RESULT_H
#define _INCLUDED_MWTS_RESULT_H

#include "mwtsglobal.h"
#include "mwtsmeasure.h"
#include "mwtsiteration.h"

#include <QFile>

/** Class for writing test case results and creating report */
class MWTSCOMMON_EXPORT MwtsResult
{
public:
	static MwtsResult* instance();
	virtual ~MwtsResult();

	void Open(QString sCasename);
	void Close();
	void Write(QString text);
	void StepPassed(QString step, bool bPassed);
	void AddMeasure(QString name, double value, QString unit);
	void NextIteration();
	void WriteReport();
	void AddError(QString sError);
	bool IsPassed();
	void SetLimits(double lfTarget, double lfFail);
	void SetResultFilter(QString filter);
	void StartSeriesMeasure(QString name, QString unit, double lfTarget=0, double lfFailLimit=0);
	void AddSeriesMeasure(QString name, double value);

protected:
	static MwtsResult* inst;
	MwtsResult();
	
	void WriteSeriesFile(QString name, QString text);

	QList<MwtsIteration*> m_listIterations;
	MwtsIteration*	m_pCurrentIteration;
	MwtsIteration*	m_pInitIteration;
	int m_nIteration;
	QFile m_File;
	double m_lfTarget;
	double m_lfFailLimit;
	QString m_sResultFilter;
	
	bool ReadLimitsFromFile();

	void WriteXmlReport(QString sMeasureName, QString sValue,
		QString sMeasureUnit, QString sFailLimit, QString sTarget);

	void WriteCsvReport(QString sMeasureName, QString sValue,
		QString sMeasureUnit, QString sFailLimit, QString sTarget);

	bool m_bSpEnducance;
};



#endif // #ifndef _INCLUDED_MWTS_RESULT_H

