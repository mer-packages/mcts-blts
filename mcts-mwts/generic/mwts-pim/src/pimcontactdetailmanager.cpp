/* PimContactDetailManager.cpp -- source code for PimPimContactDetailManager class
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


// class header
#include "pimcontactdetailmanager.h"


/**
 * PimContactDetailManager constructor
 */
PimContactDetailManager::PimContactDetailManager(MwtsConfig &dFetcher)
    :m_detailFetcher(dFetcher)
{
    MWTS_ENTER;
}

/**
 * PimContactDetailManager destructor
 */
PimContactDetailManager::~PimContactDetailManager()
{
    MWTS_ENTER;

    m_contactDetails.clear();
}

/**
 * getDefaultm_contactDetails function
 */
bool PimContactDetailManager::addDefaultContactDetails(QContact &target)
{
    MWTS_ENTER;

    m_returnValue = true;

    // add and verify first the biggest details
    // address
    m_contactDetails.append(address());
    // geo location
    m_contactDetails.append(geoLocation());
    // contact name
    m_contactDetails.append(contactName());
    // organization
    m_contactDetails.append(organization());
    // precence
    m_contactDetails.append(precence());
    // global precence
    m_contactDetails.append(globalPrecence());
    // anniversary
    m_contactDetails.append(anniversary());
    // contact family
    m_contactDetails.append(family());
    // online account
    m_contactDetails.append(onlineAccount());
    // time stamp
    m_contactDetails.append(timeStamp());


    QContactAvatar avatar;
    avatar.setImageUrl(QUrl(m_avatar_url));
    m_contactDetails.append(avatar);

    qDebug()<<"DATE: "<<m_date;
    QContactBirthday birthday;
    birthday.setDate(m_date.date());
    m_contactDetails.append(birthday);

    QContactEmailAddress emailAddress;
    emailAddress.setEmailAddress(m_emailAddress);
    m_contactDetails.append(emailAddress);

    QContactGender gender;
    gender.setGender(m_gender);
    m_contactDetails.append(gender);

    QContactGuid guid;
    guid.setGuid(m_guid);
    m_contactDetails.append(guid);

    QContactNickname nickName;
    nickName.setNickname(m_nameNick);
    m_contactDetails.append(nickName);

    QContactNote note;
    note.setNote(m_note);
    m_contactDetails.append(note);

    QContactPhoneNumber phoneNumber;
    phoneNumber.setNumber(m_phoneNumber);
    m_contactDetails.append(phoneNumber);

    QContactRingtone ringtone;
    ringtone.setAudioRingtoneUrl(QUrl(m_ringTone));
    m_contactDetails.append(ringtone);

    QContactSyncTarget syncTarget;
    syncTarget.setSyncTarget(m_syncTarget);
    m_contactDetails.append(syncTarget);

    QContactTag tag;
    tag.setTag(m_tag);
    m_contactDetails.append(tag);

    QContactThumbnail thumbnail;
    thumbnail.setThumbnail(QImage(m_thumbUri));
    m_contactDetails.append(thumbnail);

    QContactType type;
    type.setType(m_type);
    m_contactDetails.append(type);

    QContactUrl url;
    url.setUrl(m_contactUrl);
    m_contactDetails.append(url);

    // save details
    saveDetailsToContact(target);
    m_contactDetails.clear();

    // verify (true means that all set values are checked)
    verifyContactDetails(target,true);

    return m_returnValue;
}

/**
 * Verifies contact details
 */
