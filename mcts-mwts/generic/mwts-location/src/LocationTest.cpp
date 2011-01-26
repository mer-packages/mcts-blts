/*
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


#include <qgeocoordinate.h>
#include <MwtsCommon>
#include "stable.h"
#include "LocationTest.h"

// 10 minutes, maximum time we wait for cold fix
#define COLD_FIX_TIMEOUT 600000
// 1 minute, maximum time we wait for warm fix
#define WARM_FIX_TIMEOUT 60000
// 20 seconds, maximum time we wait for hot fix
#define HOT_FIX_TIMEOUT 20000
// how many updates we wait for when measuring hot fix time
#define UPDATE_COUNT_HOT_FIX 30

LocationTest::LocationTest()
{

	MWTS_ENTER;
}

LocationTest::~LocationTest()
{
	MWTS_ENTER;
}


void LocationTest::OnInitialize()
{
	MWTS_ENTER;

	m_nHotMode=0;
	m_nPositioningMethod=0;

	m_gpisLocationSource = QGeoPositionInfoSource::createDefaultSource(this);
	if (m_gpisLocationSource)
	{
		connect(m_gpisLocationSource,
			SIGNAL(positionUpdated(const QGeoPositionInfo)),
			this, SLOT(OnPositionUpdated(const QGeoPositionInfo)));
		connect(m_gpisLocationSource,
			SIGNAL(updateTimeout()),
			this, SLOT(OnPositionSourceTimeoutExpired()));
	}
	else
	{
		qCritical() << "Default source not found";
		g_pResult->StepPassed("Create Default Source", false);
		MWTS_LEAVE;
		return;
	}
	m_pTimeout=new QTimer(this);
	m_nFixCountLeft = 0; //Initialize
	m_bGetLocFix = false;

}

void LocationTest::OnUninitialize()
{
	MWTS_ENTER;
	//signals are disconnected when objects are destroyed
	//child objects are destroyed when this object is destroyed
	// ->  nothing to do
}

void LocationTest::SetPositioningMethod(int method)
{
	m_nPositioningMethod=method;
	if (m_nPositioningMethod==METHOD_ALL)
	{
		m_gpisLocationSource->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);
	}
	else if(m_nPositioningMethod==METHOD_SATELLITE)
	{
		m_gpisLocationSource->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);
	}
	else if(m_nPositioningMethod==METHOD_NON_SATELLITE)
	{
		m_gpisLocationSource->setPreferredPositioningMethods(QGeoPositionInfoSource::NonSatellitePositioningMethods);
	}
	else
	{
		qCritical() << "Invalid positioning method selected: " << m_nPositioningMethod;
		return;
	}
}

void LocationTest::SetHotMode(int mode)
{
	m_nHotMode=mode;
}

void LocationTest::OnPositionUpdated(const QGeoPositionInfo &info)
{
	int fromStart = m_oElapsedFromStart.elapsed();
	bool firstFix = m_oElapsedSinceLastFix.isNull();
	int sinceLast = m_oElapsedSinceLastFix.elapsed();
	m_oElapsedSinceLastFix.start();
	m_nFixCountLeft--;

	QGeoCoordinate coord = info.coordinate();
	m_listPositions.append(coord);

	//should we allow invalid timestamp and check only validity of the coordinates??
	if(info.isValid())
	{
		qDebug() << ("Position updated: ")
			 << info.coordinate().toString()
			 << ("Timestamp: ")
			 << info.timestamp().toString();

		if(m_bGetLocFix)
		{
			m_listTimesToFix.append(fromStart);
			qDebug() << "GetLocationFix, received in:" << fromStart << "ms";
		}
		else if( firstFix )
		{
			m_listTimesToFix.append(fromStart);
			qDebug() << "Cold or warm fix, received in:" << fromStart << "ms";
		}
		else
		{
			m_listTimesToFix.append(sinceLast);
			qDebug() << "Hot fix, received in:" << sinceLast << "ms";
		}

		if( m_nFixCountLeft == 0 )
		{
			Stop();
		}
		else
		{
			m_pTimeout->start( HOT_FIX_TIMEOUT );
		}
	}
	else
	{
		qWarning() << "Position update not valid, time: " << fromStart;
		Stop();
	}
}

void LocationTest::OnTimeoutExpired()
{
	MWTS_ENTER;
	QString sinceLastString=( m_oElapsedSinceLastFix.isNull() ?
		              "" :
		              ", " + QString().setNum(m_oElapsedSinceLastFix.elapsed()) + " ms since last fix" );
	qCritical() << "Timeout: position update was not received in time: "
		<< m_oElapsedFromStart.elapsed()
		<< "ms from start"
		<< sinceLastString;
	MWTS_LEAVE;
	Stop();
}

void LocationTest::OnPositionSourceTimeoutExpired()
{
	MWTS_ENTER;
	QString sinceLastString=( m_oElapsedSinceLastFix.isNull() ?
		"" :
		", " + QString().setNum(m_oElapsedSinceLastFix.elapsed()) + " ms since last fix" );
	qWarning() << "Position source signalled TIMEOUT @"
		<< m_oElapsedFromStart.elapsed()
		<< "ms from start"
		<< sinceLastString;
	MWTS_LEAVE;
}

void LocationTest::TestLocationFix()
{
	MWTS_ENTER;

	if (!m_gpisLocationSource)
	{
		qCritical() << "No location source";
		return;
	}

	m_nFixCountLeft = 1;

	if(m_nHotMode == MODE_HOT)
		m_nFixCountLeft += UPDATE_COUNT_HOT_FIX;

	m_oElapsedSinceLastFix = QTime(); //invalid/null
	m_oElapsedFromStart.start();
	m_pTimeout->start( COLD_FIX_TIMEOUT );
	m_gpisLocationSource->startUpdates();

	Start();

	if( m_nFixCountLeft != 0 )
	{
		g_pResult->Write("Has not received enough fix(es).");
		g_pResult->StepPassed("LocationFix", false);
		return;
	}

	if(m_nHotMode == MODE_HOT)
	{
		QString s= "Succesfully received " + QString().setNum(1 + UPDATE_COUNT_HOT_FIX) + " fixes";
		g_pResult->Write(s);
		//removing cold/warm fix
		m_listTimesToFix.takeFirst();
		qSort(m_listTimesToFix);
		int median = m_listTimesToFix.at( m_listTimesToFix.count() / 2 );
		if( m_listTimesToFix.count() % 2 == 0 )
		{
			median = ( median + m_listTimesToFix.at( m_listTimesToFix.count() / 2 - 1 ) ) / 2;
		}

		g_pResult->AddMeasure("Hot fix time (med)", median, "ms");
		g_pResult->AddMeasure("Hot fix time (min)", m_listTimesToFix.first(), "ms");
		g_pResult->AddMeasure("Hot fix time (max)", m_listTimesToFix.last(), "ms");
		g_pResult->StepPassed("LocationFix", true);
	}
	else // not hot
	{
		g_pResult->Write("Succesfully received cold (or warm) fix.");
		g_pResult->AddMeasure("Cold fix time", m_listTimesToFix.first(), "ms");
		g_pResult->StepPassed("LocationFix", true);
	}

}


void LocationTest::GetLocationFix()
{
	MWTS_ENTER;
	if (!m_gpisLocationSource)
	{
		qCritical() << "No location source";
		return;
	}


	if (m_nFixCountLeft == 0) //First time, select positioning method only once
	{

		m_bGetLocFix = true;

		if(m_nHotMode == MODE_HOT)
		{
			m_nFixCountLeft = 1;
			qDebug() << "HOT mode selected, creating one fix and ignoring results completely";
			m_pTimeout->start( COLD_FIX_TIMEOUT );
			m_oElapsedFromStart.start();
			m_gpisLocationSource->startUpdates();
			Start();
			if(!m_listTimesToFix.isEmpty())
			{
				qDebug() << "First fix was done (Warm or Cold) and result is ignored: " << m_listTimesToFix.first() << "ms";
				m_listTimesToFix.clear();
				m_listPositions.clear();
			}
		}
		else
		{
			qDebug() << "COLD mode enabled, first result is either COLD or WARM fix";
		}

	}

	m_nFixCountLeft = 1; //only one fix
	m_pTimeout->start( COLD_FIX_TIMEOUT );

	m_oElapsedFromStart.start();
	m_gpisLocationSource->startUpdates();
	Start();

	if(!m_listTimesToFix.isEmpty())
	{
		//Take first result and write it
		g_pResult->AddMeasure("Fix got in time", m_listTimesToFix.first(), "ms");
		g_pResult->StepPassed("GetLocation", true);

		m_listTimesToFix.clear();
	}
	else
	{
		//List was empty, no fix time got
		g_pResult->Write("No fix got");
		g_pResult->StepPassed("GetLocation", false);
	}

	//For iterative usage, reset counter to one
	m_nFixCountLeft = 1;


	MWTS_LEAVE;

}

void LocationTest::TestAccuracy()
{
    MWTS_ENTER;

    if (!m_gpisLocationSource)
    {
        qCritical() << "No location source";
        return;
    }// fetch ap ssid from NetworkTest.conf

    const double numOfFixes = g_pConfig->value("Accuracy/Number_of_fixes").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Number_of_fixes" << numOfFixes;

    const double allowedRadius = g_pConfig->value("Accuracy/Allowed_radius").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Allowed_radius" << allowedRadius ;

    const double antennaX = g_pConfig->value("Accuracy/Antenna_X").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_X" << antennaX;

    const double antennaY = g_pConfig->value("Accuracy/Antenna_Y").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_Y" << antennaY;

    const double antennaZ = g_pConfig->value("Accuracy/Antenna_Z").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_Z" << antennaZ;

    const double requiredProsent = g_pConfig->value("Accuracy/Required_prosent").toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Required_prosent" << requiredProsent;

    MWTS_LEAVE;

}


void LocationTest::CalculateDistances()
{
	MWTS_ENTER;
	bool result = false;


	if(!m_listPositions.isEmpty())
	{
		qDebug() << "Positions list was empty, nothing to calculate";
		g_pResult->Write("Calculate Distances: Positions list was empty, nothing to calculate");
		return;
	}

	//Not empty, total count is:
	qDebug() << "Total count of positions:" << m_listPositions.size();

	if(m_listPositions.size() > 1)
	{
		int numOfPositions = m_listPositions.size();

		//Calculating sum of distances to each position and try to determinate
		//which is the middle point

		QGeoCoordinate middlepoint;
		qreal middleDist = -1;
		for(int i = 0; i < numOfPositions; i++)
		{
			qreal calcDist = 0;
			for(int y = 1; y < numOfPositions; y++)
			{
				calcDist += m_listPositions.value(i).distanceTo(m_listPositions.value(y));
			}

			if(middleDist > calcDist || middleDist == -1) //Coordinate in use is more closer to center point
			{
				middleDist = calcDist;
				middlepoint = m_listPositions.value(i);
				calcDist = 0;
			}
			else //previous coordinate is still the center point
			{
				calcDist = 0;
			}
		}

		qDebug() << "Middlepoint is: " << middlepoint.toString() <<
			"and it avarage distance to others is" << (middleDist / (numOfPositions -1 ));
		QList<qreal>  distances;
		for (int i = 0; i < numOfPositions; i++)
		{
			qDebug() << middlepoint.toString() << "distance to " << m_listPositions.value(i).toString()
				<< "is" << middlepoint.distanceTo(m_listPositions.value(i));
			distances.append(middlepoint.distanceTo(m_listPositions.value(i)));
		}

		qSort(distances);
		distances.removeFirst();

		qreal median;
		if( distances.size() % 2 == 0 )
		{
			median = (distances.at(distances.size() / 2) + distances.at((distances.size() / 2) + 1)) / 2;
		}
		else
		{
			median = distances.at(distances.size() / 2);
		}

		g_pResult->Write("");
		g_pResult->Write("Most center point is (Altitude ignored): " + middlepoint.toString());
		g_pResult->Write("Median distance to other points "+ QString().setNum(median) +" meters");
		g_pResult->Write("Distance to nearest point "+ QString().setNum(distances.first()) +" meters");
		g_pResult->Write("Distance to most faraway point "+ QString().setNum(distances.last()) +" meters");
		g_pResult->Write("");

		result = true;

	}
	else
	{
		qDebug() << "At least two positions are needed";
		g_pResult->Write("Calculate Distances: At least two positions are needed");
	}

	MWTS_LEAVE;
}
