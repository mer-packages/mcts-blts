/* UsbTest.h -- interface of the UsbTest-asset
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

#ifndef _INCLUDED_USB_TEST_H
#define _INCLUDED_USB_TEST_H

#include "stable.h"

#include <MwtsCommon>


class UsbTest : public MwtsTest
{
	Q_OBJECT
public:
	/**
	 * Constructor
	 */
	UsbTest();
	/**
	 * Destructor
	 */
	virtual ~UsbTest();

	/**
	 * Inherited from mwts-test
	 */
	void OnInitialize();
	/**
	 * Inherited from mwts-test
	 */
	void OnUninitialize();

	/**
	 * Make file transfer from src to dest
	 * @param a_destination	Destination path where to copy
	 * @param a_source		Source path from to copy
	 */
	void Transfer(const char* a_destination, const char* a_source);

	/**
	 * Get overall result
	 * @return true if success, false if fail
	 */
	inline bool GetStatus(){ return m_bResult; }

	/**
	 * Get send speed in MB/s
	 * @return send speed in MB/s
	 */
	inline float GetSendSpeed() { return m_fSendSpeed; }
	/**
	 * Get received speed in MB/s
	 * @return received speed in MB/s
	 */
	inline float GetReceiveSpeed() { return m_fReceiveSpeed; }

protected slots:

	/**
	 * Slots for QProcess signals
	 */
	void slotError(QProcess::ProcessError error);
	void slotFinished( int exitCode, QProcess::ExitStatus exitStatus );
	void slotStateChanged  ( QProcess::ProcessState newState );
	void slotReadStandardOutput();

private:
	QProcess* m_pProcess;

	/**
	 * Parse SCP stdout and store speeds to local variables
	 */
	void ParseOutput();

	QByteArray m_baStdErr;
	// Overall result
	bool m_bResult;

	// Variables for storing speed
	float m_fSendSpeed;
	float m_fReceiveSpeed;
};

#endif //#ifndef _INCLUDED_USB_TEST_H

