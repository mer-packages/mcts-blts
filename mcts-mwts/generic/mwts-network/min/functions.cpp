/* function.cpp --
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
#include "NetworkTest.h"
#include "mwtsthroughput.h"
#include <MwtsCommon>

const char *module_date = __DATE__;
const char *module_time = __TIME__;

NetworkTest Test;

/*
 * Wlan scan
 */
LOCAL int Scan( MinItemParser * item)
{
	Test.UpdateConfigs();

        bool retval = false;

        char* ap_name = NULL;
        if (ENOERR != mip_get_next_string(item, &ap_name))
        {
                qCritical() << "Missing parameter: access point name";
                return EINVAL;

        }

	if(Test.IsUpdateComplete())
	{
            // scan completed, lets see if the ap was found
            retval = Test.IsAccessPointFound(ap_name);
	}

        g_pResult->StepPassed( __FUNCTION__, retval );

 	return 0;
}

/*
 * Connects to a default configuration
 */
LOCAL int ConnectToDefault( MinItemParser * item)
{
	qDebug() << "connecting to default ap";

	bool retval = false;
	retval = Test.ConnectToDefault();

	g_pResult->StepPassed( __FUNCTION__, retval );

	return 0;
}

/*
 * Establishes wlan/psd connection
 */
LOCAL int Connect( MinItemParser * item)
{
	char * ap_name = NULL;

	if ( mip_set_parsing_type( item, EQuoteStyleParsing ) != 0)
	{
		//LDX_MESSAGE("");
		qCritical() << "Error: cannot set parser to quote mode";

	}
	else
	{
		if (ENOERR != mip_get_next_string(item,&ap_name)) {
			qCritical() << "Missing argument: access point identifier";
			return EINVAL;

		}
	}

	QString qap_name = ap_name;
	bool retval = false;

        retval = Test.ConnmanConnection(ap_name);

	free(ap_name);
	ap_name = NULL;

        g_pResult->StepPassed( __FUNCTION__, retval );

	return 0;
}

/*
 * Removes access point configuration from device
 */
LOCAL int RemoveService( MinItemParser * item)
{
        char * ap_name = NULL;

        if ( mip_set_parsing_type( item, EQuoteStyleParsing ) != 0)
        {
                //LDX_MESSAGE("");
                qCritical() << "Error: cannot set parser to quote mode";

        }
        else
        {
                if (ENOERR != mip_get_next_string(item,&ap_name)) {
                        qCritical() << "Missing argument: access point identifier";
                        return EINVAL;

                }
        }

        QString qap_name = ap_name;
        bool retval = false;

        retval = Test.RemoveService(ap_name);

        free(ap_name);
        ap_name = NULL;

        g_pResult->StepPassed( __FUNCTION__, retval );

        return 0;
}

/*
 * Removes access point configuration from device
 */
LOCAL int SetTethering( MinItemParser * item)
{
        char * mode = NULL;

        if ( mip_set_parsing_type( item, EQuoteStyleParsing ) != 0)
        {
                //LDX_MESSAGE("");
                qCritical() << "Error: cannot set parser to quote mode";

        }
        else
        {
                if (ENOERR != mip_get_next_string(item,&mode)) {
                        qCritical() << "Missing argument: tethering mode";
                        return EINVAL;

                }
        }

        QString tethering_mode = mode;
        bool retval = false;

        retval = Test.SetTethering(tethering_mode);

        free(mode);
        mode = NULL;

        g_pResult->StepPassed( __FUNCTION__, retval );

        return 0;
}

/*
 * Brings down the wlan interface
 */
LOCAL int Disconnect (__attribute__((unused)) MinItemParser * item)
{
        char * ap_name = NULL;

        if ( mip_set_parsing_type( item, EQuoteStyleParsing ) != 0)
        {
                //LDX_MESSAGE("");
                qCritical() << "Error: cannot set parser to quote mode";

        }
        else
        {
                if (ENOERR != mip_get_next_string(item,&ap_name)) {
                        qCritical() << "Missing argument: access point identifier";
                        return EINVAL;

                }
        }

        QString qap_name = ap_name;
        bool retval = false;

	// stop session just to make sure. Disconnect function brings down the interface
        retval = Test.StopSession(ap_name);

	g_pResult->StepPassed( __FUNCTION__, true );

	return 0;

}

/*
 * Download file from target url
 */
LOCAL int DownloadfileHttp( MinItemParser * item)
{
	char* filename = NULL;
        bool success = false;

	if (ENOERR != mip_get_next_string(item, &filename))
	{
		qCritical() << "Missing parameter: filename";
		return EINVAL;

	}

        // if we are not online, then there is no point to go further
        if (!Test.IsOnline()) {
                qCritical() << "No connection available, aborting http download!";
                return 1;
        } else {
                success = Test.DownloadfileHttp(filename);
        }

	g_pResult->StepPassed( __FUNCTION__, success );

 	return 0;
}

/*
 * Downloads file from target ftp
 */
LOCAL int Downloadfile( MinItemParser * item)
{
	char* filename = NULL;
        bool success = false;

	if (ENOERR != mip_get_next_string(item, &filename))
	{
		qCritical() << "Missing parameter: filename";

		return EINVAL;

	}

        // if we are not online, then there is no point to go further
        if (!Test.IsOnline()) {
                qCritical() << "No connection available, aborting ftp download!";
                return 1;
        } else {
                success = Test.Downloadfile(filename);
        }

	g_pResult->StepPassed( __FUNCTION__, success );

 	return 0;
}

/*
 * Uploads file to target ftp
 */
