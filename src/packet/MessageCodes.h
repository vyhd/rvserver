#ifndef MESSAGE_CODES_H
#define MESSAGE_CODES_H

enum MessageCode
{
	// user messages
	USER_LIST	= 0,
	USER_JOIN	= 1,
	USER_PART	= 2,

	// communication messages
	ROOM_MESSAGE	= 3,
	ROOM_ACTION	= 4,
	USER_PM		= 5,

	// moderator commands
	USER_KICK	= 6,
	USER_DISABLE	= 7,
	USER_BAN	= 8,
	USER_UNBAN	= 9,
	USER_MUTE	= 10,
	USER_UNMUTE	= 11,

	SERVER_DOWN	= 12,
	IDLE_KICK	= 13,
	IP_QUERY	= 14,
	DEBUG_COMMAND	= 15,

	// timed mod commands
	USER_TIMEDBAN	= 16,
	USER_TIMEDMUTE	= 17,

	// mod chat command
	MOD_CHAT	= 18,

	// login responses
	ACCESS_GRANTED	= 100,
	ACCESS_DENIED	= 101,
	LIMIT_REACHED	= 102,

	// ???
	PM_BOX		= 200,

	// other mod commands
	FORCE_CLEAR	= 300,
	FORCE_URL	= 301,

	// room commands
	JOIN_ROOM	= 400,
	CREATE_ROOM	= 402,
	DESTROY_ROOM	= 403,
	ROOM_LIST	= 404,
	FORCE_JOIN	= 405,

	// client messages
	CLIENT_IDLE	= 500,
	CLIENT_AWAY	= 501,
	CLIENT_BACK	= 502,
	CLIENT_CONFIG	= 600,

	// global broadcast
	WALL_MESSAGE	= 601,

	// more client messages
	START_TYPING	= 602,
	STOP_TYPING	= 603,
	RESET_TYPING	= 604
};

#endif // MESSAGE_CODES_H

/* 
 * Copyright (c) 2009-10 Mark Cannon ("Vyhd")
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301, USA.
 */
