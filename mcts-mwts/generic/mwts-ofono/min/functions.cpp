/*
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
#include "ofonotest.h"

#include <MwtsCommon>

// MIN's module information
const char *module_date = __DATE__;
const char *module_time = __TIME__;

// ofono-qt test class
OfonoTest test;

/**
  ChangePin
  Changes pin code
  Usage: ChangePin [pinType, oldPin, newPin]
  @param pinType, oldPin, newPin
  @return ENOERR
*/
LOCAL gint ChangePin(MinItemParser * item)
{
    MWTS_ENTER;

    gchar* mode = NULL;
    bool ret;


    g_free(mode);
}

/**
  Test function list for MIN
  @return ENOERR
*/
gint ts_get_test_cases( DLList** list )
{
    // declare common functions like Init, Close..
    MwtsMin::DeclareFunctions(list);

    ENTRYTC(*list,"ChangePin", ChangePin);

    return ENOERR;

}
