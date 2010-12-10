/* functions.cpp -- Test assets MIN functions
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
#include "BluetoothTest.h"
#include "BluetoothCommonTypes.h"
#include "bluetoothhci.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

TransferType ParseSocketType (char* sockettype_str );
ServiceRole ParseSocketRole (char* socketrole_str );


// Test class
BluetoothTest Test;

/**
 * PowerMode
 * Min scripter function for bluetooth devices power mode
 *      -mode, on/off
 * @param item 	scripter parameters
 */
LOCAL int PowerMode ( MinItemParser* item)
{
    int ret = -1;
    char *mode_str = NULL;
    bool powerMode;
    char *tag = NULL;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "mode") == 0)
        {
            ret = mip_get_next_string( item, &mode_str);
            if (ret != ENOERR) return EINVAL;
        }
    }

    if( mode_str )
    {
        if(strcmp(mode_str, "true") == 0 || strcmp(mode_str, "on") == 0)
        {
            qDebug("Power mode: On");
            powerMode = true;
        }
        else if(strcmp(mode_str, "false") == 0 || strcmp(mode_str, "off") == 0)
        {
            qDebug("Power mode: Off");
            powerMode = false;
        }
        else
        {
            qDebug("Unknown power mode, using default: On");
            powerMode = true;
        }
    }
    else
    {
        qDebug("Power mode not set, using default: On");
        powerMode = true;
    }

    if(Test.SetPowerMode(powerMode))
        ret = 0;
    else
        ret = -1;

    qDebug() << "Test returned == " << ret;

    if(mode_str)
    {
        free(mode_str);
        mode_str = NULL;
    }
    if(tag)
    {
        free(tag);
        tag = NULL;
    }
    return ret;
}

/**
 * PowerLatency
 * Min scripter function for bluetooth devices power mode change latency
 *      -mode, on/off
 * @param item 	scripter parameters
 */
LOCAL int PowerLatency ( MinItemParser* item)
{
    int ret = -1;
    char *mode_str = NULL;
    bool powerMode;
    char *tag = NULL;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "mode") == 0)
        {
            ret = mip_get_next_string( item, &mode_str);
            if (ret != ENOERR) return EINVAL;
        }
    }

    if( mode_str )
    {
        if(strcmp(mode_str, "true") == 0 || strcmp(mode_str, "on") == 0)
        {
            qDebug("PowerLatency mode: On");
            powerMode = true;
        }
        else if(strcmp(mode_str, "false") == 0 || strcmp(mode_str, "off") == 0)
        {
            qDebug("PowerLatency mode: Off");
            powerMode = false;
        }
        else
        {
            qDebug("Unknown power mode, using default: On");
            powerMode = true;
        }
    }
    else
    {
        qDebug("Power mode not set, using default: On");
        powerMode = true;
    }

    if(Test.MeasurePowerLatency(powerMode))
        ret = 0;
    else
        ret = -1;

    qDebug() << "Test returned == " << ret;

    if(mode_str)
    {
        free(mode_str);
        mode_str = NULL;
    }
    if(tag)
    {
        free(tag);
        tag = NULL;
    }
    return ret;
}

/**
 * ScanMode
 * Min scripter function for bluetooth scan mode
 * @param item  scripter parameters
 *              - Mode, off, discoverable or pairable
 */
LOCAL int ScanMode ( MinItemParser* item )
{
    int ret;
    char *mode_str = NULL;
    char *tag = NULL;
    BluetoothHci::ScanMode scanMode;

    // Parse parameters
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "mode") == 0)
        {
            ret = mip_get_next_string( item, &mode_str);
            if (ret != ENOERR) return EINVAL;
        }
    }

    // Set default mode if not set
    if( !mode_str)
    {
        qDebug() << "Setting default mode to discoverable";
        mode_str = strdup("discoverable");
    }

    // Get mode
    if(strcmp(mode_str, "off") == 0)
    {
        qDebug() << "Mode set to off";
        scanMode = BluetoothHci::ScanModeInvisible;
    }
    else if(strcmp(mode_str, "discoverable") == 0)
    {
        qDebug() << "Mode set to discoverable";
        scanMode = BluetoothHci::ScanModeInquiry;
    }
    else if(strcmp(mode_str, "pairable") == 0)
    {
        qDebug() << "Mode set to pairable";
        scanMode = BluetoothHci::ScanModeBoth;
    }
    else
    {
        qDebug() << "Mode set to pairable";
        scanMode = BluetoothHci::ScanModeBoth;
    }

    ret = Test.SetScanMode(scanMode);
    qDebug() << "Test returned == " << ret;


    if(mode_str)
    {
        free(mode_str);
        mode_str = NULL;
    }
    if(tag)
    {
        free(tag);
        tag = NULL;
    }
    if (ret)
        return 0;
    else
        return 1;
}


