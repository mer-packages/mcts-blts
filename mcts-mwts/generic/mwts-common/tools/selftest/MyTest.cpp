#include "stable.h"
#include "MyTest.h"
#include <MwtsCommon.h>


void MyTest::OnInitialize()
{
	MWTS_ENTER;
	qDebug() << "MyTest::OnInitialize called!";

	MWTS_LEAVE;
}

void MyTest::OnUninitialize()
{
	MWTS_ENTER;
	qDebug() << "MyTest::OnUninitialize called!";
	MWTS_LEAVE;
}

QString MyTest::CaseName()
{
	return "MyTest selftest";
}
