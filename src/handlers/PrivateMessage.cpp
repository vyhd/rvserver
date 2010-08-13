#include "packet/PacketHandler.h"

bool HandlePM( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( USER_PM, HandlePM );

bool HandlePM( ChatServer *server, User *user, const ChatPacket *packet )
{
	// handled, but ignored
	if( !user->IsLoggedIn() || user->IsMuted() )
		return false;

	// find the user whose name is given in the first param
	User *recipient = server->GetUserByName( packet->sUsername );

	// user not found. bizarre, huh?
	if( recipient == NULL )
		return false;

	// copy the packet code and message
	ChatPacket msg( USER_PM, user->GetName(), packet->sMessage );

	// send this packet to the recipient
	recipient->Write( msg.ToString() );

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
