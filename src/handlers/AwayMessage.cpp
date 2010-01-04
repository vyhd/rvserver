#include "packet/PacketHandler.h"

bool HandleAway( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( CLIENT_AWAY, HandleAway );

bool HandleAway( ChatServer *server, User *user, const ChatPacket *packet )
{
	// this could be abused by muted users, so don't let 'em use it
	if( user->IsMuted() )
		return false;

	user->SetMessage( packet->sParam2 );

	// broadcast a status change packet
	ChatPacket away( CLIENT_AWAY, user->GetName(), user->GetMessage() );
	server->Broadcast( &away );

	// broadcast a notification if the user wasn't away yet
	if( !user->IsAway() )
		server->WallMessage( user->GetName() + " has gone away." );

	user->SetAway( true );

	return true;
}