/**
 * Scan
 * Min scripter function for bluetooth device scan
 * @param item 	scripter parameters
 */
LOCAL int Scan ( MinItemParser* item )
{
    /* Helper */
    int ret;

    ret = Test.Scan();
    qDebug() << "Test returned == " << ret;

    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * Transfer
 * Min scripter function for bluetooth server socker creation
 * Parameters:
 *      -role
 *      -bytes
 *      -buffsize
 *      -socket-type
 *      -time
 * @param item 	scripter parameters
 */
LOCAL int Transfer( MinItemParser* item )
{
    /* Helper */
    int ret;

    char *sockettype_str = NULL;
    char *socketrole_str = NULL;
    char *host_str = NULL;
    long int bytes = -1;
    long int time= -1;
    int buffsize = -1;
    char *tag = NULL;
    TransferType sockettype = UndefinedType;
    ServiceRole socketrole = UndefinedRole;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "role") == 0)
        {
            ret = mip_get_next_string( item, &socketrole_str);
            if (ret != ENOERR) return EINVAL;
            /* Check Role */
            socketrole = ParseSocketRole(socketrole_str);
        }
        else
        if (tag && strcmp (tag, "socket-type") == 0)
        {
            ret = mip_get_next_string( item, &sockettype_str);
            if (ret != ENOERR) return EINVAL;
            /* Check SocketType */
            sockettype = ParseSocketType(sockettype_str);
        }
        else
        if (tag && strcmp (tag, "host") == 0)
        {
            ret = mip_get_next_string( item, &host_str);
        }
        else
        if (tag && strcmp (tag, "bytes") == 0)
        {
            ret = mip_get_next_string(item, &tag);
            if (ret == ENOERR)
                 sscanf(tag,"%ld", &bytes);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "buffsize") == 0)
        {
            ret = mip_get_next_int( item, &buffsize);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "time") == 0)
        {
            ret = mip_get_next_string(item, &tag);
            if (ret == ENOERR)
                 sscanf(tag,"%ld", &time);
            if (ret != ENOERR) return EINVAL;
        }
        else
        {
            return EINVAL;
        }
    }


    if (( socketrole != UndefinedRole) && (bytes > 0L) && (buffsize > 0))
    {
        ret = Test.Transfer(socketrole,sockettype, host_str, bytes, buffsize, time);
        qDebug() << "BluetoothTest returned == " << ret;
    }
    else
    {
        return EINVAL;
    }


    if(sockettype_str)
    {
        free(sockettype_str);
        sockettype_str = NULL;
    }
    if(host_str)
    {
        free(host_str);
        host_str = NULL;
    }
    if(tag)
    {
        free(tag);
        tag = NULL;
    }
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * ParseSocketType
 * Parse TransferType parameter
 * @param sockettype_str 	scripter parameters
 */
TransferType ParseSocketType (char* sockettype_str )
{
    TransferType sockettype = UndefinedType;

    if(sockettype_str)
    {
        if(strcmp(sockettype_str,"L2CAP") == 0)
        {
            sockettype = L2CAP;
        }
        else
        if(strcmp(sockettype_str,"RfComm") == 0)
        {
            sockettype = RfComm;
        }
        else
        if(strcmp(sockettype_str,"RfCommTTY") == 0)
        {
            sockettype = RfCommTTY;
        }
    }
    return sockettype;
}

/**
 * ParseSocketRole
 * Parse SocketRole parameter
 * @param socketrole_str 	scripter parameters
 */
ServiceRole ParseSocketRole (char* socketrole_str )
{
	ServiceRole socketrole = UndefinedRole;

	qDebug() << "ROLE:" << socketrole_str;

    if(socketrole_str)
    {
        if(strcmp(socketrole_str,"client") == 0)
        {
            socketrole = Client;
            qDebug() << "Role to client.";
        }
        else
        if(strcmp(socketrole_str,"server") == 0)
        {
            socketrole = Server;
            qDebug() << "Role to server";
        }
    }

    return socketrole;
}

/**
*   Device
*   Parameters:
*       -connect == 0
*       -disconnect == 1
*       -remove == 2
*       -pin
*       -host
*       -client == true
*       -server == false
*/
LOCAL int Device( MinItemParser* item )
{
    /* Helper */
    int ret = ENOERR;
    char *tag = 0;

    char *host_str = 0;
    char *pin = 0;
    int   command = -1;
    bool  isClient = true;
    bool  isHeadset = false;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "connect") == 0)
        {
            command = 0;
        }
        else
        if (tag && strcmp (tag, "disconnect") == 0)
        {
            command = 1;
        }
        else
        if (tag && strcmp (tag, "remove") == 0)
        {
            command = 2;
        }
        else
        if (tag && strcmp (tag, "host") == 0)
        {
            ret = mip_get_next_string( item, &host_str);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "pin") == 0)
        {
            ret = mip_get_next_string( item, &pin);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "client") == 0)
        {
            isClient = true;
        }        
        else
        if (tag && strcmp (tag, "server") == 0)
        {
            isClient = false;
        }
        else
        if (tag && strcmp (tag, "headset") == 0)
        {
            isHeadset = true;
        }
        else
        {
            return EINVAL;
        }
    }

    if(command >= 0)
    {
        //call BluetoothTest function
        ret = Test.Device(command, host_str,pin,isClient,isHeadset);
        qDebug() << "Test returned: " << ret;
    }
    else
        return EINVAL;


    if(tag)
    {
        free(tag);
        tag = 0;
    }
    if(host_str)
    {
        free(host_str);
        host_str = 0;
    }
    if(pin)
    {
        free(pin);
        pin = 0;
    }

    return ret;
}

