#include "packet/PacketHandler.h"

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
	const std::string &sRoom = packet->sMessage;

	// doesn't exist, so can't join
	if( !server->RoomExists(sRoom) )
		return false;

	// already here
	if( user->IsInRoom(sRoom) )
		return false;

	user->SetRoom( sRoom );

	// broadcast the new room join
	ChatPacket msg( JOIN_ROOM, user->GetName(), sRoom );
	server->Broadcast( &msg );

	return true;
}

bool HandleCreate( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	const std::string &sRoom = packet->sMessage;

	if( server->RoomExists(sRoom) )
	{
		ChatPacket msg( WALL_MESSAGE, "_", "That room already exists!" );
		server->Send( &msg, user );
		return false;
	}

	server->AddRoom( sRoom );
	user->SetRoom( sRoom );

	// broadcast the creation of the room
	{
		ChatPacket msg( CREATE_ROOM, BLANK, sRoom );
		server->Broadcast( &msg );
	}

	// broadcast the new room join
	{
		ChatPacket msg( JOIN_ROOM, user->GetName(), sRoom );
		server->Broadcast( &msg );
	}

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
		const std::string HAL = "[Server] I'm afraid I can't let you do that, " + user->GetName();
		ChatPacket msg( WALL_MESSAGE, "_", HAL );
		server->Broadcast( &msg );
		server->Condemn( user );
		return true;
	}

	server->RemoveRoom( sRoom );

	// broadcast the room's destrucity
	ChatPacket msg( DESTROY_ROOM, BLANK, sRoom );
	server->Broadcast( &msg );

	return true;
}

bool HandleForceJoin( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	const std::string& sRoom = packet->sMessage;

	// doesn't exist, don't let mods send users into oblivion
	if( !server->RoomExists(sRoom) )
		return false;

	User *target = server->GetUserByName( packet->sUsername );

	// can't find the desired target, so can't mess with
	if( target == NULL )
		return false;

	target->SetRoom( sRoom );

	// broadcast the new room join
	ChatPacket msg( JOIN_ROOM, target->GetName(), sRoom );
	server->Broadcast( &msg );

	const std::string sMessage = target->GetName() + " was forced to join "
		+ sRoom + " by " + user->GetName();

	server->WallMessage( sMessage );

	return true;
}
