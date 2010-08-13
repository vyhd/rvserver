#include "packet/PacketHandler.h"
#include "model/Room.h"

bool Away( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( CLIENT_AWAY, Away );

bool Away( ChatServer *server, User *user, const ChatPacket *packet )
{
	// this can be abused by muted users, so don't let them use it
	if( user->IsMuted() )
		return false;

	user->SetMessage( packet->sMessage );

	// broadcast a status change packet
	ChatPacket away( CLIENT_AWAY, user->GetName(), user->GetMessage() );
	server->Broadcast( away );

	// broadcast a notification to the room if the user just went away
	if( !user->IsAway() )
	{
		ChatPacket msg( WALL_MESSAGE, BLANK, user->GetName() + " has gone away." );
		user->GetRoom()->Broadcast( msg );
	}

	user->SetAway( true );

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
