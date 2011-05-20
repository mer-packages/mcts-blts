#include "foobarclass.h"
#include <iostream>
//#include <QApplication>


FooBarClass::FooBarClass(QObject *parent) : QObject(parent)
{
	std::cout << "*************************** in FooBar constructor" << std::endl;
	g_pApp=NULL;

	m_pIdleTimer=new QTimer(this);
	m_pIdleTimer->setSingleShot(false);


}

void FooBarClass::Initialize()
{
	//MwtsMonitor::instance()->Start();
	g_pApp = MwtsApp::instance();

	MWTS_ENTER;

	//MwtsMonitor::instance()->Start();
	/*QString testpath="/var/log/tests/";
	QDir testdir(testpath);
	if(!testdir.exists())
	{
		if(!testdir.mkpath(testpath))
		{
			qCritical() << "Unable to create test log dir: " << testpath;
		}
	}

	qDebug() << "im here *";

	QString logpath=testpath+CaseName()+".log";

	qDebug() << "im here **" << logpath;

	g_pLog->Open(logpath);

	qDebug() << "im here ***";

	g_pResult->Open(CaseName());

	qDebug() << "im here ****";

	qDebug()<<"Creating config";
	QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/lib/tests/");
	g_pConfig=new MwtsConfig();

	qDebug()<<"Creating application";
	setenv("DISPLAY", ":0.0", 1);
	g_pApp = MwtsApp::instance();
	this->OnInitialize();

	connect(m_pIdleTimer, SIGNAL(timeout()), this, SLOT(OnIdle()));
	m_pIdleTimer->start(1000);*/

	MWTS_LEAVE;

}

void FooBarClass::OnInitialize()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

void FooBarClass::OnUninitialize()
{
	MWTS_ENTER;
	MWTS_LEAVE;
}

QString FooBarClass::CaseName()
{
	// fetch name from MIN
	QString sName = QString(tm_get_caller_name());
	if(sName!="")
	{
		return sName;
	}
	if(m_sCaseName=="") // if no name is set
	{
		return "noname";
	}
	return m_sCaseName;
}

void FooBarClass::SetCaseName(QString sName)
{
	m_sCaseName=sName;
}

void FooBarClass::Start()
{
	std::cout << "*************************** startign main loop";
	g_pApp->exec();
}
