/* -*- mode: C; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* bt_fute_agent.h -- Bluetooth pairing agent for tests

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

#ifndef BT_FUTE_AGENT_H_
#define BT_FUTE_AGENT_H_

/* Roles in connection */
typedef enum {
	BT_FUTE_ROLE_MASTER = 0x00,
	BT_FUTE_ROLE_SLAVE = 0x01,
	BT_FUTE_ROLE_UNDEFINED,
} BtFuteAgentRole;

/* Private data implementation */
typedef struct _BtFuteAgent BtFuteAgent;

/* Create new instance, free with bt_fute_agent_unref() */
BtFuteAgent *bt_fute_agent_new (int adapter);

/* Ref, aka copy the instance (if needed) */
BtFuteAgent *bt_fute_agent_ref (BtFuteAgent *agent);

/* Unref the instance, NULL is returned if the agent was freed */
BtFuteAgent *bt_fute_agent_unref (BtFuteAgent *agent);

/* Run the agent (starts a thread if it's not having one already)
 *
 * Returns: Zero for success, negative for failure
 */
int bt_fute_agent_run (BtFuteAgent *agent);

/* Stop the agent
 *
 * Returns: Positive for success, Zero if not running, negative for
 *          thread or syscall failures
 */
int bt_fute_agent_stop (BtFuteAgent *agent);

/*
 * XXX:
 * - Roles for the agent mean that it needs to handle some events differently.
 *   On the other hand, the agent could be stupid, and just provide means for
 *   the "client" to ask for existence of certain events...
 */

/* Get the agent role at the moment */
BtFuteAgentRole bt_fute_agent_get_role (BtFuteAgent *agent);

/* Set the desired role, this might not happen though... */
int bt_fute_agent_set_role (BtFuteAgent *agent, BtFuteAgentRole role);

#endif /* BT_FUTE_AGENT_H_ */
