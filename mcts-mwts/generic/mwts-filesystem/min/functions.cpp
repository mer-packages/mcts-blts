/* functions.cpp
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
#include "FilesystemTest.h"

#include <MwtsCommon>


const char *module_date = __DATE__;
const char *module_time = __TIME__;

FilesystemTest Test;

int get_device_from_param(MinItemParser * item)
{
	int device=0;
	char* string=NULL;
	if(mip_get_next_string( item, &string) != ENOERR )
	{
		MWTS_ERROR("Unable to get needed device parameter");
		return 0;
	}

	if(0==strcmp(string, "ROOT"))
		device=DEV_ROOT;
	else if(0==strcmp(string, "TMP"))
		device=DEV_TMP;
	else if(0==strcmp(string, "EMMC_FAT"))
		device=DEV_EMMC_FAT;
	else if(0==strcmp(string, "EMMC_EXT"))
		device=DEV_EMMC_EXT;
	else if(0==strcmp(string, "MMC"))
		device=DEV_MMC;
	else if(0==strcmp(string, "USBMSD_MMC"))
		device=DEV_USBMSD_MMC;
	else if(0==strcmp(string, "USBMSD_EMMC"))
		device=DEV_USBMSD_EMMC;
	else if(0==strcmp(string, "UBI"))
		device=DEV_UBI;
	else if(0==strcmp(string, "SECURITY"))
		device=DEV_SECURITY;
	else
	{
		qCritical() << "Unknown device type: " << string;
		return 0;
	}
	return device;
}



/***** Min scripter functions *****/

LOCAL int SelectSource( MinItemParser * item)
{
	MWTS_ENTER;
	int device=get_device_from_param(item);
	if(!device)
	{
		qCritical() << "Failed to get device parameter for SelectSource";
		return 1;
	}
	if(!Test.SelectSourceDevice(device))
	{
		qCritical() << "Failed to select source device: " << device;
		return 1;
	}

	MWTS_LEAVE;
	return 0;
}

LOCAL int SelectTarget( MinItemParser * item)
{
	MWTS_ENTER;
	int device=get_device_from_param(item);
	if(!device)
	{
		qCritical() << "Failed to get device parameter for SelectTarget";
		return 1;
	}
	if(!Test.SelectTargetDevice(device))
	{
		qCritical() << "Failed to select target device: " << device;
		return 1;
	}

	MWTS_LEAVE;
	return 0;
}

LOCAL int SetIterationCount( MinItemParser * item)
{
	MWTS_ENTER;
	int count=0;
	if(mip_get_next_int( item, &count) != ENOERR )
	{
		qCritical() << "No iteration count given";
		return 1;
	}
	Test.SetIterationCount(count);

	return 0;
}

LOCAL int SetBlockSize( MinItemParser * item)
{
	MWTS_ENTER;
	int size=0;
	if(mip_get_next_int( item, &size) != ENOERR )
	{
		qCritical() << "No block size given";
		return 1;
	}
	Test.SetBlockSize(size);
	return 0;
}

LOCAL int SetBlockCount( MinItemParser * item)
{
	MWTS_ENTER;
	int count=0;
	if(mip_get_next_int( item, &count) != ENOERR )
	{
		qCritical() << "No block count given";
		return 1;
	}
	Test.SetBlockCount(count);
	return 0;
}


LOCAL int CreateFileLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureCreateFileLatency();
	return 0;
}

LOCAL int RemoveFileLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureRemoveFileLatency();
	return 0;
}

LOCAL int MoveFileLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureMoveFileLatency();
	return 0;
}

LOCAL int CreateDirLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureCreateDirectoryLatency();
	return 0;
}

LOCAL int RemoveDirLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureRemoveDirectoryLatency();
	return 0;
}

LOCAL int SyncLatency( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureSyncLatency();
	return 0;
}

LOCAL int ReadSpeed( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureReadSpeed();
	return 0;
}

LOCAL int WriteSpeed ( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureWriteSpeed();
	return 0;
}

LOCAL int CopySpeed( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.MeasureCopySpeed();
	return 0;
}

LOCAL int FormatTarget( __attribute__((unused)) MinItemParser * item)
{
	MWTS_ENTER;
	Test.FormatTarget();
	return 0;
}


int ts_get_test_cases(DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);
	// filesystem min interface
	ENTRYTC(*list, "SelectTarget", SelectTarget);
	ENTRYTC(*list, "SelectSource", SelectSource);

	ENTRYTC(*list, "SetIterationCount", SetIterationCount);
	ENTRYTC(*list, "SetBlockSize", SetBlockSize);
	ENTRYTC(*list, "SetBlockCount", SetBlockCount);

	ENTRYTC(*list, "CreateFileLatency", CreateFileLatency);
	ENTRYTC(*list, "RemoveFileLatency", RemoveFileLatency);
	ENTRYTC(*list, "MoveFileLatency", MoveFileLatency);
	ENTRYTC(*list, "CreateDirLatency", CreateDirLatency);
	ENTRYTC(*list, "RemoveDirLatency", RemoveDirLatency);
	ENTRYTC(*list, "ReadSpeed", ReadSpeed);
	ENTRYTC(*list, "WriteSpeed", WriteSpeed);
	ENTRYTC(*list, "CopySpeed", CopySpeed);
	ENTRYTC(*list, "SyncLatency", SyncLatency);
	ENTRYTC(*list, "FormatTarget", FormatTarget);

	return ENOERR;
}


