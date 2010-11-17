/* FilesystemTest.cpp
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
#include "FilesystemTest.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/statfs.h>


FilesystemTest::FilesystemTest(QObject* pParent/*=0*/, QString platform/*=""*/)
: MwtsTest(pParent, platform)
{
	MWTS_ENTER;

}

FilesystemTest::~FilesystemTest()
{
	MWTS_ENTER;
}


void FilesystemTest::OnInitialize()
{
	MWTS_ENTER;

	m_nBlockSize=1024;
	m_nBlockCount=1024;
	m_nIterationCount=100;
	m_nTargetDevice=0;
	m_nSourceDevice=0;
	m_sTargetDir="/invalid/target/directory";
	m_sSourceDir="/invalid/source/directory";
	m_sDataFilename="testfile";
	m_sTargetSubdir="fs_target";
	m_sSourceSubdir="fs_source";
}

void FilesystemTest::OnUninitialize()
{
	MWTS_ENTER;
}


bool FilesystemTest::CleanCache()
{
	sync();
	int file=open("/proc/sys/vm/drop_caches", O_WRONLY);
	write(file, "3\n", 2);
	close(file);
	return true;
}

/**
  Cleans test directories. To be used before testing. m_nTargetDevice
  and m_nSourceDevice must be defined before usage. If target or source
  devices are mountable, they will be unmounted, formatted and then mounted.
  Additionally any existing test directory is removed and data recursively deleted.
*/

bool FilesystemTest::CleanAll()
{
	MWTS_ENTER;
	if(m_nTargetDevice)
	{
		if(IsMountable(m_nTargetDevice))
		{
			UnmountDevice(m_nTargetDevice);
			// FormatDevice(m_nTargetDevice);
			MountDevice(m_nTargetDevice);
		}
		CreateTargetDir();
	}
	if(m_nSourceDevice)
	{
		if(m_nTargetDevice!=m_nSourceDevice &&
			IsMountable(m_nSourceDevice))
		{
			UnmountDevice(m_nSourceDevice);
			// FormatDevice(m_nSourceDevice);
			MountDevice(m_nSourceDevice);
		}
		CreateSourceDir(); 
	}
	return true;
}


/**
  Recursive cleaning of directory
*/
bool FilesystemTest::CleanDirectory(QString dirname)
{
	MWTS_ENTER;
	int i;
	if(!QFile::exists(dirname))
	{
		qDebug() << "Directory " << dirname << " does not exist";
		return false;
	}

	QDir dir(dirname);

	QStringList listDirectories=dir.entryList(QDir::Dirs);
	for(i = 0; i < listDirectories.size(); ++i)
	{
		QString subdirname=listDirectories.at(i);
		if(subdirname==".")
			continue;
		if(subdirname=="..")
			continue;
		QString dirpath=dirname+"/"+subdirname;
		qDebug() << "Cleaning " << dirpath;
		CleanDirectory(dirpath);
		dir.rmdir(dirpath);
	}

	QStringList listFiles=dir.entryList(QDir::Files);

	for(i = 0; i < listFiles.size(); ++i)
	{
		QString filename=listFiles.at(i);
		QString filepath=dirname+"/"+filename;
		qDebug() << "Removing " << filepath;
		if(!QFile(filepath).remove())
		{
			qDebug() << "Could not remove " <<filepath;
		}
	}

	return true;
}

bool FilesystemTest::IsMountable(int nDevice)
{
	switch(nDevice)
	{
	case DEV_EMMC_FAT:
		return false;
		break;
	case DEV_EMMC_EXT3:
	case DEV_EMMC_EXT4:
		return false;
		break;
	case DEV_MMC:
		return true;
		break;
	default:
		break;
	}
	return false;
}

bool FilesystemTest::MountDevice(int device)
{
	MWTS_ENTER;
	QString devicename;
	QString devicetype;
	switch(device)
	{
	case DEV_EMMC_FAT:
		devicename = g_pConfig->value("emmc_fat_device").toString();
		devicetype = "vfat";
		break;
	case DEV_EMMC_EXT3:
		devicename = g_pConfig->value("emmc_ext3_device").toString();
		devicetype = "ext3";
		break;
	case DEV_EMMC_EXT4:
		devicename = g_pConfig->value("emmc_ext4_device").toString();
		devicetype = "ext4";
		break;
	case DEV_MMC:
		devicename = g_pConfig->value("mmc_device").toString();
		devicetype = "vfat";
		break;
	default:
		qCritical() << "Only eMMC and MMC mounts supported!";
		return false;
	}
	QString mountdir = GetDeviceDir(device);

	QProcess process;
	QStringList params;
	params << "-t" << devicetype << devicename << mountdir;
	process.start("/bin/mount", params);
	if( ! process.waitForFinished (5000))
	{
		qCritical() << "Mount command not done after 5 seconds - killing";
		process.kill();
		return false;
	}
	// todo: verify the mount was succesfull
	return true;
}