void PimContactDetailManager::verifyContactDetails(QContact& contact, bool valueSetting)
{
     MWTS_ENTER;

     QContactName name = contact.detail<QContactName>();
     QContactAvatar avatar = contact.detail<QContactAvatar>();
     QContactBirthday birthday = contact.detail<QContactBirthday>();
     QContactEmailAddress emailAddress = contact.detail<QContactEmailAddress>();
     QContactGender gender = contact.detail<QContactGender>();
     QContactGuid guid = contact.detail<QContactGuid>();
     QContactNickname nickName = contact.detail<QContactNickname>();
     QContactNote note = contact.detail<QContactNote>();
     QContactPhoneNumber phoneNumber = contact.detail<QContactPhoneNumber>();
     QContactRingtone ringtone = contact.detail<QContactRingtone>();
     QContactSyncTarget syncTarget = contact.detail<QContactSyncTarget>();
     QContactTag tag = contact.detail<QContactTag>();
     QContactThumbnail thumbnail = contact.detail<QContactThumbnail>();
     QContactType type = contact.detail<QContactType>();
     QContactUrl url = contact.detail<QContactUrl>();
     QContactAddress address = contact.detail<QContactAddress>();
     QContactGeoLocation geoLocation = contact.detail<QContactGeoLocation>();
     QContactGlobalPresence globalPrecence = contact.detail<QContactGlobalPresence>();
     QContactOrganization organization = contact.detail<QContactOrganization>();
     QContactPresence precence = contact.detail<QContactPresence>();
     QContactAnniversary anniversary = contact.detail<QContactAnniversary>();
     QContactFamily contactFamily = contact.detail<QContactFamily>();
     QContactOnlineAccount onlineAccount = contact.detail<QContactOnlineAccount>();
     QContactTimestamp timeStamp = contact.detail<QContactTimestamp>();

     verifyDetailAddition("imageUrl",avatar,avatar.imageUrl()==QUrl(m_avatar_url));
     verifyDetailAddition("date",birthday,birthday.date()==m_date.date());
     verifyDetailAddition("emailAddress",emailAddress,emailAddress.emailAddress()==m_emailAddress);
     verifyDetailAddition("gender",gender,gender.gender()==m_gender);

     // Don't verify these after saving, the database modifies this
     if(valueSetting)
     {
        verifyDetailAddition("guid",guid,guid.guid()==m_guid);
     }

     verifyDetailAddition("nickname",nickName,nickName.nickname()==m_nameNick);
     verifyDetailAddition("note",note,note.note()==m_note);
     verifyDetailAddition("number",phoneNumber,phoneNumber.number()==m_phoneNumber);
     verifyDetailAddition("audioRingtoneUrl",ringtone,ringtone.audioRingtoneUrl()==m_ringTone);
     verifyDetailAddition("syncTarget",syncTarget,syncTarget.syncTarget()==m_syncTarget);
     verifyDetailAddition("tag",tag,tag.tag()==m_tag);
     verifyDetailAddition("thumbnail",thumbnail,thumbnail.thumbnail()==QImage(m_thumbUri));
     verifyDetailAddition("type",type,type.type()==m_type);
     verifyDetailAddition("url",url,url.url()==m_contactUrl);

      // address
     verifyDetailAddition("country",address,address.country()==m_country);
     verifyDetailAddition("locality",address,address.locality()==m_locality);
     verifyDetailAddition("postcode",address,address.postcode()==m_postcode);
     verifyDetailAddition("postOfficeBox",address,address.postOfficeBox()==m_pobox);
     verifyDetailAddition("region",address,address.region()==m_region);
     verifyDetailAddition("street",address,address.street()==m_street);

     // geo location
     verifyDetailAddition("accuracy",geoLocation,geoLocation.accuracy()==m_geoAccuracy);
     verifyDetailAddition("altitude",geoLocation,geoLocation.altitude()==m_geoAltitude);
     verifyDetailAddition("altitudeAccuracy",geoLocation,geoLocation.altitudeAccuracy()==m_geoAltAccuracy);
     verifyDetailAddition("heading",geoLocation,geoLocation.heading()==m_geoHeading);
     verifyDetailAddition("label",geoLocation,geoLocation.label()==m_geoLabel);
     verifyDetailAddition("latitude",geoLocation,geoLocation.latitude()==m_geoLat);
     verifyDetailAddition("longitude",geoLocation,geoLocation.longitude()==m_geoLon);
     verifyDetailAddition("speed",geoLocation,geoLocation.speed()==m_geoSpeed);
     verifyDetailAddition("timestamp",geoLocation,geoLocation.timestamp().date()==m_date.date());

     // global precence
     verifyDetailAddition("customMessage",globalPrecence,globalPrecence.customMessage()==m_precenceMsg);
     verifyDetailAddition("nickname",globalPrecence,globalPrecence.nickname()==m_precenceNick);
     verifyDetailAddition("presenceState",globalPrecence,globalPrecence.presenceState()==m_precenceState);
     verifyDetailAddition("presenceStateImageUrl",globalPrecence,globalPrecence.presenceStateImageUrl()==m_precenceImgUrl);
     verifyDetailAddition("presenceStateText",globalPrecence,globalPrecence.presenceStateText()==m_precenceTxt);
     verifyDetailAddition("timestamp",globalPrecence,globalPrecence.timestamp().date()==m_date.date());

     // name
     verifyDetailAddition("customLabel",name,name.customLabel()==m_nameLabel);
     verifyDetailAddition("firstName",name,name.firstName()==m_nameFirst);
     verifyDetailAddition("lastName",name,name.lastName()==m_nameLast);
     verifyDetailAddition("middleName",name,name.middleName()==m_nameMiddle);
     verifyDetailAddition("prefix",name,name.prefix()==m_namePrefix);
     verifyDetailAddition("suffix",name,name.suffix()==m_nameSuffix);

     // organization
     verifyDetailAddition("assistantName",organization,organization.assistantName()==m_assistant);
     if(organization.department().count()>0)
     {
        verifyDetailAddition("department",organization,organization.department()[0]==m_department);
     }
     verifyDetailAddition("location",organization,organization.location()==m_location);
     verifyDetailAddition("logoUrl",organization,organization.logoUrl()==m_thumbUri);
     verifyDetailAddition("name",organization,organization.name()==m_organizationName);
     verifyDetailAddition("role",organization,organization.role()==m_role);
     verifyDetailAddition("title",organization,organization.title()==m_title);

     // precence
     verifyDetailAddition("customMessage",precence,precence.customMessage()==m_precenceMsg);
     verifyDetailAddition("nickname",precence,precence.nickname()==m_precenceNick);
     verifyDetailAddition("presenceState",precence,precence.presenceState()==m_precenceState);
     verifyDetailAddition("presenceStateImageUrl",precence,precence.presenceStateImageUrl()==m_precenceImgUrl);
     verifyDetailAddition("presenceStateText",precence,precence.presenceStateText()==m_precenceTxt);
     verifyDetailAddition("timestamp",precence,precence.timestamp().date()==m_date.date());

     // anniversary
     verifyDetailAddition("originalDate",anniversary,anniversary.originalDate()==m_date.date());
     verifyDetailAddition("event",anniversary,anniversary.event()==m_anniversary);

     // contact family
     if(contactFamily.children().count()>0)
     {
         verifyDetailAddition("children",contactFamily,contactFamily.children()[0]==m_childName);
     }
     verifyDetailAddition("spouse",contactFamily,contactFamily.spouse()==m_spouseName);

     // online account
     verifyDetailAddition("accountUri",onlineAccount,onlineAccount.accountUri()==m_accUri);
     verifyDetailAddition("serviceProvider", onlineAccount,onlineAccount.serviceProvider()==m_serviceProd);

     //time stamp
     // Don't verify these after saving, the database modifies these
     if(valueSetting)
     {
        verifyDetailAddition("created",timeStamp,timeStamp.created().date()==m_date.date());
        verifyDetailAddition("lastModified",timeStamp,timeStamp.lastModified().date()==m_date.addDays(1).date());
     }
}

