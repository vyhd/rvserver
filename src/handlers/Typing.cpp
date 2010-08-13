/* Essentially, we just pass these onto the targeted user, so we can just
 * define one function, get the target, and pass on the packet code. */

#include "packet/PacketHandler.h"

bool HandleTyping( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( START_TYPING, HandleTyping );
REGISTER_HANDLER( STOP_TYPING, HandleTyping );
REGISTER_HANDLER( RESET_TYPING, HandleTyping );

bool HandleTyping( ChatServer *server, User *user, const ChatPacket *packet )
{
	// never send any of these messages if the user shouldn't
	if( user->IsMuted() )
		return false;

	// try to find the target, ignore the packet if they don't exist
	User *recipient = server->GetUserByName( packet->sUsername );

	if( recipient == NULL )
		return false;

	// create a packet with the sender's name and send the handled code
	ChatPacket notification( packet->iCode, user->GetName(), BLANK );
	recipient->Write( notification.ToString() );

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
