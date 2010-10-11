/* mwtsmin.cpp --
 *
 * This file is part of MCTS 
 * 
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
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


#include "stable.h"
#include <MwtsCommon>
#include "mwtsload.h"
#include "mwtsthroughput.h"
#include "mwtsradio.h"

#include <stdio.h>
#include <stdlib.h>

/* MIN interface stuff */

#ifdef ENTRYTC
#undef ENTRYTC
#endif

#define ENTRYTC(_l_, _n_,_f_)                                           \
	do {                                                            \
	TestCaseInfoTC* tc = (TestCaseInfoTC*)malloc(sizeof(TestCaseInfoTC));\
	if( tc == NULL ) break;                                         \
	STRCPY(tc->name_,_n_,MaxTestCaseName);                          \
	tc->test_ = _f_;                                                \
	tc->id_   = dl_list_size(_l_)+1;                                \
	dl_list_add( _l_, (void*)tc );                                  \
	} while(0)

typedef struct _TestCaseInfoTC TestCaseInfoTC;
typedef struct _ScriptVariable ScriptVariable;
typedef int (*ptr2testtc)( MinItemParser * tcr );

struct _TestCaseInfoTC
{
	char name_[MaxTestCaseName];
	ptr2testtc test_;
	unsigned int id_;
};
struct _ScriptVariable
{
	char *var_name_;
	TSBool is_initialized_;
	char *var_value_;
};


/* Min scripter functions */

/**
  Initializes the test module
*/

int Init ( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	g_pTest->Initialize();
	MWTS_LEAVE;
    return 0;
}

/**
  Uninitializes the test module
*/

int Close ( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	int ret = 0;
	if(!g_pTest->IsPassed())
	{
		MWTS_LEAVE;
		ret = 1;
	}
	g_pTest->Uninitialize();
	MWTS_LEAVE;
	return ret;
}

/**
  Starts new iteration
  Used as first function after loop starts
*/
int StartIteration( __attribute__((unused)) MinItemParser * item )
{
	MWTS_ENTER;
	g_pResult->NextIteration();
	return 0;
}

/**
  Ends current iteration
  Used as the last function in loop
*/
int EndIteration( __attribute__((unused)) MinItemParser * item )
{
	MWTS_ENTER;
	return 0;
}

/**
  Sets testing timeout
  Test is stopped when timeout occurs
  param: Timeout (int) sets the amount of time in milliseconds
*/
int SetTestTimeout( MinItemParser * item )
{
	MWTS_ENTER;
	int arg = 10000;
	if ( mip_get_next_int( item, &arg ) == 0 )
	{
		g_pTest->SetTestTimeout( arg );
		return 0;
	}
	return 1;
}

/**
  Sets the fail timeout
  If the test is not stopped naturally before this timeout
  the test is marked as timeouted and is failed
  Test is stopped when timeout occurs.
  param: Timeout (int) sets the amount of time in milliseconds
*/
int SetFailTimeout( MinItemParser * item )
{
	MWTS_ENTER;
	int arg = 30000;
	if ( mip_get_next_int( item, &arg ) == 0 )
	{
		g_pTest->SetFailTimeout( arg );
		return 0;
	}
	return 1;
}

/**
  Starts CPU load
  param: Load (int) amount of cpu load, percentage
*/
int StartCpuLoad ( MinItemParser* item )
{
	MWTS_ENTER;

	int load = 0, ret;

	ret = mip_get_next_int( item, &load);

	if (ret != ENOERR)
	{
		MWTS_ERROR ("Missing load parameter in StartCpuLoad");
		return 1;
	}

	g_pCpuLoad->SetAmount(load);
	if(!g_pCpuLoad->Start())
	{
		MWTS_ERROR("Failed to start CPU load");
		return 1;
	}
	return 0;
}

/**
  Stops current cpu load process
*/

int StopCpuLoad ( __attribute__((unused)) MinItemParser* item )
{
	MWTS_ENTER;
	g_pCpuLoad->Stop();
	return 0;
}

/**
   Starts memory load
   param: Load (int) amount of memory load in megabytes
*/
int StartMemLoad ( MinItemParser* item )
{
	MWTS_ENTER;
	int load = 0, ret;
	ret = mip_get_next_int( item, &load);
	if (ret != ENOERR)
	{
		MWTS_ERROR ("Missing load parameter in StartMemLoad");
		return 1;
	}

	g_pMemLoad->SetAmount(load);
	if(!g_pMemLoad->Start())
	{
		MWTS_ERROR("Failed to start CPU load");
		return 1;
	}
	return 0;
}

/**
  Stops current memory load process
*/
int StopMemLoad ( __attribute__((unused)) MinItemParser* item )
{
	MWTS_ENTER;
	g_pMemLoad->Stop();
	return 0;
}

/**
  Sets client side throughput maeasuring
  param : serverIP (string) IP to connect
*/
int StartClientThroughput ( MinItemParser* item )
{
	MWTS_ENTER;

	char *IP_str = (char*) INITPTR;
	int transtime = 0;
	if (mip_get_next_string(item, &IP_str) != ENOERR)
	{
		MWTS_ERROR ("Missing server IP parameter in StartClientThroughput");
		return 1;
	}

	if (mip_get_next_int(item, &transtime) != ENOERR)
	{
		MWTS_DEBUG("No time given, using default");
		transtime = 0;
	}

	g_pIPerfThroughput-> SetClientMeasurement(IP_str, transtime, "upload");
	if(!g_pIPerfThroughput->Start())
	{
		MWTS_ERROR("Failed to start client throughput measuring");
		return 1;
	}
	return 0;
}

