/* BluetoothCommonTypes.h -- Header for common types of bluetooth library
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

#ifndef __BLUETOOTHCOMMONTYPES_H__
#define __BLUETOOTHCOMMONTYPES_H__

/**
 *	Define test timeout
 */
#define TEST_TIMEOUT 30000

/**
 * 	Common agent path used by all our agents
 * */
#define DBUS_AGENT_PATH "/test/agent"

/**
 *	Defines the role of service.
 * */
enum ServiceRole {
	UndefinedRole = 0,
	Client,
	Server
};

/**
 * 	Defines transfer protocol used
 * */
enum TransferType {
	UndefinedType = 0,
	L2CAP,
	RfComm,
	RfCommTTY    // Serial type
};


#endif /*__BLUETOOTHCOMMONTYPES_H__*/
