#include "packet/PacketHandler.h"

namespace RoomAction
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( ROOM_ACTION, RoomAction );

bool RoomAction::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// handled, but ignored
	if( !user->IsLoggedIn() ||  user->IsMuted() )
		return false;

	// create a packet for the broadcast using the sender's name
	ChatPacket msg( packet->iCode, user->GetName(), packet->sParam2 );

	// broadcast the packet to the user's room
	server->Broadcast( &msg, &user->GetRoom() );

	return true;
}
