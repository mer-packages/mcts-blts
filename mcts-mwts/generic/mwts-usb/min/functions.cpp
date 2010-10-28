/* functions.cpp -- min interface for mwts-usb test asset
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
#include "interface.h"
#include "UsbTest.h"

#include <MwtsCommon>


const char *module_date = __DATE__;
const char *module_time = __TIME__;

UsbTest g_UsbTest;


// SendFile
// Send a file to server
// Parameters:
//  ip to send file
//	src file to copy
//	dst to copy
LOCAL int SendFile( MinItemParser* item )
{
	MWTS_ENTER;

	char *arg = NULL;
	char *ip = NULL;
	char *dst = NULL;
	char *src = NULL;
	int ret = 0;

	while ( mip_get_next_string( item, &arg ) == ENOERR )
	{
		if ( strcmp( arg, "ip" ) == 0 )
		{
			int ret = mip_get_next_string( item, &ip );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}

		if ( strcmp( arg, "srcfile" ) == 0 )
		{
			int ret = mip_get_next_string( item, &src );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}

		if ( strcmp( arg, "destfile" ) == 0 )
		{
			int ret = mip_get_next_string( item, &dst );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}
	}

	bool success = false;

	if( ip != NULL && dst != NULL && src != NULL)
	{
		// Check that file exists
		QFile file;
		file.setFileName(src);
		if (file.exists())
		{
			// dst
			char* dst2 = new char[128];
			sprintf(dst2, "root@%s:%s", ip, dst);
			g_UsbTest.Transfer(dst2, src);
			success = g_UsbTest.GetStatus();

			g_pResult->AddMeasure("Throughput", g_UsbTest.GetSendSpeed(), "MB/s");

			if(dst2)
			{
				delete dst2;
			}
		}
		else
		{
			qCritical() << "File doesn't exists: " << file.fileName();
		}
	}

	if(ip)
	{
		free(ip);
	}
	if(dst)
	{
		free(dst);
	}
	if(src)
	{
		free(src);
	}

	g_pResult->StepPassed( __FUNCTION__, success );

	ret = !success;

	MWTS_LEAVE;
	return ret;
}


// ReceiveFile
// Receives a file
// Parameters:
//  ip to send file
//	src file to copy
//	dst to copy
LOCAL int ReceiveFile( MinItemParser* item )
{
	MWTS_ENTER;

	char *arg = NULL;
	char *ip = NULL;
	char *dst = NULL;
	char *src = NULL;
	int ret = 0;

	while ( mip_get_next_string( item, &arg ) == ENOERR )
	{
		if ( strcmp( arg, "ip" ) == 0 )
		{
			int ret = mip_get_next_string( item, &ip );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}

		if ( strcmp( arg, "srcfile" ) == 0 )
		{
			int ret = mip_get_next_string( item, &src );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}

		if ( strcmp( arg, "destfile" ) == 0 )
		{
			int ret = mip_get_next_string( item, &dst );
			if ( ret != ENOERR )
			{
				MWTS_LEAVE;
				return ret;
			}
		}
	}

	bool success = false;

	if( ip != NULL && dst != NULL && src != NULL)
	{
		// dst
		char* src2 = new char[128];
		sprintf(src2, "root@%s:%s", ip, src);
		g_UsbTest.Transfer(dst, src2);
		success = g_UsbTest.GetStatus();
		g_pResult->AddMeasure("Throughput", g_UsbTest.GetReceiveSpeed(), "MB/s");

		if(src2)
		{
			delete src2;
		}
	}

	if(ip)
	{
		free(ip);
	}
	if(dst)
	{
		free(dst);
	}
	if(src)
	{
		free(src);
	}

	g_pResult->StepPassed( __FUNCTION__, success );

	ret = !success;

	MWTS_LEAVE;
	return ret;
}

int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...

	MwtsMin::DeclareFunctions(list);

	ENTRYTC (*list, "SendFile", SendFile);
	ENTRYTC (*list, "ReceiveFile", ReceiveFile);
	return ENOERR;
}

