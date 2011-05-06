#include "foobarclass.h"
#include <iostream>
//#include <QApplication>


FooBarClass::FooBarClass(QObject *parent) : QObject(parent)
{
	std::cout << "*************************** in FooBar constructor" << std::endl;
	g_pApp=NULL;
}

void FooBarClass::Initialize()
{
	g_pApp = MwtsApp::instance();
}

void FooBarClass::Start()
{
	std::cout << "*************************** startign main loop";
	g_pApp->exec();
}