bool FilesystemTest::UnmountDevice(int device)
{
	MWTS_ENTER;
	QString devicename;
	QString devicetype;
	switch(device)
	{
	case DEV_EMMC_FAT:
	case DEV_EMMC_EXT3:
	case DEV_EMMC_EXT4:
	case DEV_MMC:
		break;
	default:
		qCritical() << "Only eMMC and MMC mounts supported!";
		return false;
	}
	QString mountdir = GetDeviceDir(device);

	QProcess process;
	QStringList params;
	params << mountdir;
	process.start("/bin/umount", params);
	if( ! process.waitForFinished (5000))
	{
		qCritical() << "Umount command not done after 5 seconds - killing";
		process.kill();
		return false;
	}
	// todo: verify the unmount was succesfull
	return true;
}


bool FilesystemTest::FormatDevice(int device)
{
	MWTS_ENTER;
	QString devicename;
	switch(device)
	{
	case DEV_EMMC_FAT:
		devicename = g_pConfig->value("emmc_fat_device").toString();
		break;
	case DEV_MMC:
		devicename = g_pConfig->value("mmc_device").toString();
		break;
	default:
		qCritical() << "Only eMMC FAT and MMC formatting supported!";
		return false;
	}

	QProcess process;
	QStringList params;
	params << devicename;
	qDebug() << "running mkfs.vfat for "<< devicename;
	process.start("/sbin/mkfs.vfat", params);

	g_pTime->start();
	if( ! process.waitForFinished (5000))
	{
		qCritical() << "Formatting device not ready after 5 seconds";
		process.kill();
		return false;
	}

	double elapsed=g_pTime->elapsed();

	if(process.exitCode() == 0)
	{
		qDebug()<<"Succesfully formatted in "<< elapsed <<" milliseconds";
	}
	else
	{
		qCritical()<<"Formatting failed : " << process.readAll();
		
		return false;
	}

	return true;

}



/* interface change here: now returns allocated instead of static strings (mount points read via HAL) -JS */
QString FilesystemTest::GetDeviceDir(int device)
{
	MWTS_ENTER;
	switch(device)
	{
	case DEV_ROOT:
        return g_pConfig->value("root_dir").toString();
	case DEV_TMP:
		return g_pConfig->value("tmp_dir").toString();
	case DEV_EMMC_FAT:
		return g_pConfig->value("emmc_fat_dir").toString();
	case DEV_EMMC_EXT3:
		return g_pConfig->value("emmc_ext3_dir").toString();
	case DEV_EMMC_EXT4:
		return g_pConfig->value("emmc_ext4_dir").toString();
	case DEV_MMC:
		return g_pConfig->value("mmc_dir").toString();
	case DEV_USBMSD_MMC:
		return g_pConfig->value("usbmsd_mmc_dir").toString();
	case DEV_USBMSD_EMMC:
		return g_pConfig->value("usbmsd_emmc_dir").toString();
	case DEV_UBI:
		return g_pConfig->value("ubi_dir").toString();
	case DEV_SECURITY:
		return g_pConfig->value("security_dir").toString();
	default:
		qCritical() << "Invalid device type : " << device;
	}

	return "";
}

/**
  Create directory and clean it if existing 
  @param directory Directory to be created or cleaned
*/
bool FilesystemTest::CreateDir(QString directory)
{
	MWTS_ENTER;

	qDebug() << "Creating directory " << directory;
	// try to create directory
	int error=0;
	if(0 != mkdir(directory.toAscii().constData(), S_IRWXU | S_IRWXG | S_IRWXO))
	{
		error=errno; // errno has to be saved locally as qDebug sometimes changes errno

		qDebug() << "Creating directory " << directory << " failed";
		// if it already exists, clean it
		if(error==EEXIST)
		{
			// this is ok, we just need to clean it
			qDebug() << "Test directory " << directory.toAscii().constData() << " already existing";
			if(!CleanDirectory(directory))
			{
				qCritical() << "Failed to clean test directory " << directory;
				return false;
			}
		}
		else
		{
			qCritical() << "Unable to create directory " << directory << " : error " << strerror(error);
			return false;
		}
	}
	else
	{
		qDebug() << "Created directory " << directory;
	}

	return true;
}

