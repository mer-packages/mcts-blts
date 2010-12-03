/* mwtsresult.cpp --
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
#include <MwtsCommon>
#include "math.h"
#include <QDomDocument>
#include "mwtsstatistics.h"


MwtsResult* MwtsResult::inst = NULL;
/** This static method returns instance to singleton result object*/
MwtsResult* MwtsResult::instance()
{
	if(!inst)
		inst=new MwtsResult;
	return inst;
}

MwtsResult::MwtsResult()
{
	m_nIteration=0;
	m_pInitIteration=new MwtsIteration;
	m_pCurrentIteration=m_pInitIteration;
	MWTS_ENTER;
}

MwtsResult::~MwtsResult()
{
}

/**
  Opens result file and initializes result
  @param: CaseName name of the test case
*/

void MwtsResult::Open(QString sCaseName)
{
	MWTS_ENTER;
	if(m_File.isOpen())
	{
		m_File.close();
	}
	m_File.setFileName("/var/log/tests/"+sCaseName+".result");
	m_File.open(QIODevice::WriteOnly);

	QString text;
	QTextStream(&text) << "--- Test '" << sCaseName << "' started "; // todo : add time like: Sat Jan  1 01:32:22 2000
	Write(text);
}

/**
  Closes the result file
*/
void MwtsResult::Close()
{
	MWTS_ENTER;
	m_File.close();
	MWTS_LEAVE;
}

/**
  Writes message in the result file
  @param: Text string to be written in file
*/
void MwtsResult::Write(QString text)
{
	MWTS_ENTER;
	m_File.write(text.toLatin1()+"\n");
	m_File.flush();
}

/**
  Adds error to result
*/
void MwtsResult::AddError(QString sError)
{
	Write("Error: "+sError);
	m_pCurrentIteration->AddError(sError);
}

/**
  Marks whether test step is passed or not
  @param: Text string to be written in file
*/
void MwtsResult::StepPassed(QString step, bool bPassed)
{
	MWTS_ENTER;
	m_pCurrentIteration->AddStep(new MwtsStep(step, bPassed));
	if(bPassed)
	{
		Write("- " + g_pTest->Name() + ": "+ step + ": passed");
	}
	else
	{
		Write("- " + g_pTest->Name() + ": "+ step + ": failed");
	}

}


/**
  Sets target and fail limit for measurements. These are written to test output and
  the test will be marked failed if median of measurements are below fail limit
  @param lfTarget Target that should be exceeded in final product
  @param lfFail Limit that must be exceeded for test case to pass
*/

void MwtsResult::SetLimits(double lfTarget, double lfFail)
{
	MWTS_ENTER;
	qDebug() << "Setting limits (" << lfTarget << "," << lfFail << ")";
	m_lfFailLimit=lfFail;
	m_lfTarget=lfTarget;
}


/**
  Adds a measurement value in to results
  @param: Name name of the measurement
  @param: Value value to be added
  @param: Unit unit of the measurement, for example "kB/s"
*/
void MwtsResult::AddMeasure(QString name, double value, QString unit)
{
	MWTS_ENTER;
	m_pCurrentIteration->AddMeasure(new MwtsMeasure(name, value, unit));
	QString text;
	QTextStream(&text) << "- " << g_pTest->Name() << ": " << name << ": " << value << " : " << unit;
	Write(text);
}

/** Tells whether test case is passed in result perspective*/
bool MwtsResult::IsPassed()
{
	MWTS_ENTER;
	if(!m_pInitIteration->IsPassed())
	{
		return false;
	}
	for(int i=0; i<m_listIterations.size(); i++)
	{
		if(!m_listIterations.at(i)->IsPassed())
		{
			return false;
		}
	}
	return true;
}


