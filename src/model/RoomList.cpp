#include <algorithm>
#include "logger/Logger.h"
#include "model/RoomList.h"
#include "model/Room.h"
#include "model/User.h"
#include "packet/ChatPacket.h"
#include "packet/MessageCodes.h"
#include "util/Config.h"
#include "util/StringUtil.h"

using namespace std;

RoomList::RoomList( Config *cfg )
{
	const char* DEFAULT_ROOM = cfg->Get( "DefaultRoom", true, "Main" );

	// ensure that the default room always exists
	m_pDefaultRoom = new Room;
	m_Rooms[DEFAULT_ROOM] = m_pDefaultRoom;
}

RoomList::~RoomList()
{
	map<string,Room*>::iterator it;

	// delete all Room pointers in our map
	for( it = m_Rooms.begin(); it != m_Rooms.end(); ++it )
		delete it->second;

	m_pDefaultRoom = NULL;
	m_Rooms.clear();
}

Room* RoomList::GetRoom( const std::string &sRoom ) const
{
	map<string,Room*>::const_iterator it;

	for( it = m_Rooms.begin(); it != m_Rooms.end(); ++it )
		if( !StringUtil::CompareNoCase(sRoom, it->first) )
			return it->second;

	return NULL;	// no match
}

std::string RoomList::GetName( const Room *room ) const
{
	map<string,Room*>::const_iterator it;

	for( it = m_Rooms.begin(); it != m_Rooms.end(); ++it )
		if( it->second == room )
			return it->first;

	return string();
}

void RoomList::RemoveUser( User *user )
{
	map<string,Room*>::iterator it;

	// just remove from all rooms...we can afford it.
	// that's easier than the logic needed to find/erase.
	for( it = m_Rooms.begin(); it != m_Rooms.end(); ++it )
		it->second->RemoveUser( user );
}
		
bool RoomList::RoomExists( const std::string &sRoom ) const
{
	return GetRoom(sRoom) != NULL;
}

void RoomList::AddRoom( const std::string &sRoom )
{
	// don't allow duplicates
	if( RoomExists(sRoom) )
		return;

	m_Rooms[sRoom.c_str()] = new Room;
}

void RoomList::RemoveRoom( const std::string &sRoom )
{
	if( !RoomExists(sRoom) )
		return;

	const string &sDefault = this->GetName( m_pDefaultRoom );

	if( sRoom.compare(sDefault) == 0 )
	{
		LOG->System( "Someone tried to /destroy %s! Ignoring...", sDefault.c_str() );
		return;
	}

	// find the room in the room list and remove it.
	map<string,Room*>::iterator it = m_Rooms.find( sRoom.c_str() );

	if( it == m_Rooms.end() )
		return;

	// get the Room pointer from this entry, erase the entry
	Room *pRoom = it->second;
	m_Rooms.erase( it );

	// remove all users who were in this room and boot them back to Main
	const set<User*> *users = pRoom->GetUserSet();

	for( set<User*>::const_iterator it = users->begin(); it != users->end(); ++it )
	{
		ChatPacket msg( JOIN_ROOM, (*it)->GetName(), sDefault );
		m_pDefaultRoom->Broadcast( msg );
	}

	delete( pRoom );
}

void RoomList::ClearRooms()
{
	const string &sDefault = this->GetName( m_pDefaultRoom );

	for( map<string,Room*>::iterator it = m_Rooms.begin(); it != m_Rooms.end(); ++it )
	{
		const string &sName = it->first;

		// skip the main room (we're booting everyone else back here)
		if( sName.compare(sDefault) )
			continue;

		RemoveRoom( sName );
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
