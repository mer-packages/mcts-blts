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
	m_lfFailLimit=0.0;
	m_lfTarget=0.0;
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


void MwtsResult::SetResultFilter(QString filter)
{
	m_sResultFilter=filter;
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
	// if result filter is set and the name does not match it, don'd do anything
	if(!m_sResultFilter.isEmpty())
	{
		if(!name.contains(m_sResultFilter, Qt::CaseInsensitive))
		{
			return;
		}
	} 

	m_pCurrentIteration->AddMeasure(new MwtsMeasure(name, value, unit));
	QString text;
	QTextStream(&text) << "- " << g_pTest->Name() << ": " << name << ": " << value << " : " << unit;
	Write(text);
}


void MwtsResult::WriteSeriesFile(QString name, QString text)
{
    //let's avoid fullfilling the log with this call
    //MWTS_ENTER;
	QFile file;
	file.setFileName("/var/log/tests/"+g_pTest->CaseName()+"."+name+".csv");
	file.open(QIODevice::Append);
	file.write(text.toLatin1()+"\n");
}

void MwtsResult::StartSeriesMeasure(QString name, QString unit, double lfTarget, double lfFailLimit)
{
	QString str=name + QString(";") + unit + QString(";");
	if(lfTarget!=0)
		str+= QString::number(lfTarget) + QString(";");
	if(lfFailLimit!=0)
		str+= QString::number(lfFailLimit) + QString(";");
	WriteSeriesFile(name, str);	
	
}

void MwtsResult::AddSeriesMeasure(QString name, double value)
{
	QString str;
	QDateTime timestamp = QDateTime::currentDateTime();
	QString timestampStr=timestamp.toString("yyyy-mm-dd\thh:mm:ss");
	str=timestampStr+QString(";")+QString::number(value);
	WriteSeriesFile(name, str);	
	
}


/** Tells whether test case is passed in result perspective*/
bool MwtsResult::IsPassed()
{	
	MWTS_ENTER;
	if(!m_pInitIteration->IsPassed())
	{
		return false;
	}

	QList<double> values;
	MwtsIteration* pIteration;
	for(int i=0; i<m_listIterations.size(); i++)
	{
		pIteration=m_listIterations.at(i);
		if(!pIteration->IsPassed())
		{
			return false;
		}
		if(!m_listIterations.at(i)->IsPassed())
		{
			return false;
		}

		if(pIteration->m_listMeasures.size() > 1)
		{
			MWTS_ERROR("More than one measurement values per iteration");
			return FALSE;
		}
		if(pIteration->m_listMeasures.size() < 1)
		{
			qDebug() << "No values for iteration " << i;
			continue;
		}
		values.append(pIteration->m_listMeasures.at(0)->m_lfValue);
	}

	if (!values.isEmpty())
	{
		MwtsStatistics stats (values);
		bool ret = IsLimitExceeded(stats);
		return ret;
	}

	return true;
}


/**
  Writes report to result file. Uses measurement values and calculates
  mean, median, max, min and standard deviation
*/
void MwtsResult::WriteReport()
{

	MWTS_ENTER;
	int i;
	double sum=0;
	double max=-100000;
	double min=100000;
	double value;
	MwtsIteration* pIteration;
	int nIterations=m_listIterations.size();

	if(nIterations==0)
	{
		m_listIterations.append(m_pInitIteration);
		nIterations++;
		// add the init iteration in report, if there are no real iterations
	}

	QString text;
	Write(text);
	QString sMeasureName = "None";
	QString sMeasureUnit = "None";

	if(nIterations < 1)
	{
		return;
	}

	QList<double> values;
	int nPassedIterations=0;

	for (i = 0; i < nIterations; ++i)
	{
		pIteration=m_listIterations.at(i);
		if(pIteration->IsPassed())
		{
			nPassedIterations++;
		}
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
	QTextStream(&text) << nIterations << " iterations total" ;
	QTextStream(&text) << nPassedIterations << " iterations passed" ;
	QTextStream(&text) << (double)nPassedIterations/(double)nIterations*100.0 << "% pass rate";

	QString verdict="FAILED";

	if (!values.isEmpty())
	{
		MwtsStatistics stats (values);
		Write(QString("Min: %1 \nMax: %2 \nMean: %3 \nStdev: %4 \nMedian: %5 \nTarget: %6 \nFail: %7")
			.arg(stats.Min(), 0, 'f', 2).arg(stats.Max(), 0, 'f', 2)
			.arg(stats.Mean(), 0, 'f', 2).arg(stats.Stdev(), 0, 'f', 2)
			.arg(stats.Median(), 0, 'f', 2).arg(m_lfTarget, 0, 'f', 2)
			.arg(m_lfFailLimit, 0, 'f', 2));

		// TODO: fix this to be overall verdict
		if(!IsLimitExceeded(stats))
		{
			Write("Target value not exceeded! : CASE FAILED");
		}

		QString sValue=QString::number(stats.Median());
		QString sTarget=QString::number(m_lfTarget);
		QString sFailLimit=QString::number(m_lfFailLimit);

		WriteXmlReport(sMeasureName, sValue, sMeasureUnit, sFailLimit, sTarget);
		WriteCsvReport(sMeasureName, sValue, sMeasureUnit, sFailLimit, sTarget);
	}

	if(this->IsPassed())
	{
		verdict="PASSED";
	}
	Write("Overall result : " + verdict);
	MwtsMonitor::instance()->WriteResults();
	MWTS_LEAVE;

}

bool MwtsResult::IsLimitExceeded(MwtsStatistics &stats)
{
	MWTS_ENTER;
	bool bLimitExceeded=true;
	bool bBiggerIsBetter=true;

	// for example in latency measurements smaller values are better
	// this is identified by smaller target value than fail limit.
	// -> comparison should be done accordingly

	if(m_lfFailLimit > m_lfTarget)
		bBiggerIsBetter=false;
	if(bBiggerIsBetter && stats.Median()<m_lfFailLimit)
		bLimitExceeded=false;

	if((!bBiggerIsBetter) && stats.Median()>m_lfFailLimit)
		bLimitExceeded=false;

	return bLimitExceeded;
}

void MwtsResult::WriteXmlReport(QString sMeasureName, QString sValue,
		QString sMeasureUnit, QString sFailLimit, QString sTarget)
{
	QFile report;
	report.setFileName("/var/log/tests/"+g_pTest->CaseName()+".xmlreport");
	report.open(QIODevice::WriteOnly);

	QString sReport="<testreport>\n\t<measures>\n\t\t<measure name=\"" + sMeasureName +
		"\" description=\"median\" value=\"" + sValue +
		"\" unit=\"" + sMeasureUnit +
		"\" fail=\"" + sFailLimit +
		"\" target=\"" + sTarget + "\" />\n\t</measures>\n</testreport>\n";

	report.write(sReport.toLatin1());
	report.close();
}

void MwtsResult::WriteCsvReport(QString sMeasureName, QString sValue,
		QString sMeasureUnit, QString sFailLimit, QString sTarget)
{
	QFile report;
	report.setFileName("/var/log/tests/"+g_pTest->CaseName()+".csvreport");
	report.open(QIODevice::WriteOnly);

	QString sReport = sMeasureName + ";" + sValue + ";" + sMeasureUnit + ";";

	if (m_lfTarget != 0.00 and m_lfFailLimit != 0.00)
	{
 		sReport += sTarget + ";" + sFailLimit + ";";
	}
	sReport += "\n";
	report.write(sReport.toLatin1());
	report.close();
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



