/* versit.h -- Header for Versit class
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


#ifndef VERSIT_H
#define VERSIT_H

// Includes
#include <qmobilityglobal.h>
#include <QContact>

// namespace
QTM_USE_NAMESPACE


/**
  * Versit class. Provides importing / exporting functionalities
  */
class Versit
{
public: // constructor & destructor

    /**
     * Versit constructor
     */
    Versit();

    /**
     * Versit constructor
     */
    ~Versit();

public: // new functions

    /**
     * Imports contacts from vCard.
     * returns true if success and imported contact(s) in the list
     */
     bool ImportContactsFromVcard(QList<QContact>& contacts, QString vCardFile);

    /**
     * Exports vCard from contact and verifies
     * returns true if success
     */
     bool ExportContactsToVcard(QList<QContact>& contacts);

    /**
     * Imports calendar item from vCard. Not supported yet
     */
     bool ImportCalendarItemFromIcalendar() { return false; }

    /**
      * Exports iCalendar from calendar item. Not supported yet
      */
     bool ExportCalendarItemToIcalendar() { return false; }

private: // data

     // vCard data
     QByteArray m_inputVCard;
};

#endif // VERSIT_H
