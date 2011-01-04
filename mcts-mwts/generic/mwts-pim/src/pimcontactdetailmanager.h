/* PimContactDetailManager.h -- Header for PimPimContactDetailManager class
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

#ifndef PimContactDetailManager_H
#define PimContactDetailManager_H

// Includes
#include <MwtsCommon>
#include <QContactDetail>
#include <QContactAddress>
#include <QContactAnniversary>
#include <QContactAvatar>
#include <QContactBirthday>
#include <QContactEmailAddress>
#include <QContactFamily>
#include <QContactGender>
#include <QContactGeoLocation>
#include <QContactGlobalPresence>
#include <QContactGuid>
#include <QContactName>
#include <QContactNickname>
#include <QContactNote>
#include <QContactOnlineAccount>
#include <QContactOrganization>
#include <QContactPhoneNumber>
#include <QContactPresence>
#include <QContactRingtone>
#include <QContactSyncTarget>
#include <QContactTag>
#include <QContactThumbnail>
#include <QContactTimestamp>
#include <QContactType>
#include <QContactUrl>

// namespace
QTM_USE_NAMESPACE


/**
 * Helper class for handling contact details
 */
class PimContactDetailManager
{

public: // constructor & destructor

    /**
     * PimContactDetailManager constructor
     */
    PimContactDetailManager(MwtsConfig &dFetcher);

    /**
     * PimContactDetailManager destructor
     */
    ~PimContactDetailManager();

public: // new functions

    /**
     * Adds default contact details to an array. Information is fetched via MwtsConfig.
     * Returns true, if data is successfully added
     */
    bool addDefaultContactDetails(QContact &target);

    /**
     * Initializes contact detail variables via MwtsConfig
     */
    void initializeContactData();

    /**
     * Verifies contact details. if valueSetting is true, timestamp and quid are not tested because database modifies those
     */
    void verifyContactDetails(QContact& contact, bool valueSetting);

private: // new functions

    /**
     * Saves details to contact
     */
    void saveDetailsToContact(QContact &target);

    /**
     * Verifying function for adding data to detail
     */
    void verifyDetailAddition(QString detailValueName, QContactDetail detail, bool success);

    /**
     * Data addition functions for largest details
     */
    QContactAddress address();
    QContactGeoLocation geoLocation();
    QContactGlobalPresence globalPrecence();
    QContactName contactName();
    QContactOrganization organization();
    QContactPresence precence();
    QContactAnniversary anniversary();
    QContactFamily family();
    QContactOnlineAccount onlineAccount();
    QContactTimestamp timeStamp();

private: // data

    // for fetching data
    MwtsConfig &m_detailFetcher;

    // return value for details
    bool m_returnValue;

    // detail array
    QList<QContactDetail> m_contactDetails;

    // Contact data
    float m_geoAccuracy;
    float m_geoAltitude;
    float m_geoAltAccuracy;
    float m_geoHeading;
    float m_geoLat;
    float m_geoLon;
    float m_geoSpeed;
    QString m_country;
    QString m_locality;
    QString m_postcode;
    QString m_pobox;
    QString m_region;
    QString m_street;
    QString m_anniversary;
    QDateTime m_date;
    QString m_avatar_url;
    QString m_emailAddress;
    QString m_childName;
    QString m_spouseName;
    QString m_gender;
    QString m_geoLabel;
    QString m_precenceMsg;
    QString m_precenceNick;
    QContactPresence::PresenceState m_precenceState;
    QString m_precenceImgUrl;
    QString m_precenceTxt;
    QString m_guid;
    QString m_nameLabel;
    QString m_nameFirst;
    QString m_nameLast;
    QString m_nameMiddle;
    QString m_namePrefix;
    QString m_nameSuffix;
    QString m_nameNick;
    QString m_note;
    QString m_accUri;
    QString m_serviceProd;
    QString m_assistant;
    QString m_department;
    QString m_location;
    QString m_organizationName;
    QString m_role;
    QString m_title;
    QString m_phoneNumber;
    QString m_ringTone;
    QString m_syncTarget;
    QString m_tag;
    QString m_thumbUri;
    QString m_type;
    QString m_contactUrl;
};

#endif // PimContactDetailManager_H
