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

#include <QDebug>
#include <qgeocoordinate.h>
#include <MwtsCommon>
#include "stable.h"
#include "TestDriver.h"



TestDriver::TestDriver(double a_antennaLatitude,
                       double a_antennaLongitude,
                       double a_antennaAltitude,
                       bool a_allow3Demulation,
                       qreal a_emulatedRadius)
    : QObject(this)
#ifdef newcode2
      m_pTestworker(NULL)
#endif
{
#if newcode
    //    qDebug() << "TestDriver::TestDriver m_pTestworker = new Testworker";
    m_pTestworker = new Testworker(a_antennaLatitude,
                                   a_antennaLongitude,
                                   a_antennaAltitude,
                                   a_allow3Demulation,
                                   a_emulatedRadius);
    qDebug() << "TestDriver::TestDriver connect";
    connect(m_pTestworker,
        SIGNAL(positionUpdated(const QGeoPositionInfo)),
        this, SLOT(OnPositionUpdated(const QGeoPositionInfo)));
#endif
//    qDebug() << "TestDriver::TestDriver end";
}

TestDriver::~TestDriver()
{
#ifdef newcode2
    qDebug() << "TestDriver::~TestDriver delete m_pTestworker";
    if (m_pTestworker)
    {
        delete m_pTestworker;
        m_pTestworker = NULL;
    }
    qDebug() << "TestDriver::~TestDriver end";
#endif
}

#ifdef newcode2
void TestDriver::startUpdates()
{
    qDebug() << "TestDriver::startUpdates m_pTestworker->start()";
    if (m_pTestworker)
        m_pTestworker->start();
    qDebug() << "TestDriver::startUpdates end";
}

void TestDriver::stopUpdates()
{
    qDebug() << "TestDriver::stopUpdates";
    qDebug() << "TestDriver::stopUpdates end";
}

void TestDriver::requestUpdate ( int timeout/* = 0 */)
{
    qDebug() << "TestDriver::requestUpdate";
    qDebug() << "TestDriver::requestUpdate end";
}


void TestDriver::OnPositionUpdated(const QGeoPositionInfo &info)
{
    qDebug() << "TestDriver::OnPositionUpdate emit positionUpdated(info)";
    emit positionUpdated(info);
    qDebug() << "TestDriver::OnPositionUpdate end";
}

#ifdef newcode
QGeoPositionInfo TestDriver::lastKnownPosition(bool fromSatellitePositioningMethodsOnly/* = false*/ ) const
{
    qDebug() << "TestDriver::lastKnownPosition return m_pTestworker->lastKnownPosition(fromSatellitePositioningMethodsOnly)";
    if (m_pTestworker)
        return m_pTestworker->lastKnownPosition(fromSatellitePositioningMethodsOnly);
}
#endif

int TestDriver::minimumUpdateInterval() const
{
    qDebug() << "TestDriver::minimumUpdateInterval return 1";
    return 1;
}

QGeoPositionInfoSource::PositioningMethods TestDriver::supportedPositioningMethods() const
{
    qDebug() << "TestDriver::supportedPositioningMethods return methods";
    QGeoPositionInfoSource::PositioningMethods methods;
    return methods;
    //return QFlags<QGeoPositionInfoSource::AllPositioningMethods>;
}





Testworker::Testworker(double a_antennaLatitude,
                       double a_antennaLongitude,
                       double a_antennaAltitude,
                       bool m_allow3Demulation,
                       qreal m_emulatedRadius)
: QThread(this),
  m_antennaLatitude(a_antennaLatitude),
  m_antennaLongitude(a_antennaLongitude),
  m_antennaAltitude(a_antennaAltitude),
  m_allow3Demulation(m_allow3Demulation),
  m_emulatedRadius(m_emulatedRadius),
  m_pQGeoPositionInfo(NULL)

{
    qDebug() << "Testworker::Testworker";
}

Testworker::~Testworker()
{
    qDebug() << "Testworker::~Testworker";
}

void Testworker::run()
{
    QGeoCoordinate coord( m_antennaLatitude, m_antennaLongitude, m_antennaAltitude );
    QGeoPositionInfo posinfo(coord,QDateTime::currentDateTime());
    qDebug() << "Testworker::run";
    while (true)
    {
        // TODO make variation defined in conf file
        qDebug() << "Testworker::run emit positionUpdated(m_QGeoPositionInfo)";
        posinfo.setTimestamp ( QDateTime::currentDateTime() );
        emit positionUpdated(posinfo);
        msleep(100);
    }
}

#ifdef newcode
QGeoPositionInfo Testworker::lastKnownPosition(bool fromSatellitePositioningMethodsOnly/* = false*/ ) const
{
    qDebug() << "Testworker::lastKnownPosition return m_QGeoPositionInfo";
    return m_QGeoPositionInfo;
}
#endif

#endif

