#include "packet/PacketHandler.h"
#include "util/StringUtil.h"
#include "network/DatabaseConnector.h"
#include "model/RoomList.h"
#include "model/Room.h"
#include "logger/Logger.h"
#include <cstdlib>

namespace Login
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_JOIN, Login );

bool Login::HandlePacket( ChatServer *server, User* const user, const ChatPacket *packet )
{
	// don't take this packet from a user that's already in
	if( user->IsLoggedIn() )
		return false;

	// if this name is already logged in, don't let it log in again.
	if( server->GetUserByName(packet->sUsername) != NULL )
	{
		// HACK: the server doesn't always boot dead users quickly enough.
		// Work around it until we get a proper solution (keepalive?) in place.
		User *other = server->GetUserByName(packet->sUsername);

		if( other->GetIP() == user->GetIP() )
			other->Kill();
	}

	// set the user's name from the login packet
	user->SetName( packet->sUsername );

	// dispatch a message to the connector to check login status.
	DatabaseConnector *conn = server->GetConnection();
	conn->Login( user, packet->sMessage );

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
