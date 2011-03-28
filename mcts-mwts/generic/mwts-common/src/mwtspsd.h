/* mwtsradio.h -- radio access mode change
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

#ifndef _INCLUDED_MWTS_PSD_H
#define _INCLUDED_MWTS_PSD_H

/** Class for handling radio access technology */

#define RAT_UMTS	1
#define RAT_GSM		2
#define RAT_DUAL	3

class MwtsPsd
{
public:
	static bool ChangeMode(int mode);
	static bool Connect();
};


#endif // #ifndef _INCLUDED_MWTS_RAT_H
