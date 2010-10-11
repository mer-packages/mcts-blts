#ifndef _INCLUDED_MYTEST_H
#define _INCLUDED_MYTEST_H

#include <MwtsTest.h>

class MyTest : public MwtsTest
{
	void OnInitialize();
	void OnUninitialize();
	QString CaseName();
};

#endif // #ifndef _INCLUDED_MYTEST_H
