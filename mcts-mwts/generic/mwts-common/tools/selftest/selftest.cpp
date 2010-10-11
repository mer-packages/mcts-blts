#include "MyTest.h"
#include <MwtsCommon.h>
#include <MwtsMonitor.h>


int main()
{

	MwtsApp::EnableUI(true);
	MyTest test;
	g_pLog->EnableDebug(true);

	test.Initialize();

	for(int i=0; i<20; i++)
	{
		g_pResult->NextIteration();
		g_pResult->AddMeasure(
			"TestValue",
			i+(rand()%10)/10.0,
			"units");

		sleep(2);
	}

	test.Uninitialize();
	return 0;
}
