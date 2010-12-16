/* mwtsradio.cpp -- radio access mode change
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

#include "mwtsradio.h"
#include <MwtsCommon>


/** Singleton implementation */
MwtsRadio* MwtsRadio::inst = NULL;
MwtsRadio* MwtsRadio::instance()
{
	if(!inst)
		inst=new MwtsRadio();
	return inst;		
}

#ifdef OFONO_QT_EXISTS

MwtsRadio::MwtsRadio()
{
	m_radioSettings = new OfonoRadioSettings(OfonoModem::AutomaticSelect, QString(), NULL);
}

/** Changes radio access technology mode*/
bool MwtsRadio::ChangeMode(int mode)
{
	MWTS_ENTER;

	QString sMode;

	switch (mode)
	{
	case RADIO_ANY:
		sMode="any";
		break;
	case RADIO_GSM:
		sMode="gsm";
		break;
	case RADIO_UMTS:
		sMode="umts";
		break;
	case RADIO_LTE:
		sMode="lte";
		break;
	default:
		qCritical() << "Invalid radio technology preference";
		return false;
	}

        qDebug() << "Radio settings validity:" << m_radioSettings->isValid();
        qDebug() << "Current radio mode:" << m_radioSettings->technologyPreference();
	qDebug() << "Setting mode to: " << sMode;
        m_radioSettings->setTechnologyPreference(sMode);
	MWTS_LEAVE;
	return true;
}

/** 
  slot for notification when radio mode has really changed
*/
void MwtsRadio::onRadioModeChanged()
{
	qDebug() << "Radio mode changed to:" << m_radioSettings->technologyPreference();
}






#else
// ----------------- DUMMY implementation ----------------

MwtsRadio::MwtsRadio()
{
	MWTS_ENTER;
}
bool MwtsRadio::ChangeMode(int mode)
{
	MWTS_ENTER;
	qWarning() << "Radio mode change not available due to missing ofono-qt";
	return true;
}
void MwtsRadio::onRadioModeChanged()
{
	MWTS_ENTER;
}
#endif


