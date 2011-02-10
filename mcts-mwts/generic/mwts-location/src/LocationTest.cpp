/*
 * This file is part of MCTS
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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
#include "TestDriver.h"

// 10 minutes, maximum time we wait for cold fix
#define COLD_FIX_TIMEOUT 600000
// Test 20 minutes, maximum time we wait accuracy fixes
#define ACCURACY_FIX_TIMEOUT 1200000
// 1 minute, maximum time we wait for warm fix
#define WARM_FIX_TIMEOUT 60000
// 20 seconds, maximum time we wait for hot fix
#define HOT_FIX_TIMEOUT 20000
// how many updates we wait for when measuring hot fix time
#define UPDATE_COUNT_HOT_FIX 10
// how many updates we wait for when measuring cold fix time
#define UPDATE_COUNT_COLD_FIX 10
// default how many fixes are waited in accuracy test
#define NUM_OF_FIXES 100
// default allowe radius in meters
#define ALLOWED_RADIUS 5.0
// default antenna location, is illegal, so must set in conf
#define ANTENNA_LATITUDE -91
#define ANTENNA_LONGITUDE -181
#define ANTENNA_ALTITUDE -10001
// default required procent in accurance test
#define REQUIRED_PROS 90.0




LocationTest::LocationTest()
: m_numOfFixes(NUM_OF_FIXES),
  m_allowedRadius(ALLOWED_RADIUS),
  m_antennaLatitude(ANTENNA_LATITUDE),
  m_antennaLongitude(ANTENNA_LONGITUDE),
  m_antennaAltitude(ANTENNA_ALTITUDE),
  m_requiredProsent(REQUIRED_PROS),
  m_useTestDriver(false),
  m_allow3Demulation(false),
  m_emulatedRadius(ALLOWED_RADIUS),
  m_pTestDriver(NULL)

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
        qDebug() << "LocationTest::OnInitialize connect(m_gpisLocationSource)";
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

    qDebug() << "LocationTest::OnInitialize _pTimeout=new QTimer";
    m_pTimeout=new QTimer(this);
	m_nFixCountLeft = 0; //Initialize
	m_bGetLocFix = false;
    m_bAccuracyResult = false;

// NOTE EXPERIMENTAL Test Driver part, normally set of in configuration file
    m_useTestDriver = g_pConfig->value("TestDriver/UseTestDriver", false).toBool();
    qDebug() << "LocationTest::OnInitialize TestDriver/UseTestDriver" << m_useTestDriver;
    if (m_useTestDriver)
    {
        m_emulatedRadius = g_pConfig->value("TestDriver/Allowed_radius", ALLOWED_RADIUS).toDouble();
        qDebug() << "LocationTest::OnInitialize TestDriver/Allowed_radius" << m_emulatedRadius ;

        m_antennaLatitude = g_pConfig->value("Accuracy/Antenna_latitude",ANTENNA_LATITUDE).toDouble();
        qDebug() << "LocationTest::OnInitialize Accuracy/Antenna_latitude" << m_antennaLatitude;

        m_antennaLongitude = g_pConfig->value("Accuracy/Antenna_longitude",ANTENNA_LONGITUDE).toDouble();
        qDebug() << "LocationTest::OnInitialize Accuracy/Antenna_longitude" << m_antennaLongitude;

        m_antennaAltitude = g_pConfig->value("Accuracy/Antenna_altitude", ANTENNA_ALTITUDE).toDouble();
        qDebug() << "LocationTest::OnInitialize Accuracy/Antenna_altitude" << m_antennaAltitude;

        m_allow3Demulation = g_pConfig->value("TestDriver/Allow3D", false).toBool();
        qDebug() << "LocationTest::OnInitialize TestDriver/Allow3D" << m_allow3Demulation;

        qDebug() << "LocationTest::OnInitialize do m_pTestDriver = new TestDriver";
        m_pTestDriver = new TestDriver(m_antennaLatitude,
                                       m_antennaLongitude,
                                       m_antennaAltitude,
                                       m_allow3Demulation,
                                       m_emulatedRadius);
        qDebug() << "LocationTest::OnInitialize done m_pTestDriver = new TestDriver";

#ifdef newcode2
        qDebug() << "LocationTest::OnInitialize connect(m_pTestDriver)";
        connect(m_pTestDriver,
            SIGNAL(positionUpdated(const QGeoPositionInfo)),
            this, SLOT(OnPositionUpdated(const QGeoPositionInfo)));
        connect(m_pTestDriver,
            SIGNAL(updateTimeout()),
            this, SLOT(OnPositionSourceTimeoutExpired()));
#endif
    }

    MWTS_LEAVE;
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
        //bool firstFix = m_oElapsedSinceLastFix.isNull();
	int sinceLast = m_oElapsedSinceLastFix.elapsed();
	m_nFixCountLeft--;

	QGeoCoordinate coord = info.coordinate();
	//m_listPositions.append(coord);

	//should we allow invalid timestamp and check only validity of the coordinates??
	if(info.isValid())
	{
		qDebug() << ("Position updated: ")
			 << info.coordinate().toString()
			 << ("Timestamp: ")
             << info.timestamp().toString()
             << ("Type: ")
             << coord.type();

        m_listPositions.append(coord);

        if  (coord.type() < m_CoordinateType)
        {
            m_CoordinateType = coord.type();
        }

		if(m_bGetLocFix)
		{
			m_listTimesToFix.append(fromStart);
			qDebug() << "GetLocationFix, received in:" << fromStart << "ms";
		}
                else if(firstFix)
		{
                    m_listTimesToFix.append(fromStart);
                    qDebug() << "Cold or warm fix, received in:" << fromStart << "ms";
                    //remove GPS data if that is cold fix case
                    if (m_nHotMode == MODE_COLD)
                        RemoveGPSData();
		}
                else if (m_nHotMode == MODE_COLD)
                {
                    //qDebug() << "counts " << m_listTimesToFix.count();
                    m_listTimesToFix.append(sinceLast);
                    qDebug() << "Cold or warm fix, received in:" << sinceLast << "ms";
                    RemoveGPSData();
                }
                else if (m_nHotMode == MODE_HOT)
		{
			m_listTimesToFix.append(sinceLast);
			qDebug() << "Hot fix, received in:" << sinceLast << "ms";
		}

		if( m_nFixCountLeft == 0 )
		{
                    //set m_oElapsedSinceLastFix to invalid for next iteration
                    m_oElapsedSinceLastFix = QTime();
                    m_gpisLocationSource->stopUpdates();
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

        //set that fix is not first
        firstFix = false;

        //start to measure next fix
        m_oElapsedSinceLastFix.start();
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

        firstFix = true;
	m_nFixCountLeft = 1;

        if (m_nHotMode == MODE_HOT)
		m_nFixCountLeft += UPDATE_COUNT_HOT_FIX;
        else if (m_nHotMode == MODE_COLD)
            m_nFixCountLeft += UPDATE_COUNT_COLD_FIX;


	m_oElapsedSinceLastFix = QTime(); //invalid/null
	m_oElapsedFromStart.start();
        qDebug() << "Removing GPS data to be sure that first fix is cold.";
        RemoveGPSData();
	m_pTimeout->start( COLD_FIX_TIMEOUT );
	m_gpisLocationSource->startUpdates();

	Start();

	if( m_nFixCountLeft != 0 )
	{
		g_pResult->Write("Has not received enough fix(es).");
		g_pResult->StepPassed("LocationFix", false);
		return;
	}

        QString s;
        if(m_nHotMode == MODE_HOT)
            s = "Succesfully received " + QString().setNum(1 + UPDATE_COUNT_HOT_FIX) + " hot fixes";
        else if(m_nHotMode == MODE_COLD)
            s = "Succesfully received " + QString().setNum(1 + UPDATE_COUNT_COLD_FIX) + " cold fixes";

        g_pResult->Write(s);

        //removing cold/warm fix
        if (m_nHotMode == MODE_HOT)
            m_listTimesToFix.takeFirst();

        qSort(m_listTimesToFix);

        int median = m_listTimesToFix.at( m_listTimesToFix.count() / 2 );
        if( m_listTimesToFix.count() % 2 == 0 )
        {
            median = ( median + m_listTimesToFix.at( m_listTimesToFix.count() / 2 - 1 ) ) / 2;
        }

        // calculate the average time for fixes
        double average = 0.0;
        for( int i = 0; i < m_listTimesToFix.count(); i++ )
        {
            average += m_listTimesToFix.at(i);
        }
        average /= m_listTimesToFix.count();

        QString sFixType;
        if(m_nHotMode == MODE_HOT)
            sFixType = "Hot";
        else if(m_nHotMode == MODE_COLD)
            sFixType = "Cold";

        g_pResult->Write(sFixType + " fix time (med): "+ QString().setNum(median) +" ms");
        g_pResult->Write(sFixType + " fix time (min): "+ QString().setNum(m_listTimesToFix.first()) +" ms");
        g_pResult->Write(sFixType + " fix time (max): "+ QString().setNum(m_listTimesToFix.last()) +" ms");

        g_pResult->AddMeasure(sFixType + " fix time (avg)", average, "ms");

        g_pResult->StepPassed("LocationFix", true);

        m_listTimesToFix.erase(m_listTimesToFix.begin(), m_listTimesToFix.end());
}


void LocationTest::GetLocationFix()
{
    MWTS_ENTER;

    if (!m_gpisLocationSource)
    {
        qCritical() << "No location source";
        return;
    }

    //setting that this will be first fix
    //it is important because first fix will be always cold
    firstFix = true;

    qDebug() << "Removing GPS data to be sure that first fix is cold.";
    RemoveGPSData();

    // fix will be taken once (cold) or twice (for hot fix, second will be hot)
    QString s = "Starting to measure fix time...";
    if(m_nHotMode == MODE_HOT)
    {
        qDebug() << s;
        g_pResult->Write(s);
        m_nFixCountLeft = 2;
    }
    else if(m_nHotMode == MODE_COLD)
    {
        qDebug() << s;
        g_pResult->Write(s);
        m_nFixCountLeft = 1;
    }

    m_pTimeout->start( COLD_FIX_TIMEOUT );

    m_oElapsedFromStart.start();
    m_gpisLocationSource->startUpdates();
    Start();

    if (m_nHotMode == MODE_HOT && m_listTimesToFix.count() != 2)
    {
        qCritical() << "No hot fix got";
        g_pResult->Write("No hot fix got");
        g_pResult->StepPassed("GetLocation", false);
        return;
    }
    else if (m_nHotMode == MODE_COLD && m_listTimesToFix.count() != 1)
    {
        qCritical() << "No cold fix got";
        g_pResult->Write("No cold fix got");
        g_pResult->StepPassed("GetLocation", false);
        return;
    }

    if (m_nHotMode == MODE_HOT)
        g_pResult->AddMeasure("Time To First Fix (Hot)", m_listTimesToFix.at(1), "ms");
    else if (m_nHotMode == MODE_COLD)
        g_pResult->AddMeasure("Time To First Fix (Cold)", m_listTimesToFix.at(0), "ms");

    g_pResult->StepPassed("GetLocation", true);
    m_listTimesToFix.clear();

    MWTS_LEAVE;
}

void LocationTest::TestAccuracy()
{
    MWTS_ENTER;
    m_bAccuracyResult = true;

    firstFix = true;

    if (!m_gpisLocationSource)
    {
        qCritical() << "No location source";
        return;
    }// fetch ap ssid from NetworkTest.conf

    m_numOfFixes = g_pConfig->value("Accuracy/Number_of_fixes", NUM_OF_FIXES).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Number_of_fixes" << m_numOfFixes;

    m_allowedRadius = g_pConfig->value("Accuracy/Allowed_radius", ALLOWED_RADIUS).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Allowed_radius" << m_allowedRadius ;

    m_antennaLatitude = g_pConfig->value("Accuracy/Antenna_latitude",ANTENNA_LATITUDE).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_latitude" << m_antennaLatitude;

    m_antennaLongitude = g_pConfig->value("Accuracy/Antenna_longitude",ANTENNA_LONGITUDE).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_longitude" << m_antennaLongitude;

    m_antennaAltitude = g_pConfig->value("Accuracy/Antenna_altitude", ANTENNA_ALTITUDE).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Antenna_altitude" << m_antennaAltitude;

    m_requiredProsent = g_pConfig->value("Accuracy/Required_prosent",REQUIRED_PROS).toDouble();
    qDebug() << "LocationTest::TestAccuracy Accuracy/Required_prosent" << m_requiredProsent;


    m_nFixCountLeft = m_numOfFixes;
    m_CoordinateType = QGeoCoordinate::Coordinate3D; // Hope we gets coordinates with valid latitude and longitude values,
                                                     // and also an altitude value.
                                                     // Process will change this to lover is not


    qDebug() << "LocationTest::TestAccuracy m_nFixCountLeft"  << m_nFixCountLeft;

    m_oElapsedSinceLastFix = QTime(); //invalid/null
    m_oElapsedFromStart.start();
    //m_pTimeout->start( COLD_FIX_TIMEOUT );

    if (m_useTestDriver && (m_pTestDriver != NULL))
    {
#ifdef newcode2
        m_pTestDriver->startUpdates();
#endif
    }
    else
    {
        m_gpisLocationSource->startUpdates();
    }

    qDebug() << "LocationTest::TestAccuracy call Start()";
    Start();
    qDebug() << "LocationTest::TestAccuracy Start() returned, m_nFixCountLeft" << m_nFixCountLeft;

    if( m_nFixCountLeft != 0 )
    {
        g_pResult->Write("Has not received enough fix(es).");
        g_pResult->StepPassed("LocationFix", false);
        return;
    }

    if(m_nHotMode == MODE_HOT)
    {
        qSort(m_listTimesToFix);
        int median = m_listTimesToFix.at( m_listTimesToFix.count() / 2 );
        if( m_listTimesToFix.count() % 2 == 0 )
        {
            median = ( median + m_listTimesToFix.at( m_listTimesToFix.count() / 2 - 1 ) ) / 2;
        }


        g_pResult->AddMeasure("Hot fix time (med)", median, "ms");
        g_pResult->AddMeasure("Hot fix time (min)", m_listTimesToFix.first(), "ms");
        g_pResult->AddMeasure("Hot fix time (max)", m_listTimesToFix.last(), "ms");
    }
    else // not hot
    {
        g_pResult->Write("Succesfully received cold (or warm) fix.");
        g_pResult->AddMeasure("Cold fix time", m_listTimesToFix.first(), "ms");
    }

    CalculateDistances();
    CalculateAccuracy();
    g_pResult->StepPassed("Accuracy", m_bAccuracyResult);


    MWTS_LEAVE;

}

void LocationTest::CalculateDistances()
{
	MWTS_ENTER;
	bool result = false;


    if (m_listPositions.isEmpty())
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

void LocationTest::CalculateAccuracy()
{
    MWTS_ENTER;
    int numOfAllowed = 0;
    qreal sumDist = 0.0;
    qreal resultPros = 0.0;
    double sumAntennaLatitude = 0.0;
    double sumAntennaLongitude = 0.0;
    double sumAntennaAltitude = 0.0;


    if (m_listPositions.isEmpty())
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

        QGeoCoordinate anttennapoint(m_antennaLatitude, m_antennaLongitude, m_antennaAltitude);
        for(int i = 0; i < numOfPositions; i++)
        {
            qreal dist = anttennapoint.distanceTo(m_listPositions.value(i));
            qDebug() << "dist " << dist;
            sumAntennaLatitude += m_listPositions.value(i).latitude();
            sumAntennaLongitude += m_listPositions.value(i).longitude();
            if (m_CoordinateType == QGeoCoordinate::Coordinate3D)
                sumAntennaAltitude += m_listPositions.value(i).altitude();

            if (dist <= m_allowedRadius)
            {
                numOfAllowed++;
            }
            sumDist += dist;
        }

        resultPros =((qreal) 100.0) * ((qreal) numOfAllowed)/((qreal) numOfPositions);

        m_bAccuracyResult = (resultPros >= m_allowedRadius);
        g_pResult->Write("");
        g_pResult->Write("Total count of positions: "  + QString::number(numOfPositions));
        g_pResult->Write("Percent in allowed distance: "  + QString::number(resultPros));
        g_pResult->Write("Mean distance of antenna of all fixes: "  + QString::number(sumDist/(qreal) numOfPositions));
        g_pResult->Write("Mean latitude of antenna of all fixes: "  + QString::number(sumAntennaLatitude/(double) numOfPositions));
        g_pResult->Write("Mean longitude of antenna of all fixes: "  + QString::number(sumAntennaLongitude/(double) numOfPositions));
        if (m_CoordinateType == QGeoCoordinate::Coordinate3D)
            g_pResult->Write("Mean altitude of antenna of all fixes: "  + QString::number(sumAntennaAltitude/(double) numOfPositions));
        else
            g_pResult->Write("2D coordinates got, so mean altitude of antenna of all fixes is not available");
        g_pResult->Write("");

    }
    else
    {
        qDebug() << "At least one position is needed";
        g_pResult->Write("Calculate Accuracy: At least one positions is needed");
    }
    MWTS_LEAVE;
}

void LocationTest::GetCoordinates(int count) {

    if (!m_gpisLocationSource)
    {
        qCritical() << "No location source";
        return;
    }

    m_nHotMode = MODE_HOT;

    m_bGetLocFix = true;

    m_nFixCountLeft = count;

    QString s = "Starting to measure fix time...";
    qDebug() << s;
    g_pResult->Write(s);

    m_oElapsedFromStart.start();
    m_gpisLocationSource->startUpdates();
    Start();

    if (m_nFixCountLeft == 0 && m_listPositions.count() == 10)
    {
        g_pResult->Write("Received " + QString::number(count) + " coordinates:");
        QList<QGeoCoordinate>::iterator it;

        for (it = m_listPositions.begin(); it != m_listPositions.end(); ++it)
        {
            g_pResult->Write(it->toString());
        }
        g_pResult->StepPassed(__FUNCTION__, true);
    }
    else
    {
        QString s = "Got not enough coordinates (" + QString::number(m_listPositions.count()) + ")";
        qCritical() << s;
        g_pResult->Write(s);
        g_pResult->StepPassed(__FUNCTION__, false);
        return;
    }



}

void LocationTest::RemoveGPSData() const {
    MWTS_ENTER;
    qDebug() << "Removing GPS Data -- to implement -- ";
    MWTS_LEAVE;
}
