/* mwtsconfig.cpp --
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
#include "mwtsconfig.h"
#include <MwtsCommon>

MwtsConfig::MwtsConfig()
: QSettings(QSettings::SystemScope, g_pTest->Name())
{

	if(g_pTest->Platform()!="")
	{
		g_pPlatformConfig = new QSettings(QSettings::SystemScope, g_pTest->Name()+'-'+ g_pTest->Platform());
	}
        else
        {
               g_pPlatformConfig = NULL;
        }

}

MwtsConfig::~MwtsConfig()
{
    if (g_pPlatformConfig)
    {
       delete g_pPlatformConfig;
       g_pPlatformConfig = NULL;
    }
}

QVariant MwtsConfig::value ( const QString & key, const QVariant & defaultValue/* = QVariant()*/ ) const
{
    // Get fefault value from generic platform implementation
    QVariant my_value = QSettings::value(key, defaultValue);
    if (g_pPlatformConfig)
    {
       // try override it by platform value, if not found, then generic value remains
       my_value = g_pPlatformConfig->value(key,my_value);
    }

    return my_value;

}


