#include "packet/PacketHandler.h"

bool Logout( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( USER_PART, Logout );

bool Logout( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsLoggedIn() )
		return false;

	// ChatServer takes care of reaping dead users.
	// Just kill and wait for the next update cycle.
	user->Kill();

	return true;
}

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
