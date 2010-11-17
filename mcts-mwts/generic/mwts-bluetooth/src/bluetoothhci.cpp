/* bluetoothci.cpp -- Source file for Bluetooth Hci test class
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

#include "bluetoothhci.h"
#include "stable.h"
#include <sys/ioctl.h>
#include <errno.h>

#define HCI_TIMEOUT 30000

BluetoothHci::BluetoothHci(QObject *parent) :
    BluetoothApiInterface(parent), m_pBdaddr(0)
{
    m_iDevID = 0;
    m_iDevSock = 0;
}


BluetoothHci::~BluetoothHci()
{
    if(m_iDevSock >= 0)
    {
        close(m_iDevSock);
        m_iDevSock = -1;
    }
    delete m_pBdaddr;
}

/**
*   @brief  Initializes the hci
*/
bool BluetoothHci::Init()
{
    MWTS_ENTER;
    bool bret = true;
    bool setPowerOn = false;

    // Get default bt adapter
    m_iDevID = hci_get_route(NULL);

    qDebug() << "hci_get_route returned " << m_iDevID;

    /*
     * If no bt device found. (power off probably)
     * Then use the default one by powering it up.
     */
    if(m_iDevID < 0)
    {
        qWarning() << "Using default device 0";
        m_iDevID = 0;
        setPowerOn = true;
    }

    // Open BT adapter
    m_iDevSock = hci_open_dev( m_iDevID );

    qDebug() << "hci_open_dev returned " << m_iDevSock;

    // Error checking
    if (m_iDevSock < 0)
    {
        qWarning() << "Failed to open device";
        qWarning() << "Trying alternative way to get socket to device";

        // Try to open raw socket
        m_iDevSock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);

        qDebug() << "socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI) returned " << m_iDevSock;

        if (m_iDevSock < 0)
        {
            qCritical() << "Failed to create bluetooth hci socket";
            qCritical() << "Error: " << strerror(errno);
            bret = false;
        }
    }

    // set device up
    if(setPowerOn)
    {
        this->SetPowerMode(true);
    }

    MWTS_LEAVE;
    return bret;
}

/**
*   @brief  Uninitialize hci api
*/
bool BluetoothHci::Uninit()
{
    if(m_iDevSock >= 0)
    {
        close(m_iDevSock);
        m_iDevSock = -1;
    }
}

/**
*   @brief      Sets host-mac and pin code. Inherited function
*   @param      QString     host, remote mac adress
*   @param      QString     pin, pairing code
*/
void BluetoothHci::SetSettings(const QString& host, const QString& pin, bool isClient,bool isHeadset)
{
    MWTS_ENTER;
    BluetoothApiInterface::SetSettings(host,pin,isClient,isHeadset);
    if(!m_sHost.isEmpty())
    {
        //create bdaddr
        m_pBdaddr = new bdaddr_t;

        const char* tmp = m_sHost.toLatin1().constData();
        int err = ::str2ba(tmp,m_pBdaddr);          //convert from string to bluetooth address

        if(err <0)
            qWarning() << "failed to convert from string to bluetooth address";

        //verify bdaddr
        qDebug() << "Host in string: " << m_sHost;
        qDebug() << "Host in bdaddr: " << m_pBdaddr->b[0] << ":"<< m_pBdaddr->b[1] << ":" << m_pBdaddr->b[2]<<":"<<m_pBdaddr->b[3]<<":"<<m_pBdaddr->b[4]<<":"<<m_pBdaddr->b[5];
        qDebug() << "HOX! the hexa -> int conversion has taken place";
    }
    MWTS_LEAVE;
}


/**
 * Set BT device's scan mode
 * @param mode  Scan mode
 * @return      true if succeeded, otherwise false
 */
bool BluetoothHci::SetScanMode(ScanMode mode)
{
    MWTS_ENTER;
    bool bret = true;
    struct hci_dev_req dev_req;

    // Set device id
    dev_req.dev_id = this->m_iDevID;
    // Set device mode
    dev_req.dev_opt = mode;

    // Change scan mode
    if ( ioctl(m_iDevSock, HCISETSCAN, &dev_req) < 0 )
    {
        qCritical() << "Cannot set scan mode on hci " << this->m_iDevID;
        qCritical() << "Error: " << strerror(errno);
        errno = 0;
        bret = false;
    }
    MWTS_LEAVE;
    return bret;
}

