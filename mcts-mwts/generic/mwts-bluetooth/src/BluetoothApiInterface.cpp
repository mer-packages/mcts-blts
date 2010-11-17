/* BluetoothApiInterface.cpp -- Source file for Bluetooth Api Interface
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

#include "BluetoothApiInterface.h"

BluetoothApiInterface::BluetoothApiInterface(QObject *parent) :
    QObject(parent),m_bIsClient(true),m_bIsHeadset(false)
{
    MWTS_ENTER;
    MWTS_LEAVE;
}

/**
*   @brief      Sets host-mac and pin code
*   @param      QString     host, remote mac adress
*   @param      QString     pin, pairing code
*   @param      bool        isClient, role of this device
*   @param      bool        isHeadset, type of this device (whether is audio capable)
*/
void BluetoothApiInterface::SetSettings(const QString& host, const QString& pin, bool isClient,bool isHeadset)
{
    MWTS_ENTER;
    m_sHost = host;
    m_sPin = pin;
    m_bIsClient = isClient;
    m_bIsHeadset = isHeadset;
    MWTS_LEAVE;
}

/**
*   @brief      Init settings for the bluetooth API
*/
bool BluetoothApiInterface::Init()
{
    MWTS_ENTER;
    MWTS_LEAVE;
    return true;
}

/**
*   @brief      Uninit settings for the bluetooth API
*/
bool BluetoothApiInterface::Uninit()
{
    MWTS_ENTER;
    MWTS_LEAVE;
    return true;
}

/**
*   @brief  Get the device mac address
*   @return const QString&,     the mac address
*/
QString& BluetoothApiInterface::GetAddress()
{
    MWTS_ENTER;
    MWTS_LEAVE;
    return m_sAddress;
}