/**
  * addDetailsToContact function
  */
void PimContactDetailManager::saveDetailsToContact(QContact &target)
{
    MWTS_ENTER;

    bool result;
    QContactDetail detail;
    const int count = m_contactDetails.count();
    for(int i=0; i<count; ++i)
    {
        detail = m_contactDetails[i];
        result = target.saveDetail(&detail);
        if(!result)
        {
            qCritical()<<"Detail saving failed, detail: "<<detail.definitionName();
            m_returnValue = false;
            break;
        }
    }
}

/**
  * addAddress function
  */
QContactAddress PimContactDetailManager::address()
{
    MWTS_ENTER;

    QContactAddress address;
    address.setCountry(m_country);
    address.setLocality(m_locality);
    address.setPostcode(m_postcode);
    address.setPostOfficeBox(m_pobox);
    address.setRegion(m_region);
    address.setStreet(m_street);
    return address;
}

/**
  * addGeoLocation function
  */
QContactGeoLocation PimContactDetailManager::geoLocation()
{
    MWTS_ENTER;

    QContactGeoLocation geoLocation;
    geoLocation.setAccuracy(m_geoAccuracy);
    geoLocation.setAltitude(m_geoAltitude);
    geoLocation.setAltitudeAccuracy(m_geoAltAccuracy);
    geoLocation.setHeading(m_geoHeading);
    geoLocation.setLabel(m_geoLabel);
    geoLocation.setLatitude(m_geoLat);
    geoLocation.setLongitude(m_geoLon);
    geoLocation.setSpeed(m_geoSpeed);
    geoLocation.setTimestamp(m_date);
    return geoLocation;
}