bool FilesystemTest::CreateTargetDir()
{
	MWTS_ENTER;
	return CreateDir(m_sTargetDir);
}

bool FilesystemTest::CreateSourceDir()
{
	MWTS_ENTER;
	return CreateDir(m_sSourceDir);
}


bool FilesystemTest::SelectSourceDevice(int device)
{
	MWTS_ENTER;

	m_nSourceDevice=device;

	QString devdir = GetDeviceDir(device);
	if(devdir=="")
	{
		qCritical() << "Cannot find test directory for source device" << device;
		return false;
	}
	m_sSourceDir=devdir+"/"+m_sSourceSubdir + this->CaseName();
	qDebug() << "Source directory " << m_sSourceDir;

	return true;
}

bool FilesystemTest::SelectTargetDevice(int device)
{
	MWTS_ENTER;

	m_nTargetDevice=device;

	QString devdir = GetDeviceDir(device);
	if(devdir=="")
	{
		qCritical() << "Cannot find test directory for target device" << device;
		return false;
	}
	m_sTargetDir=devdir+"/" + m_sTargetSubdir + this->CaseName();
	qDebug() << "Target directory " << m_sTargetDir;

	return true;
}



bool FilesystemTest::CreateDirectories()
{
	MWTS_ENTER;
	int status;
	int i;
	for(i = 1; i <= m_nIterationCount; ++i)
	{
		sprintf(m_Buffer, "%s/_dir_%04d", m_sTargetDir.toAscii().constData(), i);
		status = mkdir(m_Buffer, S_IRWXU | S_IRWXG | S_IRWXO);
		if(status != 0)
		{
			qCritical() << "Could not create directory " << m_Buffer << " : " << strerror(errno);
			return false;
		}
	}
	return true;
}

bool FilesystemTest::RemoveDirectories()
{
	MWTS_ENTER;
	int status;
	int i;
	for(i = 1; i <= m_nIterationCount; ++i)
	{
		sprintf(m_Buffer, "%s/_dir_%04d", m_sTargetDir.toAscii().constData(), i);

		status = rmdir(m_Buffer);
		if(status != 0)
		{
			qCritical() << "Could not remove directory "<< m_Buffer << " : " << strerror(errno);
			return false;
		}
	}
	return true;
}

bool FilesystemTest::CreateFiles(QString targetdir)
{
	MWTS_ENTER;
	QString sTarget;
	int i;
	for(i = 1; i <= m_nIterationCount; ++i)
	{
		sTarget=targetdir+"/_file_"+QString::number(i);
		FILE* f = fopen(sTarget.toAscii().constData(), "w");
		if(!f)
		{
			qCritical() << "Could not open file " << sTarget << " : " << strerror(errno);
			return false;
		}
//		fwrite(f, " ", 1, 1, f);
		fclose(f);
	}
	return true;
}

bool FilesystemTest::RemoveFiles(QString targetdir)
{
	MWTS_ENTER;
	int status;
	QString sTarget;
	int i;
	for(i = 1; i <= m_nIterationCount; ++i)
	{
		sTarget=targetdir+"/_file_"+QString::number(i);
		status = remove(sTarget.toAscii().constData());
		if(status != 0)
		{
			qCritical() << "Could not remove file " << sTarget << " : " << strerror(errno);
			return false;
		}
	}

	return true;
}


bool FilesystemTest::MoveFiles()
{
	QString sSource;
	QString sTarget;
	int i;
	for(i = 1; i <= m_nIterationCount; ++i)
	{
		sSource=m_sSourceDir+"/_file_"+QString::number(i);
		sTarget=m_sTargetDir+"/_file_"+QString::number(i);
		if(0 != rename(sSource.toAscii().constData(), sTarget.toAscii().constData()))
		{
			qCritical() << "Could not move file " << sSource << " to " << sTarget << " : " << strerror(errno);
			return false;
		}
	}
	return true;

}

