/* blts_sync.h -- Multiclient test sync interface

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

#ifndef BLTS_SYNC_H
#define BLTS_SYNC_H

int blts_sync_anon();
int blts_sync_anon_to(unsigned long msec);

int blts_sync_add_tag(char *tag);
int blts_sync_del_tag(char *tag);
int blts_sync_tagged(char *tag);
int blts_sync_tagged_to(char *tag, unsigned long msec);

int blts_sync_master_init(unsigned client_count);
int blts_sync_client_init();
void blts_sync_client_cleanup();

#endif /* BLTS_SYNC_H */

