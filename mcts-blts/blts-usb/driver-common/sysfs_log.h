/*
 * sysfs_log.h -- BLTS sysfs logging functionality
 *
 * Copyright (C) 2010 by Nokia Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SYSFS_LOG_H
#define SYSFS_LOG_H

int sysfs_log_add(void);
void sysfs_log_remove(void);

int sysfs_log_write(const char *fmt, ...);

#endif /* SYSFS_LOG_H */