bool FilesystemTest::WriteData(QString filename)
{
	MWTS_ENTER;
    int targetfile = open(filename.toAscii().constData(), O_CREAT | O_WRONLY, (S_IRUSR|S_IWUSR));
//    int targetfile = open(filename.toAscii().constData(), O_CREAT | O_WRONLY);
	if(-1 == targetfile)
	{
		qCritical() << "Unable to open file " << filename << " : " << strerror(errno);
		return false;
	}
	int written=0;
	// todo: put some data with checksum to the block

	for(int i=0; i<m_nBlockCount; i++)
	{
		written=write(targetfile, m_Buffer, m_nBlockSize);
		if(m_nBlockSize != written)
		{
			qCritical() << "Unable to write block " << i << " : " << written << " written, not " << m_nBlockSize;
			close(targetfile);
			return false;
		}
	}

	int syncret = 0;
	syncret = fdatasync(targetfile);
	if(syncret != 0)
	{
		qCritical() << "fdatasync returns " << syncret << " and errno " << strerror(errno);
	}
	//Close function will deallocate the filedescriptor which is needed by fdatasync()
	close(targetfile);

	return true;
}


bool FilesystemTest::WriteSourceData()
{
	MWTS_ENTER;
	return WriteData(m_sSourceDir+"/"+m_sDataFilename);
}


bool FilesystemTest::WriteTargetData()
{
	MWTS_ENTER;
	return WriteData(m_sTargetDir+"/"+m_sDataFilename);
}


bool FilesystemTest::ReadTargetData()
{
	MWTS_ENTER;

	QString sTargetFile = m_sTargetDir+"/"+m_sDataFilename;

	int sourcefile = open(sTargetFile.toAscii().constData(), O_RDONLY);

	if(-1 == sourcefile)
	{
		qCritical() << "Unable to open file "<< sTargetFile << " : " << strerror(errno);
		return false;
	}

	// todo: put some data with checksum to the block
	for(int i=0; i<m_nBlockCount; i++)
	{
		if(m_nBlockSize != read(sourcefile, m_Buffer, m_nBlockSize))
		{
			qCritical() << "Unable to read block " << i;
			close(sourcefile);
			return false;
		}
	}
	close(sourcefile);
	return true;

}

bool FilesystemTest::CopySourceToTarget()
{
	MWTS_ENTER;
	QString sSourceFilename = m_sSourceDir+"/"+m_sDataFilename;
	QString sTargetFilename = m_sTargetDir+"/"+m_sDataFilename;

	qDebug() << "Opening source file " << sSourceFilename;
	int sourcefile = open(sSourceFilename.toAscii().constData(), O_RDONLY);
	if(-1 == sourcefile)
	{
		qCritical() << "Unable to open file " << sSourceFilename << " : " << strerror(errno);
		return false;
	}
	qDebug() << "Opening target file " << sTargetFilename;
        int targetfile = open(sTargetFilename.toAscii().constData(), O_CREAT|O_WRONLY, (S_IRUSR|S_IWUSR));
	if(-1 == targetfile)
	{
		qCritical() << "Unable to open file " << sTargetFilename << " : " << strerror(errno);
		close(sourcefile);
		return false;
	}
	int readbytes=0;
	int i=0;
	int written;

	ssize_t read(int fd, void *buf, size_t count);

	while(m_nBlockSize == (readbytes = read(sourcefile, m_Buffer, m_nBlockSize)))
	{
		i++;
		written=write(targetfile, m_Buffer, readbytes);
		if(written!=readbytes)
		{
			qCritical() << "Read bytes != written bytes";
			close(sourcefile);
			close(targetfile);
			return false;
		}
	}
	qDebug() << i << "*" << m_nBlockSize << " bytes copied";
	if(i!=m_nBlockCount)
	{
		qCritical() << "File should be " <<
			m_nBlockCount*m_nBlockSize <<
			" bytes but only " << i*m_nBlockSize << " copied";
		close(sourcefile);
		close(targetfile);
		return false;
	}
	close(sourcefile);
	close(targetfile);
	sync();
	return true;

}




void FilesystemTest::SetBlockSize(int bytes)
{
	MWTS_ENTER;
	if(bytes > FS_MAX_BLOCK_SIZE)
	{
		qCritical() << "Trying to set block size to"<<bytes<<"while max is"<<FS_MAX_BLOCK_SIZE;
	}
	m_nBlockSize=bytes;
}

