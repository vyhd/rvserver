#include "packet/PacketHandler.h"

namespace RoomMessage
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( ROOM_MESSAGE, RoomMessage );

bool RoomMessage::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// handled, but ignored
	if( !user->IsLoggedIn() ||  user->IsMuted() )
		return false;

	// create a packet for broadcast, including text and RGB
	ChatPacket msg( *packet );

	// set the first param to the user's name
	msg.sParam1.assign( user->GetName() );

	// broadcast the packet to the user's room
	server->Broadcast( &msg, &user->GetRoom() );

	return true;
}
