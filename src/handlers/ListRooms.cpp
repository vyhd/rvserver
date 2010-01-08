#include "packet/PacketHandler.h"

using namespace std;

namespace RoomList
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( ROOM_LIST, RoomList );

bool RoomList::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	const list<string> *rooms = server->GetRoomList();

	for( list<string>::const_iterator it = rooms->begin(); it != rooms->end(); it++ )
	{
		ChatPacket room( ROOM_LIST, BLANK, *it );
		server->Send( &room, user );
	}

	return true;
}
