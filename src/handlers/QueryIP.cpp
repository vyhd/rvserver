#include "packet/PacketHandler.h"

namespace QueryIP
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( IP_QUERY, QueryIP );

bool QueryIP::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return;

	
	ChatPacket broadcast( *packet );
	server->Broadcast( &broadcast );
	return true;
}