/**
  * addGlobalPrecence function
  */
QContactGlobalPresence PimContactDetailManager::globalPrecence()
{
    MWTS_ENTER;

    QContactGlobalPresence globalPrecence;
    globalPrecence.setCustomMessage(m_precenceMsg);
    globalPrecence.setNickname(m_precenceNick);
    globalPrecence.setPresenceState(m_precenceState);
    globalPrecence.setPresenceStateImageUrl(QUrl(m_precenceImgUrl));
    globalPrecence.setPresenceStateText(m_precenceTxt);
    globalPrecence.setTimestamp(m_date);
    return globalPrecence;
}

/**
  * addContactName function
  */
QContactName PimContactDetailManager::contactName()
{
    MWTS_ENTER;

    QContactName name;
    name.setCustomLabel(m_nameLabel);
    name.setFirstName(m_nameFirst);
    name.setLastName(m_nameLast);
    name.setMiddleName(m_nameMiddle);
    name.setPrefix(m_namePrefix);
    name.setSuffix(m_nameSuffix);
    return name;
}

/**
  * addOrganization function
  */
QContactOrganization PimContactDetailManager::organization()
{
    MWTS_ENTER;

    QContactOrganization organization;
    organization.setAssistantName(m_assistant);
    QStringList depList;
    depList.append(m_department);
    organization.setDepartment(depList);
    organization.setLocation(m_location);
    organization.setLogoUrl(QUrl(m_thumbUri));
    organization.setName(m_organizationName);
    organization.setRole(m_role);
    organization.setTitle(m_title);
    return organization;
}

/**
  * addPrecence function
  */
QContactPresence PimContactDetailManager::precence()
{
    MWTS_ENTER;

    QContactPresence precence;
    precence.setCustomMessage(m_precenceMsg);
    precence.setNickname(m_precenceNick);
    precence.setPresenceState(m_precenceState);
    precence.setPresenceStateImageUrl(QUrl(m_precenceImgUrl));
    precence.setPresenceStateText(m_precenceTxt);
    precence.setTimestamp(m_date);
    return precence;
}

/**
  * addAnniversary function
  */
QContactAnniversary PimContactDetailManager::anniversary()
{
    MWTS_ENTER;

    QContactAnniversary anniversary;
    anniversary.setOriginalDate(m_date.date());
    anniversary.setEvent(m_anniversary);
    return anniversary;
}

/**
  * addFamily function
  */
QContactFamily PimContactDetailManager::family()
{
    MWTS_ENTER;

    QContactFamily contactFamily;
    QStringList list;
    list.append(m_childName);
    contactFamily.setChildren(list);
    contactFamily.setSpouse(m_spouseName);
    return contactFamily;
}

/**
  * addOnlineAccount function
  */
QContactOnlineAccount PimContactDetailManager::onlineAccount()
{
    MWTS_ENTER;

    QContactOnlineAccount onlineAccount;
    onlineAccount.setAccountUri(m_accUri);
    QStringList cabList;
    onlineAccount.setCapabilities(cabList);
    onlineAccount.setServiceProvider(m_serviceProd);
    return onlineAccount;
}

/**
  * addTimestamp function
  */
QContactTimestamp PimContactDetailManager::timeStamp()
{
    MWTS_ENTER;

    QContactTimestamp timeStamp;
    timeStamp.setCreated(m_date);
    timeStamp.setLastModified(m_date.addDays(1));
    return timeStamp;
}

/**
  * verifyDetailAddition function
  */
void PimContactDetailManager::verifyDetailAddition(QString detailValueName, QContactDetail detail, bool success)
{
    if(!detail.isEmpty())
    {
        if(!success)
        {
            //qCritical()<<detailType<<" addition failed";
            qCritical()<<detail.definitionName()<<" addition failed, detail value: "<<detailValueName<<" mismatched";
            m_returnValue = success;
        }
        // for debugging
        //else
        //{
        //    qDebug()<<"detail: "<<detail.definitionName()<<" succeeded";
        //}
    }
    else
    {
        qDebug()<<"Detail: "<<detail.definitionName()<<" not supported";
    }
}


/**
  * initializeContactData function
  */