/**
*   SetApi
*   Parameters:
*       -dbus == 1
*       -c == 0
*/
LOCAL int SetApi( MinItemParser* item )
{
    /* Helper */
    int ret = ENOERR;
    char *tag = 0;

    int api = -1;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "dbus") == 0)
        {
            api = 1;
        }
        else
        if (tag && strcmp (tag, "c") == 0)
        {
            api = 0;
        }
        else
        {
            return EINVAL;
        }
    }

    if(api >= 0)
    {
        ret = Test.SetApi(api);
        qDebug() << "Test returned: " << ret;
    }
    else
        return EINVAL;


    if(tag)
    {
        free(tag);
        tag = 0;
    }

    return ret;
}


/**
*   FileTransfer
*   Parameters:
*       -host
*       -filename
*       -role
*/
LOCAL int FileTransfer( MinItemParser* item )
{
    /* Helper */
    int ret = ENOERR;
    char *tag = 0;

    char *filename_str = 0;
	char *host_str = 0;
	char *role_str = 0;
	ServiceRole role = UndefinedRole;

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "host") == 0)
        {
            ret = mip_get_next_string( item, &host_str);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "filename") == 0)
        {
            ret = mip_get_next_string( item, &filename_str);
            if (ret != ENOERR) return EINVAL;
        }
        else
        if (tag && strcmp (tag, "role") == 0)
        {
            ret = mip_get_next_string( item, &role_str);
            if (ret != ENOERR) return EINVAL;
            role = ParseSocketRole(role_str);
        }
        else
        {
            return EINVAL;
        }
    }

    /* transfer */
    if(filename_str != 0)
    {
        ret = Test.FileTransfer(filename_str,host_str,role);
        qDebug() << "Test returned: " << ret;
    }
    else
        return EINVAL;

    /* free resources */
    if(filename_str)
    {
        free(filename_str);
        filename_str = 0;
    }
    if(host_str)
    {
        free(host_str);
        host_str = 0;
    }
    if(role_str)
    {
        free(role_str);
        role_str = 0;
    }
    if(tag)
    {
        free(tag);
        tag = 0;
    }

    return ret;
}

/**
 * Sets Bluetooth adapter MAC address to Min scripter variable.
 * @param item  Scripter input
 *              - variable name, optional input
 * @return      0 or -1
 */
LOCAL int GetAddress ( MinItemParser* item )
{
    char* variablename = 0;
    int ret = ENOERR;
    char* tag = 0;

    /* User can give variable name as an input
     * Default variable name is macaddress
     */

    /* Parse parameters */
    while (mip_get_next_string(item, &tag) == ENOERR )
    {
        if (tag && strcmp (tag, "macaddress") == 0)
        {
            ret = mip_get_next_string( item, &variablename);
        }
        else
        {
            return EINVAL;
        }
    }

    const char* address = Test.GetAddress().toLatin1().constData();

    /* Set macaddress to scripter variable */
    if(!address)
        ret = SetLocalValue(variablename, address);
    else
        ret = EINVAL;

    if(variablename)
    {
        free(variablename);
        variablename = 0;
    }
    if(tag)
    {
        free(tag);
        tag = 0;
    }

    return ret;
}

/**
 * Function for MIN to gather our test case functions.
 * @param list  Functio pointer list, out
 * @return      ENOERR
 */
int ts_get_test_cases (DLList ** list)
{
    // declare common functions like Init, Close, SetTestTimeout ...
    MwtsMin::DeclareFunctions(list);

    ENTRYTC(*list,"Scan", Scan);
    ENTRYTC(*list,"ScanMode", ScanMode);
    ENTRYTC(*list,"PowerMode", PowerMode);
    ENTRYTC(*list,"PowerLatency", PowerLatency);
    ENTRYTC(*list,"Transfer", Transfer);
    ENTRYTC(*list,"Device", Device);
    ENTRYTC(*list,"SetApi", SetApi);
    ENTRYTC(*list,"FileTransfer", FileTransfer);
    ENTRYTC(*list,"GetAddress", GetAddress);

    return ENOERR;
}

