#include "logger/Logger.h"
#include "model/Room.h"
#include "model/User.h"
#include "network/Socket.h"
#include "packet/ChatPacket.h"

using namespace std;

void Room::AddUser( User *user )
{
	// do these here, for sanity's sake
	if( user->GetRoom() != NULL )
		user->GetRoom()->RemoveUser( user );

	user->SetRoom( this );

	m_Users.insert( user );
}

void Room::RemoveUser( User *user )
{
	m_Users.erase( user );
}

bool Room::HasUser( User *user ) const
{
	return m_Users.find(user) != m_Users.end();
}

void Room::Broadcast( const ChatPacket &packet )
{
	// cache this: we only need to call it once
	const string msg = packet.ToString();

	for( set<User*>::iterator it = m_Users.begin(); it != m_Users.end(); ++it )
	{
		User *user = (*it);

		// ignore users who aren't logged in
		if( !user->IsLoggedIn() )
			continue;

		user->Write( msg );
	}
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
