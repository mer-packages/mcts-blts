/****************************************************************************
**
 #
 # Copyright (C) 2010 Intel Corporation.
 # This program is free software; you can redistribute it and/or modify it
 # under the terms and conditions of the GNU General Public License,
 # version 2, as published by the Free Software Foundation.
 #
 # This program is distributed in the hope it will be useful, but WITHOUT
 # ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 # FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 # for more details.
 #
 # You should have received a copy of the GNU General Public License along with
 # this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 # Place - Suite 330, Boston, MA 02111-1307 USA.

 # Authors:  Fu, Yi  <yi.fu@intel.com>
**
****************************************************************************/

#include <QtCore>
#include <stdio.h>
#include <QTextStream>
#include <QAccelerometer>
#include <QAmbientLightSensor>
#include <QCompass>
#include <QMagnetometer>
#include <QOrientationSensor>
#include <QProximitySensor>
#include <QRotationSensor>
#include <QTapSensor>
#include <QGyroscope>
#include <QSensorReading>

#include "accel.h"

QTM_USE_NAMESPACE

enum casename {accelerometer,als,compass,magnetometer,orientation,proximity,rotation,tap,gyroscope,invalid};

QStringList caseList;
QTextStream cin(stdin, QIODevice::ReadOnly);

QTextStream cout(stdout, QIODevice::WriteOnly);

QTextStream cerr(stderr, QIODevice::WriteOnly);

/* usage */
// Print out usage
void usage(QStringList &casenameList)
{
  enum casename i;
	cout << ("Usage: SensorTest testcase_name \n")<<endl;

  cout <<"testcase_name: \n" << endl;
  for (i=accelerometer; i<=gyroscope; i= casename(i+1))
  {	
     cout << casenameList.at(i)<< endl;
  }	
 }

/* testSesnor */
// Test sensor
int testSensor(QSensor * pSensor, QCoreApplication * pApp)
{
	SensorsTest sensorsTest;
  sensorsTest.m_Sensor = pSensor;
	sensorsTest.m_app = pApp;
  return sensorsTest.TestSensor();

}

/* main */
// 
int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  enum casename i;
  int iReturn = 0;
  caseList << "accelerometer" << "als"<<"compass"<<"magnetometer"<<"orientation"<<"proximity"<<"rotation"<<"tap"<<"gyroscope"<<"invalid";
   
  int argCount = app.arguments().length();
  QSensor * pSensor = NULL;

    
  if (argCount < 2)
  {
		usage(caseList);
    return iReturn;	
  }
     
  QString testCase = app.arguments().at(1);
  if (testCase.isEmpty())
  {
		usage(caseList);
		return iReturn;
  }	

  for (i=accelerometer; i<=invalid; i= casename(i+1))
  {
  	if(testCase == caseList.at(i))
	    	break;
  }	
       
  switch (i)
	{
	  case accelerometer:    
	  pSensor = new QAccelerometer;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  case als:
		pSensor = new QAmbientLightSensor;
    iReturn = testSensor(pSensor,&app);
		break;
		
    case compass:
		pSensor = new QCompass;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  case magnetometer:
		pSensor = new QMagnetometer;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  case orientation:
		pSensor = new QOrientationSensor;
    iReturn = testSensor(pSensor,&app);
		break;
		
    case proximity:
		pSensor = new QProximitySensor;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  case rotation:
		pSensor = new QRotationSensor;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  case tap:
		pSensor = new QTapSensor;
    iReturn = testSensor(pSensor,&app);
		break; 
		
	  case gyroscope:
		pSensor = new QGyroscope;
    iReturn = testSensor(pSensor,&app);
		break;
		
	  default:
 		usage(caseList);
		break;
	}
  if (pSensor != NULL)
  {
		delete pSensor;
    pSensor = NULL;
  }	
  return iReturn; 
}

