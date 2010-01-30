#include "packet/PacketHandler.h"
#include "model/Room.h"

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
	ChatPacket msg( packet->iCode, user->GetName(), packet->sMessage );

	// broadcast the packet to the user's room
	user->GetRoom()->Broadcast( msg );

	return true;
}