/**
  Writes report to result file. Uses measurement values and calculates
  mean, median, max, min and standard deviation
*/
void MwtsResult::WriteReport()
{
	QDomDocument dom("report");

	MWTS_ENTER;
	int i;
	double sum=0;
	double max=-100000;
	double min=100000;
	double value;
	int nIterations=m_listIterations.size();
	MwtsIteration* pIteration;

	QString verdict;
	if(this->IsPassed())
		verdict="PASSED";
	else
		verdict="FAILED";

	Write("Overall result : "+verdict);

	QString text;
	QTextStream(&text) << nIterations << " iterations total" ;
	Write(text);
	QString sMeasureName = "None";
	QString sMeasureUnit = "None";

	if(nIterations <= 1)
	{
		return;
	}
	QList<double> values;

	for (i = 0; i < nIterations; ++i)
	{
		pIteration=m_listIterations.at(i);
		if(pIteration->m_listMeasures.size() > 1)
		{
			MWTS_ERROR("More than one measurement values per iteration");
			continue;
		}
		if(pIteration->m_listMeasures.size() < 1)
		{
			qDebug() << "No values for iteration " << i;
			continue;
		}

		value=pIteration->m_listMeasures.at(0)->m_lfValue;
		sMeasureName = pIteration->m_listMeasures.at(0)->m_sName;
		sMeasureUnit = pIteration->m_listMeasures.at(0)->m_sUnit;
		sum+=value;
		values.append(value);
		if(value>max)
		{
			max=value;
		}
		if(value<min)
		{
			min=value;
		}
	}

	if(values.size() > 0)
	{
		MwtsStatistics stats(values);

		// for example in latency measurements smaller values are better
		// this is identified by smaller target value than fail limit.
		// -> comparison should be done accordingly

		bool bLimitExceeded=true;
		bool bBiggerIsBetter=true;
		if(m_lfFailLimit > m_lfTarget)
			bBiggerIsBetter=false;

		if(bBiggerIsBetter && stats.Median()<m_lfFailLimit)
			bLimitExceeded=false;

		if((!bBiggerIsBetter) && stats.Median()>m_lfFailLimit)
			bLimitExceeded=false;


		text.sprintf("Min: %.2lf", stats.Min());
		Write(text);
		text.sprintf("Max: %.2lf", stats.Max());
		Write(text);
		text.sprintf("Mean: %.2lf", stats.Mean());
		Write(text);
		text.sprintf("Stdev: %.2lf", stats.Stdev());
		Write(text);
		text.sprintf("Median: %.2lf", stats.Median());
		Write(text);
		text.sprintf("Target: %.2lf", m_lfTarget);
		Write(text);
		text.sprintf("Fail: %.2lf", m_lfFailLimit);
		Write(text);

		// TODO: fix this to be overall verdict
		if(!bLimitExceeded)
		{
			Write("Target value not exceeded! : CASE FAILED");
		}

		// Write XML report
		QFile report;
		report.setFileName("/var/log/tests/"+g_pTest->CaseName()+".report");
		report.open(QIODevice::WriteOnly);

		QString sValue=QString::number(stats.Median());
		QString sTarget=QString::number(m_lfTarget);
		QString sFailLimit=QString::number(m_lfFailLimit);

		QString sReport="<testreport>\n\t<measures>\n\t\t<measure name=\"" + sMeasureName +
			"\" description=\"median\" value=\"" + sValue +
			"\" unit=\"" + sMeasureUnit +
			"\" fail=\"" + sFailLimit +
			"\" target=\"" + sTarget + "\" />\n\t</measures>\n</testreport>\n";

		report.write(sReport.toLatin1());
		report.close();

	}
	MwtsMonitor::instance()->WriteResults();
	MWTS_LEAVE;
}

/**
  Writes next iteration mark
*/
void MwtsResult::NextIteration()
{
	MWTS_ENTER;
	m_nIteration++;
	m_pCurrentIteration=new MwtsIteration;
	m_listIterations.append(m_pCurrentIteration);
	QString text;
	QTextStream(&text) << "-- iteration " << m_nIteration << " --";
	Write(text);
	qDebug() << "-- iteration " << m_nIteration << " --";

	MwtsMonitor::instance()->NextIteration();
}


