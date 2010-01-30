#include "packet/PacketHandler.h"
#include "model/Room.h"

bool HandleAway( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( CLIENT_AWAY, HandleAway );

bool HandleAway( ChatServer *server, User *user, const ChatPacket *packet )
{
	// this could be abused by muted users, so don't let 'em use it
	if( user->IsMuted() )
		return false;

	user->SetMessage( packet->sMessage );

	// broadcast a status change packet
	ChatPacket away( CLIENT_AWAY, user->GetName(), user->GetMessage() );
	server->Broadcast( away );

	// broadcast a notification if the user wasn't away yet
	if( !user->IsAway() )
	{
		ChatPacket msg( WALL_MESSAGE, BLANK, user->GetName() + " has gone away." );
		user->GetRoom()->Broadcast( msg );
	}

	user->SetAway( true );

	return true;
}
