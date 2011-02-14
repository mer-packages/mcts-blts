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


#ifndef _INCLUDED_LOCATION_TEST_H
#define _INCLUDED_LOCATION_TEST_H

#include <mwtstest.h>
#include <qgeopositioninfo.h>
#include <qgeopositioninfosource.h>
#include <QList>

QTM_USE_NAMESPACE

#define MODE_HOT 0
#define MODE_COLD 1

#define METHOD_ALL 0
#define METHOD_SATELLITE  1
#define METHOD_NON_SATELLITE  2

class TestDriver;

class LocationTest : public MwtsTest
{
	Q_OBJECT
public:
	LocationTest();
	virtual ~LocationTest();

	void OnInitialize();
	void OnUninitialize();

        /**
         *  Sets positioning method
         *  @param mode
         *  HOT_MODE - for hot fixes,
         *  COLD_MODE - for cold fixes
         */
	void SetPositioningMethod(int method);
        /**
         *  Sets Fixing mode
         *  @param method
         *  METHOD_ALL - all methods, most probably A-GPS will be available then
         *  METHOD_SATELLITE - info only from satellites
         *  METHOD_NON_SATELLITE - info only from networks
         */
	void SetHotMode(int mode);

        /**
         *  Performs location fix. It does selected fix (hot/cold) several times
         *  and save to list. At the end of the test result times are calculated
         *  and added as a measure.
         */
	void TestLocationFix();

        /**
         *  Gets only one fix, either cold or hot and add it as a measure.
         */
	void GetLocationFix();
    void TestAccuracy();

	void CalculateDistances();

        /**
         *  This should removes GPS data, at least ephemerises
         *  which is needed for cold fix
         *  Good idea could be to specify in config file, which files/directories to erase/clear.
         */
        void RemoveGPSData() const;
	
private:
    void CalculateAccuracy();
        // object for getting position info
	QGeoPositionInfoSource *m_gpisLocationSource;
	QTimer* m_pTimeout;
        // elapsed time from beginning of latency measure
	QTime m_oElapsedFromStart;
        // elapsed time from last got fix
	QTime m_oElapsedSinceLastFix;
        //indicates if fix is done for the first time
        bool firstFix;

	int m_nHotMode;
	int m_nPositioningMethod;
	QGeoPositionInfoSource::PositioningMethod m_PositioningMethod;
    QGeoCoordinate::CoordinateType m_CoordinateType;
	int m_nFixCountLeft;
	bool m_bGetLocFix;
    bool m_bAccuracyResult;
	QList<int> m_listTimesToFix;
	QList<QGeoCoordinate> m_listPositions;

    int m_numOfFixes;
    qreal m_allowedRadius;

    // antenna location
    double m_antennaLatitude;
    double m_antennaLongitude;
    double m_antennaAltitude;

    double m_requiredProsent;

    // test driver/(A)GPSemulation
    TestDriver *m_pTestDriver;

    bool m_useTestDriver;
    bool m_allow3Demulation;
    qreal m_emulatedRadius;


private slots:
	void OnPositionUpdated(const QGeoPositionInfo &info);
	void OnTimeoutExpired();
	void OnPositionSourceTimeoutExpired();
};

#endif //#ifndef _INCLUDED_LOCATION_TEST_H

