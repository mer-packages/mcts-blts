/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* ofono-general.h -- Common definitions

   Copyright (C) 2000-2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef OFONO_GENERAL_H
#define OFONO_GENERAL_H
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

/* Test case numbers */

enum BLTSoFonoCases {
	BLTS_OFONO_INFORMATION_QUERY = 1,
	BLTS_OFONO_REGISTER_TO_NETWORK,
	BLTS_OFONO_ENABLE_MODEMS,
	BLTS_OFONO_MODEMS_ONLINE,
	BLTS_OFONO_MODEMS_OFFLINE,
	BLTS_OFONO_CREATE_VOICECALL,
	BLTS_OFONO_CREATE_VOICECALL_WITH_HIDDEN_CALLER_ID,
	BLTS_OFONO_ANSWER_TO_VOICECALL_AND_HANGUP,
	BLTS_OFONO_ANSWER_TO_VOICECALL_AND_HANGUP_ALL,
	BLTS_OFONO_ANSWER_TO_VOICECALL_AND_WAIT_FOR_CALL,
	BLTS_OFONO_ANSWER_TO_VOICECALL_AND_DEFLECT,
	BLTS_OFONO_ANSWER_TO_VOICECALL_AND_REMOTE_HANGUP,
	BLTS_OFONO_CANCEL_VOICECALL,
	BLTS_OFONO_TRANSFER,
	BLTS_OFONO_SWAP,
	BLTS_OFONO_RELEASE_AND_ANSWER,
	BLTS_OFONO_HOLD_AND_ANSWER,
	BLTS_OFONO_CALL_AND_SEND_DTMF,
	BLTS_OFONO_DISABLE_FORWARDINGS,
	BLTS_OFONO_UNCONDITIONAL_FORWARDING,
	BLTS_OFONO_FORWARD_IF_BUSY,
	BLTS_OFONO_FORWARD_IF_NO_REPLY,
	BLTS_OFONO_FORWARD_IF_NOT_REACHABLE,
	BLTS_OFONO_SEND_SMS,
	BLTS_OFONO_RECEIVE_SMS,
	BLTS_OFONO_CHANGE_PIN,
	BLTS_OFONO_ENTER_PIN,
	BLTS_OFONO_RESET_PIN,
	BLTS_OFONO_LOCK_PIN,
	BLTS_OFONO_UNLOCK_PIN,
	BLTS_OFONO_SET_MICROPHONE_VOLUME,
	BLTS_OFONO_SET_SPEAKER_VOLUME,
	BLTS_OFONO_SET_MUTED,
	BLTS_OFONO_CALL_METERS_READ,
	BLTS_OFONO_CALL_METERS_SET,
	BLTS_OFONO_CALL_METERS_RESET,
	BLTS_OFONO_CALL_METERS_NEAR_MAX_WARNING,
	BLTS_OFONO_CHECK_BARRING_PROPERTIES,
	BLTS_OFONO_DISABLE_BARRINGS,
	BLTS_OFONO_DISABLE_INCOMING_BARRING,
	BLTS_OFONO_DISABLE_OUTGOING_BARRING,
	BLTS_OFONO_CHANGE_PASSWORD_FOR_BARRINGS,
	BLTS_OFONO_CALL_BARRINGS_TEST,
	BLTS_OFONO_LIST_ALL_PROPERTIES,
	BLTS_OFONO_PROPOSE_SCAN,
	BLTS_OFONO_SMSC_NUMBER_TEST,
	BLTS_OFONO_MULTIPARTY_CALL,
	BLTS_OFONO_MULTIPARTY_CALL_PRIVATE,
	BLTS_OFONO_RAT,
	BLTS_OFONO_DATA_CONTEXT,
	BLTS_OFONO_DATA_CONTEXT_PING,
	BLTS_OFONO_CALL_SETTINGS_WAITING,
};

#define G_VALUE_INIT {0,{{0}}}

#define	DEFAULT_MODEM		"/isimodem0"	//obsolete, modems are found through oFono
#define DEFAULT_TIMEOUT		200000

/* oFono interfaces and default paths */
#define OFONO_BUS		"org.ofono"
#define OFONO_MNGR_PATH		"/"
#define OFONO_MNGR_INTERFACE	"org.ofono.Manager"
#define OFONO_MODEM_INTERFACE	"org.ofono.Modem"
#define OFONO_PB_INTERFACE	"org.ofono.Phonebook"
#define OFONO_NW_INTERFACE 	"org.ofono.NetworkRegistration"
#define OFONO_VC_INTERFACE 	"org.ofono.VoiceCallManager"
#define OFONO_MESSAGE_INTERFACE	"org.ofono.MessageManager"
#define OFONO_CALL_INTERFACE	"org.ofono.VoiceCall"
#define OFONO_SIM_INTERFACE	"org.ofono.SimManager"
#define OFONO_CV_INTERFACE	"org.ofono.CallVolume"
#define OFONO_METER_INTERFACE	"org.ofono.CallMeter"
#define OFONO_RADIO_INTERFACE	"org.ofono.RadioSettings"
#define OFONO_CONNMAN_INTERFACE	"org.ofono.ConnectionManager"
#define OFONO_CONTEXT_INTERFACE "org.ofono.ConnectionContext"
#define OFONO_CALL_SETTINGS_INTERFACE "org.ofono.CallSettings"

#define MAX_MODEMS		10	// maximum number of modems in system (hopefully, if more needed just increase)

#endif /* OFONO_GENERAL_H */
