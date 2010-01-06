#include "packet/PacketHandler.h"

namespace RoomList
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( ROOM_LIST, RoomList );

bool RoomList::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
//	const std::vector<std::string>
	ChatPacket main( ROOM_LIST, "_", "Main" );
	ChatPacket spam( ROOM_LIST, "_", "Spam Room" );

	server->Send( &main, user );
	server->Send( &spam, user );
	return true;
}
