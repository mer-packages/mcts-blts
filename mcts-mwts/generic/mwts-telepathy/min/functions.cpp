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

/**
 *  @addtogroup    MwtsTelepathyTests mwts-telepathy-tests
 *  mwts-telepathy-tests implements Telepathy test assets
 *  MIN interface functions.
 * 
 *  @{
 *
 *  @file       functions.cpp
 *  @brief      Implements MIN test interface functionality of the test asset.
 *  
 *  This file includes all MIN test interface functionality for mwts-telepathy.
 *  
 *  @author		Arttu Valo <arttu.valo@digia.com>
 */

#include <MwtsCommon>
#include "stable.h"
#include "interface.h"
#include "TelepathyTest.h"


const char *module_date = __DATE__;
const char *module_time = __TIME__;

const QString availableStatus( "available" );
const QString availableStatusMessage( "I'm available" );

const QString offlineStatus( "offline" );
const QString offlineStatusMessage( "I'm offline" );

const QString clientHandler( "org.freedesktop.Telepathy.Client.CommonClientHandler" );

TelepathyTest telepathyTest;




/**
 *  @fn CreateAccount
 *  @brief Creates new account from configuration.
 *  @param account  Name of the account configuration
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CreateAccount( MinItemParser * item )
{
	MWTS_DEBUG("CreateAccount\n");
	
	char *arg = NULL;
	char *accountgrp = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "account") == 0)
		{
			iret = mip_get_next_string(item, &accountgrp);
			if(iret != ENOERR)
				return iret;
		}
	}
	
	bool bret = false;
				
	bret = telepathyTest.CreateAccountFromConf( QString( accountgrp ) );
	if ( bret ) bret = telepathyTest.SetEnabled( true );
	Tp::SimplePresence presence;
	presence.type = Tp::ConnectionPresenceTypeAvailable;
	presence.status = availableStatus;
	presence.statusMessage = availableStatusMessage;
	if ( bret ) bret = telepathyTest.SetRequestedPresence( presence );
	if ( bret ) bret = telepathyTest.WaitForAccountConnected();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn CreateAccountFromPath
 *  @brief Creates new account from known account path.
 *  @param path  Path to the account
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CreateAccountFromPath( MinItemParser * item )
{
	MWTS_DEBUG("CreateAccountFromPath\n");
	
	char *arg = NULL;
	char *accountpath = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "path") == 0)
		{
			iret = mip_get_next_string(item, &accountpath);
			if(iret != ENOERR)
				return iret;
		}
	}
	
	bool bret = telepathyTest.CreateAccountFromPath( QString( accountpath ) );
	if ( bret ) bret = telepathyTest.SetEnabled( true );
	Tp::SimplePresence presence;
	presence.type = Tp::ConnectionPresenceTypeAvailable;
	presence.status = availableStatus;
	presence.statusMessage = availableStatusMessage;
	if ( bret ) bret = telepathyTest.SetRequestedPresence( presence );
	if ( bret ) bret = telepathyTest.WaitForAccountConnected();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn CallTo
 *  @brief Calls to given number or account.
 *  @param number  Number or account to call to
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CallTo( MinItemParser * item )
{
	MWTS_DEBUG("CallTo\n");
	
	char *arg = NULL;
	char *number = NULL;
    char *speaker = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "number") == 0)
		{
			iret = mip_get_next_string(item, &number);
			if(iret != ENOERR)
				return iret;
		}
		else if(strcmp(arg, "speaker") == 0)
        {
                iret = mip_get_next_string(item, &speaker);
                if(iret != ENOERR)
                        return iret;
        }
	}

    bool UsesSpeaker = false;
    if( speaker != NULL )
    {
    	if ( strcmp( "true", speaker ) == 0 || strcmp( "TRUE", speaker ) == 0 )
    	{
            UsesSpeaker = true;
    	}
    }

    qDebug() << "Uses speaker == " << UsesSpeaker;
	
	bool bret = false;
	QStringList names;
	names.append( number );
	// bret = telepathyTest.RequestContacts( names );
	bret = telepathyTest.ContactsForIdentifiers( names );
	if ( bret ) bret = telepathyTest.EnsureMediaCall( number,
													  QDateTime::currentDateTime(),
													  clientHandler );
	if ( bret ) bret = telepathyTest.ActivateCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn CallToContact
 *  @brief Calls to given contact from contact list.
 *  @param contact  Contact to call to
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CallToContact( MinItemParser * item )
{
	MWTS_DEBUG("CallToContact\n");
	
	char *arg = NULL;
	char *contact = NULL;
    char *speaker = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "contact") == 0)
		{
			iret = mip_get_next_string(item, &contact);
			if(iret != ENOERR)
				return iret;
		}
		else if(strcmp(arg, "speaker") == 0)
        {
                iret = mip_get_next_string(item, &speaker);
                if(iret != ENOERR)
                        return iret;
        }
	}

    bool UsesSpeaker = false;
    if( speaker != NULL )
    {
    	if ( strcmp( "true", speaker ) == 0 || strcmp( "TRUE", speaker ) == 0 )
    	{
            UsesSpeaker = true;
    	}
    }

    qDebug() << "Uses speaker == " << UsesSpeaker;
	
	bool bret = false;
	QStringList contacts;
	contacts.append( contact );
	// bret = telepathyTest.GetContacts( contacts );
	bret = telepathyTest.ContactsForIdentifiers( contacts );
	if ( bret ) bret = telepathyTest.EnsureMediaCall( contact,
													  QDateTime::currentDateTime(),
													  clientHandler );
	if ( bret ) bret = telepathyTest.ActivateCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn SendTextMessageTo
 *  @brief Sends a text message to given number with given parameters.
 *  @param number  Target number
 *  @param length  Message length
 *  @param coding  Message coding
 *  @param classtype  Message class type
 *  @param message  The message to be sent
 *  @return True, if successfull. False otherwise.
 */