void FilesystemTest::SetBlockCount(int blockcount)
{
	MWTS_ENTER;
	m_nBlockCount=blockcount;
}

void FilesystemTest::SetIterationCount(int itercount)
{
	MWTS_ENTER;
	m_nIterationCount = itercount;
}


bool FilesystemTest::FormatTarget()
{
	MWTS_ENTER;
	g_pTime->start();

	UnmountDevice(m_nTargetDevice);

	if(!FormatDevice(m_nTargetDevice))
	{
		g_pResult->StepPassed("Format", false);
		return false;
	}
	double elapsed_time = g_pTime->elapsed();
	g_pResult->StepPassed("Format", true);
	g_pResult->AddMeasure("format time", elapsed_time, "ms");
	return true;
}


bool FilesystemTest::MeasureCreateFileLatency()
{
	MWTS_ENTER;
	CleanAll();

	g_pTime->start();

	if(!CreateFiles(m_sTargetDir))
	{
		qCritical() << "Directory creation failed";
		g_pResult->StepPassed("create-files", FALSE);
		return false;
	}

	double nftresult = g_pTime->elapsed();
	nftresult /= (double)m_nIterationCount; // per directory
	g_pResult->StepPassed("create-files", TRUE);
	g_pResult->AddMeasure("create file latency", nftresult, "ms");

	CleanAll();

	return true;
}


bool FilesystemTest::MeasureRemoveFileLatency()
{
	MWTS_ENTER;
	CleanAll();

	if(true != CreateFiles(m_sTargetDir))
	{
		qCritical() << "Directory creations failed";
		g_pResult->StepPassed("remove-files", FALSE);
		return false;
	}

	g_pTime->start();

	if(true != RemoveFiles(m_sTargetDir))
	{
		qCritical() << "Directory removals failed";
		g_pResult->StepPassed("remove-files", FALSE);
		return false;
	}

	double nftresult = g_pTime->elapsed();

	nftresult /= (double)m_nIterationCount; // per directory
	g_pResult->StepPassed("remove-files", TRUE);
	g_pResult->AddMeasure("remove file latency", nftresult, "ms");

	CleanAll();

	return true;
}

bool FilesystemTest::MeasureMoveFileLatency()
{
	MWTS_ENTER;

	CleanAll();

	if(!CreateFiles(m_sSourceDir))
	{
		qCritical() << "Failed to create source files";
		return false;
	}

	g_pTime->start();

	if(!MoveFiles())
	{
		qCritical() << "Moving files failed";
		g_pResult->StepPassed("move-files", false);
		return false;
	}

	double nftresult = g_pTime->elapsed();
	nftresult /= (double)m_nIterationCount; // per directory
	g_pResult->StepPassed("move-files", TRUE);
	g_pResult->AddMeasure("move file latency", nftresult, "ms");

	CleanAll();

	return true;
}


bool FilesystemTest::MeasureCreateDirectoryLatency()
{
	MWTS_ENTER;

	CleanAll();

	g_pTime->start();

	if(true != CreateDirectories())
	{
		qCritical() << "Directory creation failed";
		g_pResult->StepPassed("create-directories", FALSE);
		return false;
	}

	double nftresult = g_pTime->elapsed();
	nftresult /= (double)m_nIterationCount; // per directory
	g_pResult->StepPassed("create-directories", TRUE);
	g_pResult->AddMeasure("create directory latency", nftresult, "ms");

	CleanAll();

	return true;
}


bool FilesystemTest::MeasureRemoveDirectoryLatency()
{
	MWTS_ENTER;
	CleanAll();

	if(true != CreateDirectories())
	{
		qCritical() << "Directory creations failed";
		g_pResult->StepPassed("remove-directories", FALSE);
		return false;
	}
	if(!CleanCache())
	{
		qCritical() << "Cleaning cache failed";
		g_pResult->StepPassed("clean-cache", FALSE);
		return false;
	}

	g_pTime->start();

	if(true != RemoveDirectories())
	{
		qCritical() << "Directory removals failed";
		g_pResult->StepPassed("remove-directories", FALSE);
		return false;
	}

	double nftresult = g_pTime->elapsed();

	nftresult /= (double)m_nIterationCount; // per directory
	g_pResult->StepPassed("remove-directories", TRUE);
	g_pResult->AddMeasure("remove directory latency", nftresult, "ms");

	CleanAll();

	return true;
}


