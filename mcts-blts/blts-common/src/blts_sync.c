/* blts_sync.c -- Multiclient test sync

   Copyright (C) 2000-2011, Nokia Corporation.

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

#include "blts_sync.h"

int blts_sync_anon()
{
	return 0;
}

int blts_sync_anon_to(unsigned long msecs)
{
	return 0;
}


int blts_sync_add_tag(char *tag)
{
	return 0;
}

int blts_sync_del_tag(char *tag)
{
	return 0;
}

int blts_sync_tagged(char *tag)
{
	return 0;
}

int blts_sync_tagged_to(char *tag, unsigned long msecs)
{
	return 0;
}

int blts_sync_prepare(char *sem_name, unsigned client_count)
{
	return 0;
}

void blts_sync_cleanup()
{
	return 0;
}