/**
 * Set BT device's power mode
 * @param mode  Power mode, on = true, off = false
 * @return      true if succeeded, otherwise false
 */
bool BluetoothHci::SetPowerMode(bool isOn)
{
    MWTS_ENTER;
    bool bret = true;
    int ioctlmode = 0;

    ioctlmode = (isOn) ? HCIDEVUP : HCIDEVDOWN;

    /* Change power mode */
    if (ioctl(m_iDevSock, ioctlmode, m_iDevID) < 0)
    {
        qCritical() << "Cannot set power mode on hci " << this->m_iDevID;
        qCritical() << "Error: " << strerror(errno);
        errno = 0;
        bret = false;
    }

    MWTS_LEAVE;
    return bret;
}

/**
 * Inquiries available BT devices
 * @param inq_info  Out value for inquiry info
 * @return Number of devices found
 */
int BluetoothHci::Inquiry(inquiry_info** inq_info)
{
    MWTS_ENTER;
    int max_rsp, len, flags, num_rsp;
    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;

    /* Frees old data if set */
    if( *inq_info != NULL)
    {
        free(*inq_info);
        *inq_info = NULL;
    }

    *inq_info = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    // Make inquiry
    num_rsp = hci_inquiry(m_iDevID, len, max_rsp, NULL, inq_info, flags);

    qDebug() <<  num_rsp << " number of devices found!";

    if( num_rsp < 0 )
    {
        qCritical() << "hci_inquiry failure";
        qCritical() << "Error: " << num_rsp;
        free(*inq_info);
        *inq_info = NULL;
    }
    MWTS_LEAVE;
    return num_rsp;
}

/**
 * Make BT device scan and resolve names
 * @return  true if succeeded, otherwise false
 */
bool BluetoothHci::Scan()
{
    MWTS_ENTER;
    inquiry_info *inq_info = NULL;
    bool bret = true;
    int ret = 0, num_rsp;
    const int device_name_maxlen = 255;
    char addr[32] = { 0 };
    char name[device_name_maxlen+1] = { 0 };
    char text[device_name_maxlen+1] = { 0 };


    // Inquiry BT devices
    num_rsp = this->Inquiry(&inq_info);

    // If less than zero, error or no devices found
    if (num_rsp > 0)
    {
        for (int i = 0; i < num_rsp; i++)
        {
            ::ba2str(&(inq_info+i)->bdaddr, addr);
            ::memset(name, 0, device_name_maxlen);

            // Get remote device name
            ret = hci_read_remote_name(m_iDevSock, &(inq_info+i)->bdaddr, device_name_maxlen, name, HCI_TIMEOUT);

            qDebug() << "hci_read_remote_name returned " << ret;

            if(ret < 0)
            {
                qWarning() << "Timeout occurred while trying to read remote device name";
                strcpy(name, "[unknown, timeout]");
            }
            sprintf(text, "%.2f\t\t%s\t%s", g_pTime->elapsed()/1000.0f, addr, name);

            qDebug() << QString::fromAscii(text);
            g_pResult->Write(QString::fromAscii(text));
        }
    }
    else
    {
        bret = false;
    }

    free(inq_info);
    MWTS_LEAVE;
    return bret;
}

bool BluetoothHci::Connect()
{
    MWTS_ENTER;
    qCritical() << "This function is DEPRECATED, please use the correspondent function from dbus Api.";
    MWTS_LEAVE;
    return false;
}

bool BluetoothHci::Disconnect()
{
    MWTS_ENTER;
    qCritical() << "This function is DEPRECATED, please use the correspondent function from dbus Api.";
    MWTS_LEAVE;
    return false;
}

bool BluetoothHci::Remove()
{
    MWTS_ENTER;
    qCritical() << "This function is DEPRECATED, please use the correspondent function from dbus Api.";
    MWTS_LEAVE;
    return false;
}