LOCAL int SendTextMessageTo( MinItemParser * item )
{
	MWTS_DEBUG("SendTextMessageTo\n");
	
	char *arg = NULL;
	char *number = NULL;
	char *length = NULL;
	char *coding = NULL;
	char *classtype = NULL;
	char *message = NULL;
	QString qlength;
	QString qcoding;
	QString qclasstype;
	QString qmessage;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "number") == 0)
		{
			iret = mip_get_next_string(item, &number);
			if(iret != ENOERR)
				return iret;
		}

		if(strcmp(arg, "length") == 0)
		{
			iret = mip_get_next_string(item, &length);
			if(iret != ENOERR)
				return iret;
		}
	}

	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "coding") == 0)
		{
			iret = mip_get_next_string(item, &coding);
			if(iret != ENOERR)
				return iret;
		}

		if(strcmp(arg, "classtype") == 0)
		{
			iret = mip_get_next_string(item, &classtype);
			if(iret != ENOERR)
				return iret;
		}

		if(strcmp(arg, "message") == 0)
		{
			iret = mip_get_next_string(item, &message);
			if(iret != ENOERR)
				return iret;
		}
	}

	qmessage.append(message);
	qlength.append(length);
	qcoding.append(coding);
	qclasstype.append(classtype);

	
	bool bret = false;
	QStringList names;
	names.append( number );
	bret = telepathyTest.EnsureTextChat( number,
										 QDateTime::currentDateTime(),
										 clientHandler );
	
	// With option to send user created 'message' 
	// we preserve ability to send in normal messages what we want
	// This may become useful in future

	// If 'length' is used, all messages are strictly determined by length, coding and class
	// to match requirements for testcases
	if ( bret ) bret = telepathyTest.SendMessage( qmessage, qlength, qcoding, qclasstype );

	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn WaitForIncomingMessage
 *  @brief Waits for incoming text message.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int WaitForIncomingMessage( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("WaitForIncomingMessage\n");
	bool bret = telepathyTest.WaitForIncomingMessage();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn WaitForIncomingMessage
 *  @brief Waits for incoming text message.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int WaitForIncomingIM( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("WaitForIncomingIM\n");
	bool bret = telepathyTest.WaitForIncomingIM();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn SendDTMF
 *  @brief Sends DTMF tone through current phone call.
 *  @param 0-D  Any parameter is considered as DTMF tone, raging from 0 ot letter D
 *  @return True, if successfull. False otherwise.
 */
LOCAL int SendDTMF( MinItemParser * item )
{
	MWTS_DEBUG("SendDTMF\n");
	
	char *arg = NULL;
	bool bret = false;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		int tone = atoi( arg );
		if ( tone < Tp::DTMFEventDigit0 || tone >= Tp::NUM_DTMF_EVENTS )
		{
			qCritical()<< "Invalid DTMF parameter in script: " << arg;
			continue;
		}
		bret=telepathyTest.SendDTMFTone( (Tp::DTMFEvent)tone );
		g_pResult->StepPassed( __FUNCTION__, bret);
	}
	
	return 0;
}


/**
 *  @fn HoldCall
 *  @brief Request current call to be held.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int HoldCall( __attribute__((unused)) MinItemParser * item )
{
	MWTS_ENTER;
	bool bret = telepathyTest.HoldCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn UnholdCall
 *  @brief Request current call to be unheld.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int UnholdCall( __attribute__((unused)) MinItemParser * item )
{
	MWTS_ENTER;
	bool bret = telepathyTest.UnholdCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn WaitForIncomingCall
 *  @brief Wait for incoming call.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int WaitForIncomingCall( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("WaitForIncomingCall\n");
	bool bret = telepathyTest.WaitForIncomingCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn CreateConferenceCall
 *  @brief Wait for incoming call, and merge it to current call.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int CreateConferenceCall( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("CreateConferenceCall\n");
	bool bret = telepathyTest.CreateConferenceCall();
        g_pResult->StepPassed( __FUNCTION__, bret );
        return ENOERR;
}


/**
 *  @fn AcceptCall
 *  @brief Accept incoming call.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int AcceptCall(MinItemParser * item )
{
        MWTS_DEBUG("AcceptCall\n");

        char *arg = NULL;
        char *speaker = NULL;
        int iret = ENOERR;

        while(mip_get_next_string(item, &arg) == ENOERR)
        {
                if(strcmp(arg, "speaker") == 0)
                {
                        iret = mip_get_next_string(item, &speaker);
                        if(iret != ENOERR)
                                return iret;
                }
        }

        bool UsesSpeaker = false;
        if(speaker != NULL)
            UsesSpeaker = (strcmp("true",speaker) == 0 || strcmp("TRUE",speaker) == 0)?(true):(false);

        qDebug() << "Uses speaker == " << UsesSpeaker;

        bool bret = telepathyTest.AcceptCall();		//UsesSpeaker
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn EndCall
 *  @brief End currenet call.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int EndCall( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("EndCall\n");
	bool bret = telepathyTest.EndCall();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


LOCAL int WaitForCallEnded( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("WaitForCallEnded\n");
	bool bret = telepathyTest.WaitForCallEnded();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn RemoveAccount
 *  @brief Remove current account.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int RemoveAccount( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("RemoveAccount\n");
	
	Tp::SimplePresence presence;
	presence.type = Tp::ConnectionPresenceTypeOffline;
	presence.status = offlineStatus;
	presence.statusMessage = offlineStatusMessage;
	telepathyTest.SetRequestedPresence( presence );
	telepathyTest.SetEnabled( false );
	bool bret = telepathyTest.RemoveAccount();
	g_pResult->StepPassed( __FUNCTION__, bret );
	return ENOERR;
}


/**
 *  @fn Wait
 *  @brief Wait given amount of time.
 *  @param time  Time to be waiting, in milliseconds
 *  @return True, if successfull. False otherwise.
 */
LOCAL int Wait( MinItemParser * item )
{
	MWTS_DEBUG("Wait\n");
		
	char *arg = NULL;
	char *time = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "time") == 0)
		{
			iret = mip_get_next_string(item, &time);
			if(iret != ENOERR)
				return iret;
		}
	}
	
	int msec = atoi( time );
	
	// Wait for given time
	bool ret = telepathyTest.Wait( msec );
	
	g_pResult->StepPassed( __FUNCTION__, ret );
	
	return ENOERR;
}


