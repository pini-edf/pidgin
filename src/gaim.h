/*
 * gaim
 *
 * Gaim is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef _GAIM_H_
#define _GAIM_H_

#define XPATCH BAD /* Because Kalla Said So */

#include "connection.h"

/* Globals in main.c */
extern int opt_away;
extern char *opt_away_arg;
extern int opt_debug;

extern GSList *message_queue;
extern GSList *unread_message_queue;
extern GSList *away_time_queue;

/* Functions in idle.c */
extern gint check_idle(gpointer);

#endif /* _GAIM_H_ */