bool FilesystemTest::ModifyAttributes()
{
	MWTS_ENTER;
	CleanAll();
	qCritical() << "ModifyAttributes not implemented yet";
	return false;
}



bool FilesystemTest::MeasureWriteSpeed()
{
	MWTS_ENTER;
	CleanAll();

	if(!CleanCache())
	{
		qCritical() << "Cleaning cache failed";
		g_pResult->StepPassed("clean-cache", FALSE);
		return false;
	}

	g_pTime->start();

	if(true != WriteTargetData())
	{
		qCritical() << "File writing failed";
		g_pResult->StepPassed("write-data", FALSE);
		return false;
	}

	double elapsed_time = g_pTime->elapsed();

	double speed = (double)m_nBlockSize*(double)m_nBlockCount/elapsed_time;
	speed /= 1000.0; // from B/ms to MB/s
	g_pResult->StepPassed("write-data", TRUE);
	g_pResult->AddMeasure("write throughput", speed, "MB/s");

	CleanAll();

	return true;


}


bool FilesystemTest::MeasureReadSpeed()
{
	MWTS_ENTER;

	CleanAll();

	if(!WriteTargetData())
	{
		qCritical() << "File writing failed";
		g_pResult->StepPassed("read-data", FALSE);
		return false;
	}

	if(!CleanCache())
	{
		qCritical() << "Cleaning cache failed";
		g_pResult->StepPassed("clean-cache", FALSE);
		return false;
	}
	g_pTime->start();

	if(!ReadTargetData())
	{
		qCritical() << "File reading failed";
		g_pResult->StepPassed("read-data", false);
		return false;
	}

	double elapsed_time = g_pTime->elapsed();

	double speed = (double)m_nBlockSize*(double)m_nBlockCount/elapsed_time;
	speed /= 1000.0; // from B/ms to MB/s
	g_pResult->StepPassed("read-data", TRUE);
	g_pResult->AddMeasure("read throughput", speed, "MB/s");

	CleanAll();

	return true;
}



bool FilesystemTest::MeasureSyncLatency()
{
	MWTS_ENTER;

	// clean test directories
	CleanAll();
	// initial sync to device
	sync(); 

	int i = 0;

	struct timeval start, end;

	long seconds, useconds;
	double mtime = 0.0;

	sprintf(m_Buffer, "%s/_file_%04d", m_sTargetDir.toAscii().constData(), i);

        int fd=open(m_Buffer, O_RDWR | O_CREAT, (S_IRUSR|S_IWUSR));
	if(fd==-1)
	{
		qCritical() << "Failed to open file " << m_Buffer;
		return false;
	}

	for(i=0; i<m_nIterationCount; i++)
	{
		write(fd, "test\n", 5);

		gettimeofday(&start, NULL);
		fsync(fd);
		gettimeofday(&end, NULL);
		seconds  = end.tv_sec  - start.tv_sec;
		useconds = end.tv_usec - start.tv_usec;
		mtime += (((double)seconds * 1000.0) + ((double)useconds/1000.0));

		usleep(100000);
	}

	mtime/=(double)m_nIterationCount;

	g_pResult->StepPassed("SyncLatencyMeasure", TRUE);
	g_pResult->AddMeasure("sync latency", mtime, "ms");

	qDebug() << "Average fsync time for " << m_Buffer << " : " <<mtime << " milliseconds";

	CleanAll();

	return true;
}




// Copying files

bool FilesystemTest::MeasureCopySpeed()
{
	MWTS_ENTER;

	CleanAll();

	if(!WriteSourceData())
	{
		qCritical() << "Failed to create source file";
		g_pResult->StepPassed("copy-data", FALSE);
		return false;
	}
	if(!CleanCache())
	{
		qCritical() << "Cleaning cache failed";
		g_pResult->StepPassed("clean-cache", FALSE);
		return false;
	}

	g_pTime->start();

	if(!CopySourceToTarget())
	{
		qCritical() << "Failed to copy source to target";
		g_pResult->StepPassed("copy-data", FALSE);
		return false;
	}

	double elapsed_time = g_pTime->elapsed();
	double speed = (double)m_nBlockSize*(double)m_nBlockCount/elapsed_time;
	speed /= 1000.0; // from B/ms to MB/s

	g_pResult->StepPassed("copy-data", TRUE);
	g_pResult->AddMeasure("copy throughput", speed, "MB/s");

	CleanAll();

	return true;
}



