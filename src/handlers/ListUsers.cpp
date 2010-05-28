#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

bool ListUsers::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// we intentionally don't check for login status because we want
	// external processes to see who's where and doing what.

	const std::list<User*>* users = server->GetUserList();

	for( std::list<User*>::const_iterator it = users->begin(); it != users->end(); ++it )
	{
		if( !(*it)->IsLoggedIn() )
			continue;

		ChatPacket packet( USER_LIST, (*it)->GetName(), server->GetUserState(*it) );
		user->Write( packet.ToString() );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, BLANK, "done" );
	user->Write( finish.ToString() );

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