/**
 *  @fn StayOnLine
 *  @brief Wait on line of the current call going for given amount of time.
 *  @param time  Time to stay on line, in milliseconds
 *  @return True, if successfull. False otherwise.
 */
LOCAL int StayOnLine( MinItemParser * item )
{
	MWTS_DEBUG("StayOnLine\n");
		
	char *arg = NULL;
	char *time = NULL;
	int iret = ENOERR;
	
	while(mip_get_next_string(item, &arg) == ENOERR)
	{
		if(strcmp(arg, "time") == 0)
		{
			iret = mip_get_next_string(item, &time);
			if(iret != ENOERR)
				return iret;
		}
	}
	
	int msec = atoi( time );
	
	// Wait for given time
	bool ret = telepathyTest.StayOnLine( msec );
	
	g_pResult->StepPassed( __FUNCTION__, ret );
	
	return ENOERR;
}


/**
 *  @fn StartLatencyMeasuring
 *  @brief Start latency measurement.
 *  @return True, if successfull. False otherwise.
 */
LOCAL int StartLatencyMeasuring( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("StartLatencyMeasuring\n");
	telepathyTest.StartLatencyMeasuring();
	return ENOERR;
}


/**
 *  @fn NextLatencyMeasure
 *  @brief Stop current latency measurement, write it to report and restart latency measurement.
 *  @param 0-D  Any parameter is considered as DTMF tone, raging from 0 ot letter D
 *  @return True, if successfull. False otherwise.
 */
