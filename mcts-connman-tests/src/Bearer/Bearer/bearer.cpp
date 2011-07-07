/*
#  Copyright (C) 2010, Intel Corporation
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation; either version 2 of the License,
#  or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
#  USA.
#
#   Authors:
#        Jeff Zheng <jeff.zheng@intel.com>
*/
#include <QtTest/QtTest>
#include <QtNetwork>
#include <stdlib.h>

class Bearer: public QObject
{
    Q_OBJECT
private slots:
    void Scan();
    void Connect();
    void Disconnect();
    void Test();
};

void Bearer::Test()
{
    qDebug() << "The second case : Test";
    QVERIFY(1 == 2); 
}

void Bearer::Scan()
{
    char *penv;
    QNetworkConfigurationManager manager;
    QNetworkSession *session;
    QNetworkConfiguration cfg;
    penv = getenv("CM_Bearer");
    QList<QNetworkConfiguration> list = manager.allConfigurations();
    int i = 0;
    for (i = 0; i < list.size(); ++i) {
        if ( list.at(i).name() == penv ) {
		qDebug() << "Found " << penv << " !";
                cfg = list.at(i);
		break;
	}
    }
    QVERIFY2(i < list.size(), "Service not found");
}

void Bearer::Connect()
{
    char *penv;
    QNetworkConfigurationManager manager;
    QNetworkSession *session;
    QNetworkConfiguration cfg;
    penv = getenv("CM_Bearer");
    QList<QNetworkConfiguration> list = manager.allConfigurations();
    int i = 0;
    for (i = 0; i < list.size(); ++i) {
        if ( list.at(i).name() == penv ) {
		qDebug() << "Found " << penv << " !";
                cfg = list.at(i);
		break;
	}
    }
    QVERIFY2(i < list.size(), "Service not found");
    session = new QNetworkSession(cfg, this);
    session->open();
    session->waitForOpened(-1);
}

void Bearer::Disconnect()
{
    char *penv;
    QNetworkConfigurationManager manager;
    QNetworkSession *session;
    QNetworkConfiguration cfg;
    penv = getenv("CM_Bearer");
    QList<QNetworkConfiguration> list = manager.allConfigurations();
    int i = 0;
    for (i = 0; i < list.size(); ++i) {
        if ( list.at(i).name() == penv ) {
		qDebug() << "Found " << penv << " !";
                cfg = list.at(i);
		break;
	}
    }
    QVERIFY2(i < list.size(), "Service not found");
    session = new QNetworkSession(cfg, this);
    session->disconnect();
    session->stop();
    session->close();
//    session->waitForClosed(-1);
}

QTEST_MAIN(Bearer)
#include "bearer.moc"