LOCAL int UploadFile( MinItemParser * item)
{
	char* filename = NULL;
        bool success = false;

	if (ENOERR != mip_get_next_string(item, &filename))
	{
            qCritical() << "Missing parameter: filename";

            return EINVAL;

	}

        // if we are not online, then there is no point to go further
        if (!Test.IsOnline()) {
            qCritical() << "No connection available, aborting ftp download!";
            return 1;
        }
        else {
            success = Test.UploadFile(filename);
        }



	g_pResult->StepPassed( __FUNCTION__, success );

 	return 0;
}


/*
 * Launches iperf client from remote server via ssh
 * this is used when measuring download troughput (server: client => device:server)
 */
LOCAL int IperfServerRemote( MinItemParser * item)
{
	char* time = NULL;
	if (ENOERR != mip_get_next_string(item, &time))
	{
		qCritical() << "Missing parameter: time";
		MWTS_LEAVE;
		return EINVAL;
	}

	bool success = false;

	success = Test.FindIp("wlan0");

	if(!success)
	{
		qDebug() << "no ip found for wlan0, searching for grps0";
		success = Test.FindIp("gprs0");
		if(!success)
		{
			qCritical() << "No ip found for the device, are you sure you are connected? Aborting....";
			MWTS_LEAVE;
			return EINVAL;
		}
	}

	success = Test.ServerLaunchIperf(time);

	if(!success)
	{
		qCritical() << "Something went wrong with the Iperf server";
		MWTS_LEAVE;
		return EINVAL;
	}

	//g_pResult->StepPassed( __FUNCTION__, success );

 	return 0;
}

/*
 * Start iperf client to measure upload troughput
 */
LOCAL int ServerStartIperf( MinItemParser * item)
{
	int time = 0;
	if (ENOERR != mip_get_next_int(item, &time))
	{
		qCritical() << "Missing parameter: time";
		MWTS_LEAVE;
		return 1;
	}

	// if we are not online, then there is no point to go further
	if(!Test.IsOnline())
	{
		qCritical() << "No connection available, aborting file Upload!";
		return 1;
	}

	MwtsIPerfThroughput* iperf = new MwtsIPerfThroughput();

	QString strIp;

	strIp = g_pConfig->value("SERVER/server_ip").toString();;

	// qstring => char conversion. could be done better
	QByteArray ba = strIp.toLatin1();
	char *ip = ba.data();

	qDebug() << "Starting iperf upload to ip: " << ip << " with time: " << time << " seconds.";

	iperf->SetClientMeasurement(ip, time, (char*)"upload");

	if(!iperf->Start())
	{
		MWTS_ERROR("Failed to start client throughput measuring");
		return 1;
	}

	return 0;
}


/*
 * Start iperf client with -r mode to measure upload and download troughput
 */
LOCAL int ServerStartDownload( MinItemParser * item)
{
	int time = 0;
	if (ENOERR != mip_get_next_int(item, &time))
	{
		qCritical() << "Missing parameter: time";
		MWTS_LEAVE;
		return EINVAL;
	}

	// if we are not online, then there is no point to go further
	if(!Test.IsOnline())
	{
		qCritical() << "No connection available, aborting file Upload!";
		return 1;
	}

	MwtsIPerfThroughput* iperf = new MwtsIPerfThroughput();

	QString strIp;

	strIp = g_pConfig->value("SERVER/server_ip").toString();;

	QByteArray ba = strIp.toLatin1();
	char *ip = ba.data();

	qDebug() << "Starting iperf bidirectional to ip: " << ip << " with time: " << time << " seconds.";

	iperf->SetClientMeasurement(ip, time, (char*)"bidirectional");
	//iperf->SetClientMeasurement(ip, time);
	if(!iperf->Start())
	{
		MWTS_ERROR("Failed to start client bidirectional throughput measuring");
		return 1;
	}

	return 0;
}

LOCAL int Idle (__attribute__((unused)) MinItemParser * item)
{
	Test.RunIdle();
	return 0;
}

/*
* turns wlan chip on/off
*/ 
LOCAL int SwitchWlan( MinItemParser * item)
{
        char* state = NULL;
        if (ENOERR != mip_get_next_string(item, &state))
        {
                qCritical() << "Missing parameter: on/off";

                return EINVAL;
        }

        bool success = Test.SwitchWlan(state);

        g_pResult->StepPassed( __FUNCTION__, success );

        return 0;  
}



int ts_get_test_cases (DLList ** list)
{
	// declare common functions like Init, Close, SetTestTimeout ...
	MwtsMin::DeclareFunctions(list);
	ENTRYTC(*list, "Scan", Scan);
	ENTRYTC(*list, "Connect", Connect);
	ENTRYTC(*list, "ConnectToDefault", ConnectToDefault);
	ENTRYTC(*list, "Disconnect", Disconnect);
	ENTRYTC(*list, "Downloadfile", Downloadfile);
	ENTRYTC(*list, "DownloadfileHttp", DownloadfileHttp);
	ENTRYTC(*list, "UploadFile", UploadFile);
	ENTRYTC(*list, "IperfServerRemote", IperfServerRemote);
	ENTRYTC(*list, "ServerStartIperf", ServerStartIperf);
	ENTRYTC(*list, "ServerStartDownload", ServerStartDownload);
	ENTRYTC(*list, "Idle", Idle);
        ENTRYTC(*list, "SwitchWlan", SwitchWlan);
        ENTRYTC(*list, "RemoveService", RemoveService);
        ENTRYTC(*list, "SetTethering", SetTethering);

	return 0;
}

