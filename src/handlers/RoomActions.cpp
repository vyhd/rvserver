#include "packet/PacketHandler.h"
#include "logger/Logger.h"
#include "model/Room.h"
#include "model/RoomList.h"

bool HandleJoin( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleCreate( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleDestroy( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleForceJoin( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( JOIN_ROOM, HandleJoin );
REGISTER_HANDLER( CREATE_ROOM, HandleCreate );
REGISTER_HANDLER( DESTROY_ROOM, HandleDestroy );
REGISTER_HANDLER( FORCE_JOIN, HandleForceJoin );

using namespace std;

bool HandleJoin( ChatServer *server, User *user, const ChatPacket *packet )
{
	const string &sRoom = packet->sMessage;

	Room *room = server->GetRoomList()->GetRoom( sRoom );

	// doesn't exist, so can't join
	if( room == NULL )
		return false;

	// already here
	if( room->HasUser(user) )
		return false;

	room->AddUser( user );
	server->Broadcast( ChatPacket(JOIN_ROOM, user->GetName(), sRoom) );

	return true;
}

bool HandleCreate( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	const string &sRoom = packet->sMessage;

	RoomList *list = server->GetRoomList();
	Room *room = list->GetRoom( sRoom );

	if( room != NULL )
	{
		ChatPacket msg( WALL_MESSAGE, BLANK, "That room already exists!" );
		user->Write( msg.ToString() );
		return false;
	}

	// length check, since really long names make the client freak out
	if( sRoom.length() > 16 )
	{
		ChatPacket msg( WALL_MESSAGE, BLANK, "Room names are limited to 16 characters." );
		user->Write( msg.ToString() );
		return false;
	}

	list->AddRoom( sRoom );

	// this should never fail - we just added it
	room = list->GetRoom( sRoom );
	if( room == NULL )
	{
		LOG->System( "Failed to get newly created room \"%s\"!", sRoom.c_str() );
		return true;	// log it
	}

	room->AddUser( user );

	// broadcast the new room creation and join
	server->Broadcast( ChatPacket(CREATE_ROOM, BLANK, sRoom) );
	server->Broadcast( ChatPacket(JOIN_ROOM, user->GetName(), sRoom) );

	return true;
}

bool HandleDestroy( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	RoomList *pList = server->GetRoomList();
	const string &sRoom = packet->sMessage;

	// check to make sure that no one's trying to destroy Main
	const string &sDefaultRoom = pList->GetName( pList->GetDefaultRoom() );

	// consequences shall be dire!
	if( sRoom.compare( sDefaultRoom ) == 0 )
	{
		const string HAL = "[Server] I'm afraid I can't let you do that, " + user->GetName() + ".";
		server->Broadcast( ChatPacket(WALL_MESSAGE, BLANK, HAL) );
		user->Kill();
		return true;
	}

	// remove the room and broadcast its destruction
	pList->RemoveRoom( sRoom );
	server->Broadcast( ChatPacket(DESTROY_ROOM, BLANK, sRoom) );

	return true;
}

bool HandleForceJoin( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	const string& sRoom = packet->sMessage;
	RoomList *list = server->GetRoomList();
	Room *room = list->GetRoom( sRoom );

	// doesn't exist, don't let mods send users into oblivion
	if( room == NULL )
		return false;

	User *target = server->GetUserByName( packet->sUsername );

	// can't find the desired target, so can't mess with
	if( target == NULL )
		return false;

	room->AddUser( target );

	// broadcast the new room join
	server->Broadcast( ChatPacket(JOIN_ROOM, target->GetName(), sRoom) );

	const string sMessage = target->GetName() + " was forced to join "
		+ sRoom + " by " + user->GetName();

	server->WallMessage( sMessage );

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
