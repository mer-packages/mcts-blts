/* PimItem.h -- Header for PimItem class
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

#ifndef PIMITEM_H
#define PIMITEM_H

// Includes
#include <QOrganizerItemId>
#include <QContact>

// namespace
QTM_USE_NAMESPACE

/**
  * Helper class for storing calendar items and contacts
  */
class PimItem {

public: // constructor

    PimItem(QOrganizerItemId iId=QOrganizerItemId(), QContactLocalId cId=-1, QString name="default", int iType=-1)
    {
        itemLocalId = iId;
        contactLocalId = cId;
        dataStoreName = name;
        itemType = iType;
    }

public: // data

    int itemType;
    QString dataStoreName;
    QOrganizerItemId itemLocalId;
    QContactLocalId contactLocalId;
};

#endif // PIMITEM_H
