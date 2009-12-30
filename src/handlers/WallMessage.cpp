#include "packet/PacketHandler.h"

namespace WallMessage
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( WALL_MESSAGE, WallMessage );

bool WallMessage::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	ChatPacket broadcast( *packet );
	server->Broadcast( &broadcast );
	return true;
}
