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

class LocationTest : public MwtsTest
{
	Q_OBJECT
public:
	LocationTest();
	virtual ~LocationTest();

	void OnInitialize();
	void OnUninitialize();

	void SetPositioningMethod(int method);
	void SetHotMode(int mode);

    void TestLocationFix();
	void GetLocationFix();
    void TestAccuracy();

	void CalculateDistances();
	
private:
	QGeoPositionInfoSource *m_gpisLocationSource;
	QTimer* m_pTimeout;
	QTime m_oElapsedFromStart;
	QTime m_oElapsedSinceLastFix;

	int m_nHotMode;
	int m_nPositioningMethod;
	QGeoPositionInfoSource::PositioningMethod m_PositioningMethod;
	int m_nFixCountLeft;
	bool m_bGetLocFix;
	QList<int> m_listTimesToFix;
	QList<QGeoCoordinate> m_listPositions;

    int m_numOfFixes;
    double m_allowedRadius;

    // antenna location
    double m_antennaX;
    double m_antennaY;
    double m_antennaZ;

    double m_requiredProsent;



private slots:
	void OnPositionUpdated(const QGeoPositionInfo &info);
	void OnTimeoutExpired();
	void OnPositionSourceTimeoutExpired();
};

#endif //#ifndef _INCLUDED_LOCATION_TEST_H

