#include "packet/PacketHandler.h"
#include "logger/Logger.h"
#include "model/Room.h"
#include "model/RoomList.h"

bool HandleJoin( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleCreate( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleDestroy( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleForceJoin( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( JOIN_ROOM, HandleJoin );
REGISTER_HANDLER_FN( CREATE_ROOM, HandleCreate );
REGISTER_HANDLER_FN( DESTROY_ROOM, HandleDestroy );
REGISTER_HANDLER_FN( FORCE_JOIN, HandleForceJoin );

bool HandleJoin( ChatServer *server, User *user, const ChatPacket *packet )
{
	RoomList *list = server->GetRoomList();
	const std::string &sRoom = packet->sMessage;
	Room *room = list->GetRoom( sRoom );

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

	const std::string &sRoom = packet->sMessage;
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
		Logger::SystemLog( "Failed to get newly created room \"%s\"!", sRoom.c_str() );
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

	const std::string &sRoom = packet->sMessage;

	// consequences shall be dire!
	if( sRoom.compare(DEFAULT_ROOM) == 0 )
	{
		const std::string HAL = "[Server] I'm afraid I can't let you do that, " + user->GetName() + ".";
		server->Broadcast( ChatPacket(WALL_MESSAGE, BLANK, HAL) );
		user->Kill();
		return true;
	}

	// remove the room and broadcast its destruction
	server->GetRoomList()->RemoveRoom( sRoom );
	server->Broadcast( ChatPacket(DESTROY_ROOM, BLANK, sRoom) );

	return true;
}

bool HandleForceJoin( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	const std::string& sRoom = packet->sMessage;
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

	const std::string sMessage = target->GetName() + " was forced to join "
		+ sRoom + " by " + user->GetName();

	server->WallMessage( sMessage );

	return true;
}
