#include <algorithm>
#include "logger/Logger.h"
#include "model/RoomList.h"
#include "model/Room.h"
#include "model/User.h"
#include "packet/ChatPacket.h"
#include "packet/MessageCodes.h"
#include "util/StringUtil.h"

using namespace std;

RoomList::RoomList()
{
	// ensure that the default room always exists
	m_pDefaultRoom = new Room;
	m_Rooms[DEFAULT_ROOM.c_str()] = m_pDefaultRoom;
}

RoomList::~RoomList()
{
	map<string,Room*>::iterator it;

	// delete all Room pointers in our map
	for( it = m_Rooms.begin(); it != m_Rooms.end(); it++ )
		delete it->second;

	m_Rooms.clear();
}

Room* RoomList::GetRoom( const std::string &sRoom ) const
{
	map<string,Room*>::const_iterator it;

	for( it = m_Rooms.begin(); it != m_Rooms.end(); it++ )
		if( !StringUtil::CompareNoCase(sRoom, it->first) )
			return it->second;

	return NULL;	// no match
}

std::string RoomList::GetName( const Room *room ) const
{
	map<string,Room*>::const_iterator it;

	for( it = m_Rooms.begin(); it != m_Rooms.end(); it++ )
		if( it->second == room )
			return it->first;

	return string();
}

void RoomList::RemoveUser( User *user )
{
	map<string,Room*>::iterator it;

	// just remove from all rooms...we can afford it.
	// that's easier than the logic needed to find/erase.
	for( it = m_Rooms.begin(); it != m_Rooms.end(); it++ )
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

	if( sRoom.compare(DEFAULT_ROOM) == 0 )
	{
		Logger::SystemLog( "Someone tried to /destroy %s! Ignoring...", DEFAULT_ROOM.c_str() );
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

	Room *pDefault = GetRoom( DEFAULT_ROOM );	// this should never fail

	for( set<User*>::const_iterator it = users->begin(); it != users->end(); it++ )
	{
		(*it)->SetRoom( pDefault );

		ChatPacket msg( JOIN_ROOM, (*it)->GetName(), DEFAULT_ROOM );
		pDefault->Broadcast( &msg );
	}

	delete( pRoom );
}