void PimContactDetailManager::initializeContactData()
{
    MWTS_ENTER;

    // address
    m_country = m_detailFetcher.value("CONTACTDETAILS/address_country").toString();
    m_locality = m_detailFetcher.value("CONTACTDETAILS/address_locality").toString();
    m_postcode = m_detailFetcher.value("CONTACTDETAILS/address_postcode").toString();
    m_pobox = m_detailFetcher.value("CONTACTDETAILS/address_postofficebox").toString();
    m_region = m_detailFetcher.value("CONTACTDETAILS/address_region").toString();
    m_street = m_detailFetcher.value("CONTACTDETAILS/address_street").toString();

    // anniversary
    m_anniversary = m_detailFetcher.value("CONTACTDETAILS/anniversary_event").toString();
    int day = m_detailFetcher.value("CONTACTDETAILS/day").toInt();
    int month = m_detailFetcher.value("CONTACTDETAILS/month").toInt();
    int year = m_detailFetcher.value("CONTACTDETAILS/year").toInt();
    QDate tmp(year,month,day);
    m_date.setDate(tmp);

    // avatar
    m_avatar_url = m_detailFetcher.value("CONTACTDETAILS/avatar_image_url").toString();

    // email
    m_emailAddress = m_detailFetcher.value("CONTACTDETAILS/email_address").toString();

    // family
    m_childName = m_detailFetcher.value("CONTACTDETAILS/family_child_name").toString();
    m_spouseName = m_detailFetcher.value("CONTACTDETAILS/family_spouse_name").toString();

    // gender
    m_gender = m_detailFetcher.value("CONTACTDETAILS/gender").toString();

    // geo location
    m_geoAccuracy = m_detailFetcher.value("CONTACTDETAILS/geo_accuracy").toFloat();
    m_geoAltitude = m_detailFetcher.value("CONTACTDETAILS/geo_altitude").toFloat();
    m_geoAltAccuracy = m_detailFetcher.value("CONTACTDETAILS/geo_altitude_accuracy").toFloat();
    m_geoHeading = m_detailFetcher.value("CONTACTDETAILS/geo_heading").toFloat();
    m_geoLabel = m_detailFetcher.value("CONTACTDETAILS/geo_label").toString();
    m_geoLat = m_detailFetcher.value("CONTACTDETAILS/geo_latitude").toFloat();
    m_geoLon = m_detailFetcher.value("CONTACTDETAILS/geo_longitude").toFloat();
    m_geoSpeed = m_detailFetcher.value("CONTACTDETAILS/geo_speed").toFloat();

    // global precence
    m_precenceMsg = m_detailFetcher.value("CONTACTDETAILS/precence_custom_message").toString();
    m_precenceNick = m_detailFetcher.value("CONTACTDETAILS/precence_nick").toString();
    m_precenceState = (QContactPresence::PresenceState)m_detailFetcher.value("CONTACTDETAILS/precence_state").toInt();
    m_precenceImgUrl = m_detailFetcher.value("CONTACTDETAILS/precence_state_image_url").toString();
    m_precenceTxt = m_detailFetcher.value("CONTACTDETAILS/precence_state_text").toString();

    // guid
    m_guid = m_detailFetcher.value("CONTACTDETAILS/guid").toString();

    // name
    m_nameLabel = m_detailFetcher.value("CONTACTDETAILS/name_label").toString();
    m_nameFirst = m_detailFetcher.value("CONTACTDETAILS/name_first").toString();
    m_nameLast = m_detailFetcher.value("CONTACTDETAILS/name_last").toString();
    m_nameMiddle = m_detailFetcher.value("CONTACTDETAILS/name_middle").toString();
    m_namePrefix = m_detailFetcher.value("CONTACTDETAILS/name_prefix").toString();
    m_nameSuffix = m_detailFetcher.value("CONTACTDETAILS/name_suffix").toString();

    // nick name
    m_nameNick = m_detailFetcher.value("CONTACTDETAILS/name_nick").toString();

    // contact note
    m_note = m_detailFetcher.value("CONTACTDETAILS/note").toString();

    // online accaount
    m_accUri = m_detailFetcher.value("CONTACTDETAILS/onlineacc_uri").toString();
    m_serviceProd = m_detailFetcher.value("CONTACTDETAILS/onlineacc_sp").toString();

    // organization
    m_assistant = m_detailFetcher.value("CONTACTDETAILS/organization_assistant").toString();
    m_department = m_detailFetcher.value("CONTACTDETAILS/organization_department").toString();
    m_location = m_detailFetcher.value("CONTACTDETAILS/organization_location").toString();
    m_organizationName = m_detailFetcher.value("CONTACTDETAILS/organization_name").toString();
    m_role = m_detailFetcher.value("CONTACTDETAILS/organization_role").toString();
    m_title = m_detailFetcher.value("CONTACTDETAILS/organization_title").toString();

    // phone number
    m_phoneNumber = m_detailFetcher.value("CONTACTDETAILS/phonenumber").toString();

    // ring tone
    m_ringTone = m_detailFetcher.value("CONTACTDETAILS/ringtone").toString();

    // sync target
    m_syncTarget = m_detailFetcher.value("CONTACTDETAILS/synctarget").toString();

    // tag
    m_tag = m_detailFetcher.value("CONTACTDETAILS/tag").toString();

    // contact thumbnail
    m_thumbUri = m_detailFetcher.value("CONTACTDETAILS/thumbnail_url").toString();

    // type
    m_type = m_detailFetcher.value("CONTACTDETAILS/type").toString();

    // contact uri
    m_contactUrl = m_detailFetcher.value("CONTACTDETAILS/contact_url").toString();


    qDebug()<<"initializeContactData:";
    qDebug()<<"m_country "<<m_country;
    qDebug()<<"m_locality "<<m_locality;
    qDebug()<<"m_postcode "<<m_postcode;
    qDebug()<<"m_pobox "<<m_pobox;
    qDebug()<<"m_region "<<m_region;
    qDebug()<<"m_street "<<m_street;
    qDebug()<<"m_anniversary "<<m_anniversary;
    qDebug()<<"day "<<day;
    qDebug()<<"year "<<year;
    qDebug()<<"month "<<month;
    qDebug()<<"m_date "<<m_date;
    qDebug()<<"m_avatar_url "<<m_avatar_url;
    qDebug()<<"m_emailAddress "<<m_emailAddress;
    qDebug()<<"m_childName "<<m_childName;
    qDebug()<<"m_spouseName "<<m_spouseName;
    qDebug()<<"m_gender "<<m_gender;
    qDebug()<<"m_geoAccuracy "<<m_geoAccuracy;
    qDebug()<<"m_geoAltitude "<<m_geoAltitude;
    qDebug()<<"m_geoAltAccuracy "<<m_geoAltAccuracy;
    qDebug()<<"m_geoHeading "<<m_geoHeading;
    qDebug()<<"m_geoLabel "<<m_geoLabel;
    qDebug()<<"m_geoLat "<<m_geoLat;
    qDebug()<<"m_geoLon "<<m_geoLon;
    qDebug()<<"m_geoSpeed "<<m_geoSpeed;
    qDebug()<<"m_precenceMsg "<<m_precenceMsg;
    qDebug()<<"m_precenceNick "<<m_precenceNick;
    qDebug()<<"m_precenceState "<<m_precenceState;
    qDebug()<<"m_precenceImgUrl "<<m_precenceImgUrl;
    qDebug()<<"m_precenceTxt "<<m_precenceTxt;
    qDebug()<<"m_guid "<<m_guid;
    qDebug()<<"m_nameLabel "<<m_nameLabel;
    qDebug()<<"m_nameFirst "<<m_nameFirst;
    qDebug()<<"m_nameLast "<<m_nameLast;
    qDebug()<<"m_nameMiddle "<<m_nameMiddle;
    qDebug()<<"m_namePrefix "<<m_namePrefix;
    qDebug()<<"m_nameSuffix "<<m_nameSuffix;
    qDebug()<<"m_nameNick "<<m_nameNick;
    qDebug()<<"m_note "<<m_note;
    qDebug()<<"m_accUri "<<m_accUri;
    qDebug()<<"m_serviceProd "<<m_serviceProd;
    qDebug()<<"m_assistant "<<m_assistant;
    qDebug()<<"m_department "<<m_department;
    qDebug()<<"m_location "<<m_location;
    qDebug()<<"m_organizationName "<<m_organizationName;
    qDebug()<<"m_role "<<m_role;
    qDebug()<<"m_title "<<m_title;
    qDebug()<<"m_phoneNumber "<<m_phoneNumber;
    qDebug()<<"m_ringTone "<<m_ringTone;
    qDebug()<<"m_syncTarget "<<m_syncTarget;
    qDebug()<<"m_tag "<<m_tag;
    qDebug()<<"m_thumbUri "<<m_thumbUri;
    qDebug()<<"m_type "<<m_type;
    qDebug()<<"m_contactUrl "<<m_contactUrl;
    qDebug()<<"initializeContactData stop";
}

// eof
