/* FilesystemTest.h
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


#ifndef _INCLUDED_FILESYSTEM_TEST_H
#define _INCLUDED_FILESYSTEM_TEST_H


#include <MwtsCommon>

#define DEV_FIRST		1
#define DEV_ROOT		1
#define DEV_TMP			2
#define DEV_EMMC_FAT	3
#define DEV_EMMC_EXT	4
#define DEV_MMC			5
#define DEV_USBMSD_MMC	6
#define DEV_USBMSD_EMMC	7
#define DEV_UBI			8
#define DEV_SECURITY	9
#define DEV_LAST		9

#define FS_MAX_BLOCK_SIZE 65536

class FilesystemTest : public MwtsTest
{
	Q_OBJECT
public:
	FilesystemTest(QObject* pParent=0, QString platform="");
	virtual ~FilesystemTest();

	void OnInitialize();
	void OnUninitialize();

public:
	bool SelectSourceDevice(int device);
	bool SelectTargetDevice(int device);
	void SetBlockSize(int bytes);
	void SetBlockCount(int blockcount);
	void SetIterationCount(int itercount);
	bool MeasureCreateFileLatency();
	bool MeasureRemoveFileLatency();
	bool MeasureMoveFileLatency();
	bool MeasureCreateDirectoryLatency();
	bool MeasureRemoveDirectoryLatency();
	bool ModifyAttributes();
	bool MeasureWriteSpeed();
	bool MeasureReadSpeed();
	bool MeasureSyncLatency();
	bool MeasureCopySpeed();
	bool FormatTarget();

protected:
	bool CleanAll();
	bool MountDevice(int device);
	bool UnmountDevice(int device);
	bool FormatDevice(int device);

	bool CleanCache();
	bool CleanDirectory(QString dirname);
	void CleanLocalMounts();
	QString GetDeviceDir(int device);

	bool CreateDir(QString directory);
	bool CreateTargetDir();
	bool CreateSourceDir();
	bool CreateDirectories();

	bool RemoveDirectories();
	bool CreateFiles(QString targetdir);
	bool RemoveFiles(QString targetdir);
	bool MoveFiles();
	bool WriteData(QString filename);
	bool WriteSourceData();
	bool WriteTargetData();
	bool ReadTargetData();
	bool CopySourceToTarget();
	bool IsMountable(int nDevice);

	int m_nBlockSize;
	int m_nBlockCount;
	int m_nIterationCount;
	int m_nTargetDevice;
	int m_nSourceDevice;
	QString m_sTargetDir;
	QString m_sSourceDir;
	QString m_sDataFilename;
	QString m_sSourceSubdir;
	QString m_sTargetSubdir;
	char m_Buffer[FS_MAX_BLOCK_SIZE];

};




#endif //#ifndef _INCLUDED_FILESYSTEM_TEST_H

