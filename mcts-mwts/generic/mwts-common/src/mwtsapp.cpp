/* mwtsapp.cpp --
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
#include "mwtsapp.h"
#include <MwtsCommon>
#include <QMainWindow>

char args[100]="dummy_application_name";
char* dummy_args[1];
int dummy_argc=1;

/** Static members */

MwtsApp* MwtsApp::inst = NULL;
bool MwtsApp::s_bUi=false;

/** Static method for getting pointer to application singleton */
MwtsApp* MwtsApp::instance()
{
	dummy_args[0]=args;	
	if(!inst)
		inst=new MwtsApp;
	return inst;
}
/** Static method for enabling or disabling UI usage. This
  must be called before calling instance() the first time!*/
void MwtsApp::EnableUI(bool bEnable)
{
	s_bUi=bEnable;
}

MwtsApp::MwtsApp() : QApplication(dummy_argc, dummy_args, s_bUi)
{
	MWTS_ENTER;
	if(s_bUi)
	{
		qDebug() << "Creating main window for GUI";
		g_pMainWindow=new QMainWindow();
		g_pMainWindow->resize(400, 300);
		g_pMainWindow->show();
		qDebug() << "Main window succesfully created";
	}
	MWTS_LEAVE;
}

MwtsApp::~MwtsApp()
{
	// No log available at this point!
	MWTS_ENTER;
	if(g_pMainWindow)
	{
		delete g_pMainWindow;
		g_pMainWindow=NULL;
	}

	MWTS_LEAVE;
}
