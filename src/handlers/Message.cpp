#include "packet/PacketHandler.h"
#include "model/Room.h"

namespace RoomMessage
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( ROOM_MESSAGE, RoomMessage );

bool RoomMessage::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// handled, but ignored
	if( !user->IsLoggedIn() || user->IsMuted() )
		return false;

	// create a packet for broadcast, copying message and RGB
	ChatPacket msg( *packet );

	// set the first param to the user's name
	msg.sUsername.assign( user->GetName() );

	user->GetRoom()->Broadcast( &msg );

	return true;
}
