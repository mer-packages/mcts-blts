/* functions.cpp --
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

#include "interface.h"
#include <QString>
#include <QDebug>
#include <QMainWindow>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include "MwtsCommon"

const char *module_date = __DATE__;
const char *module_time = __TIME__;


class CommonSelfTest: public MwtsTest
{
	virtual void OnInitialize()
	{
		qDebug() << "Initialization of CommonSelfTest";
	}

	virtual void OnUninitialize()
	{
		qDebug() << "Uninitialization of CommonSelfTest";
	}
public:
};

CommonSelfTest Test;


LOCAL int RandomMeasurement(MinItemParser * item)
{
	MWTS_ENTER;
	double value=60+rand()*40.0/(double)RAND_MAX;

	g_pResult->AddMeasure("RandomMeasurement", value, "amount");

	return 0;
}


LOCAL int MaybePass(MinItemParser * item)
{
	MWTS_ENTER;

	bool bPassed=false;
	if (rand() > RAND_MAX/2)
		bPassed=true;

	g_pResult->StepPassed("MaybePass", bPassed);

	return 0;
}

LOCAL int ConfigTest(MinItemParser * item)
{
	MWTS_ENTER;
	QStringList keys = g_pConfig->allKeys();
	foreach (QString key, keys)
	{
		g_pResult->Write(key + QString(" = ") + g_pConfig->value(key).toString());
	}
	return 0;
}


LOCAL int SeriesMeasureTest(MinItemParser * item)
{
	MWTS_ENTER;	

	g_pResult->StartSeriesMeasure("throughput", "kb/s", g_pResult->m_lfTarget, g_pResult->m_lfFailLimit);

	for(int i=0; i<100; i++)
	{
		usleep(30000);
		g_pResult->AddSeriesMeasure("throughput", (double) i);
	}

	return 0;
}


/**
  Declares Min scripter functions
*/
int ts_get_test_cases (DLList ** list)
{

	MwtsMin::DeclareFunctions(list);

	ENTRYTC(*list, "RandomMeasurement", RandomMeasurement);
	ENTRYTC(*list, "MaybePass", MaybePass);
	ENTRYTC(*list, "ConfigTest", ConfigTest);
	ENTRYTC(*list, "SeriesMeasureTest", SeriesMeasureTest);

	return ENOERR;
}