/**
  Sets servert side throughput maeasuring
*/
int StartServerThroughput ( MinItemParser* item )
{
	MWTS_ENTER;
	item=item;
	g_pIPerfThroughput->SetServerMeasurement();
	if(!g_pIPerfThroughput->Start())
	{
		MWTS_ERROR("Failed to start server throughput maeasuring");
		return 1;
	}
	return 0;
}

/**
  Stops current throughput maeasuring process
*/

int StopThroughput ( __attribute__((unused)) MinItemParser* item )
{
	MWTS_ENTER;
	g_pIPerfThroughput->Stop();
	return 0;
}

/**
  Stops current memory load process
*/
int EnableUI ( __attribute__((unused)) MinItemParser* item )
{
	MWTS_ENTER;
	MwtsApp::EnableUI(true);
	return 0;
}



/**
* Changes Radio Mode (ANY/GSM/UMTS/LTE)
*
*/
int SetRadioMode(MinItemParser* item)
{
	MWTS_ENTER;
	char *mode_str = (char*) INITPTR;
	int mode=0;
	if (mip_get_next_string(item, &mode_str) != ENOERR)
	{
		return 1;
	}
	if(0==strcmp("ANY", mode_str))
	{
		mode=RADIO_ANY;
		MWTS_DEBUG("Changing radio access to ANY");
	}
	else if(0==strcmp("GSM", mode_str))
	{
		mode=RADIO_GSM;
		MWTS_DEBUG("Changing radio access to GSM");
	}
	else if(0==strcmp("UMTS", mode_str))
	{
		mode=RADIO_UMTS;
		MWTS_DEBUG("Changing radio access to UMTS");
	}
	else if(0==strcmp("LTE", mode_str))
	{
		mode=RADIO_LTE;
		MWTS_DEBUG("Changing radio access to LTE");
	}

	else
	{
		qCritical() << "Invalid radio access mode defined : " << mode_str;
		return 1;
	}

	if(!MwtsRadio::ChangeMode(mode))
	{
		MWTS_ERROR("Changing radio access technology failed!");
		return 1;
	}

	sleep(10); // wait 10 seconds for mode change to take effect - no need to do this in script

	free(mode_str);
	return 0;
}



/**
  Stops current memory load process
*/
int SetLimits ( MinItemParser* item )
{
	MWTS_ENTER;
	double lfTarget=0;
	double lfFail=0;
	char *target = (char*) INITPTR;
	char *fail = (char*) INITPTR;

	if (mip_get_next_string(item, &target) == ENOERR)
	{
		if(1 != sscanf(target, "%lf", &lfTarget))
		{
			qCritical() << "Invalid target value for SetLimits : "<< target;
			free(target);
			return 1;
		}
	}
	else
	{
		qCritical() << "Missing target parameter in SetLimits"<< target;
		free(target);
		return 1;
	}
	free(target);

	if (mip_get_next_string(item, &fail) == ENOERR)
	{
		if(1 != sscanf(fail, "%lf", &lfFail))
		{
			qCritical() << "invalid fail limit value for SetLimits : "<< target;
			free(fail);
			return 1;
		}
	}
	else
	{
		qCritical() << "Missing fail limit parameter in SetLimits"<< target;
		free(fail);
		return 1;
	}
	free(fail);

	g_pResult->SetLimits(lfTarget, lfFail);
	return 0;
}





/**
  Declares MIN functions of common library
  This should be called in your test assets ts_get_test_cases function
  It will declare mwts-common min functions so you can use
  them in your asset.
  param: DLList list parameter of your ts_get_test_cases function
*/

void MwtsMin::DeclareFunctions(DLList ** list)
{
	MWTS_ENTER;

	ENTRYTC(*list, "Init", Init);
	ENTRYTC(*list, "Close", Close);

	ENTRYTC(*list, "SetTestTimeout", SetTestTimeout);
	ENTRYTC(*list, "SetFailTimeout", SetFailTimeout);

	ENTRYTC(*list, "StartIteration", StartIteration);
	ENTRYTC(*list, "EndIteration", EndIteration);

	ENTRYTC(*list, "StartCpuLoad", StartCpuLoad);
	ENTRYTC(*list, "StopCpuLoad", StopCpuLoad);
	ENTRYTC(*list, "StartMemLoad", StartMemLoad);
	ENTRYTC(*list, "StopMemLoad", StopMemLoad);
	ENTRYTC(*list, "StartClientThroughput", StartClientThroughput);
	ENTRYTC(*list, "StartServerThroughput", StartServerThroughput);
	ENTRYTC(*list, "StopThroughput", StopThroughput);
	
	ENTRYTC(*list, "EnableUI", EnableUI);

	ENTRYTC(*list, "SetLimits", SetLimits);
	ENTRYTC(*list, "SetRadioMode", SetRadioMode);

}
