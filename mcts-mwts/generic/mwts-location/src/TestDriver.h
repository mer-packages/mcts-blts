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


#ifndef _INCLUDED_TEST_DRIVER_H
#define _INCLUDED_TEST_DRIVER_H

#include <LocationTest.h>
#include <qgeopositioninfo.h>
#include <qgeopositioninfosource.h>
#include <QList>
#include <QThread>

//QTM_USE_NAMESPACE

#define MODE_HOT 0
#define MODE_COLD 1

#define METHOD_ALL 0
#define METHOD_SATELLITE  1
#define METHOD_NON_SATELLITE  2

//class Testworker;

// we promise that our tesdriver implements QGeoPositionInfoSource, allways, even it chanhes hws
// NOPE, static createDefaultSource is only method to create instance and
// we can't inherit QGeoPositionInfoSource, just fake


class TestDriver : public QObject // QGeoPositionInfoSource
{
    Q_OBJECT
public:
    TestDriver(double a_antennaLatitude,
               double a_antennaLongitude,
               double a_antennaAltitude,
               bool a_allow3Demulation,
               qreal a_emulatedRadius);
    virtual ~TestDriver();

#ifdef newcode2
    void startUpdates();
    void stopUpdates();

#ifdef newcode
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false ) const;
#endif
    int minimumUpdateInterval() const;
    QGeoPositionInfoSource::PositioningMethods supportedPositioningMethods()  const;

signals:
   void positionUpdated(const QGeoPositionInfo);
   void positionSourceTimeoutExpired();

private slots:
    void OnPositionUpdated(const QGeoPositionInfo &info);
    void requestUpdate ( int timeout = 0 );
private:

    Testworker* m_pTestworker;
};

class Testworker : public QThread
{
	Q_OBJECT
public:
    Testworker(double a_antennaLatitude,
               double a_antennaLongitude,
               double a_antennaAltitude,
               bool a_allow3Demulation,
               qreal a_emulatedRadius);
    virtual ~Testworker();

    void run ();

#ifdef newcode
    QGeoPositionInfo lastKnownPosition(bool fromSatellitePositioningMethodsOnly = false ) const;
#endif

 signals:
    void positionUpdated(const QGeoPositionInfo);
    void positionSourceTimeoutExpired();


private:

    // antenna location
    double m_antennaLatitude;
    double m_antennaLongitude;
    double m_antennaAltitude;


    // test driver/(A)GPSemulation
    bool m_allow3Demulation;
    qreal m_emulatedRadius;

    QGeoPositionInfo* m_pQGeoPositionInfo;
#endif

private slots:
};

#endif //#ifndef _INCLUDED_TEST_DRIVER_H