LOCAL int NextLatencyMeasure( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("NextLatencyMeasure\n");
	int elapsed = telepathyTest.NextLatencyMeasure();
	if ( g_pResult )
	{
		g_pResult->AddMeasure( QString( "Latency" ), elapsed, QString( "msec" ) );
	}
	return ENOERR;
}


LOCAL int ReportOutgoingCallLatency( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("ReportOutgoingCallLatency\n");
	int latency = telepathyTest.ReportOutgoingCallLatency();
	return ENOERR;
}


LOCAL int ReportIncomingCallLatency( __attribute__((unused)) MinItemParser * item )
{
	MWTS_DEBUG("ReportIncomingCallLatency\n");
	int latency = telepathyTest.ReportIncomingCallLatency();
	return ENOERR;
}



/**
 * @}
 */

int ts_get_test_cases( DLList ** list )
{
	MwtsMin::DeclareFunctions( list );
	ENTRYTC (*list, "CreateAccount", CreateAccount);
	ENTRYTC (*list, "CreateAccountFromPath", CreateAccountFromPath);
	ENTRYTC (*list, "CallTo", CallTo);
	ENTRYTC (*list, "CallToContact", CallToContact);
	ENTRYTC (*list, "SendTextMessageTo", SendTextMessageTo);
	ENTRYTC (*list, "SendDTMF", SendDTMF);
	ENTRYTC (*list, "HoldCall", HoldCall);
	ENTRYTC (*list, "UnholdCall", UnholdCall);
	ENTRYTC (*list, "WaitForIncomingCall", WaitForIncomingCall);
	ENTRYTC (*list, "CreateConferenceCall", CreateConferenceCall);
	ENTRYTC (*list, "WaitForIncomingMessage", WaitForIncomingMessage);
	ENTRYTC (*list, "WaitForIncomingIM", WaitForIncomingIM);
	ENTRYTC (*list, "AcceptCall", AcceptCall);
	ENTRYTC (*list, "EndCall", EndCall);
	ENTRYTC (*list, "WaitForCallEnded", WaitForCallEnded);
	ENTRYTC (*list, "RemoveAccount", RemoveAccount);
	ENTRYTC (*list, "Wait", Wait);
	ENTRYTC (*list, "StayOnLine", StayOnLine);
	ENTRYTC (*list, "StartLatencyMeasuring", StartLatencyMeasuring);
	ENTRYTC (*list, "NextLatencyMeasure", NextLatencyMeasure);
	ENTRYTC (*list, "ReportOutgoingCallLatency", ReportOutgoingCallLatency);
	ENTRYTC (*list, "ReportIncomingCallLatency", ReportIncomingCallLatency);
	return ENOERR;
}

