#include "packet/PacketHandler.h"

bool HandleAway( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleBack( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( CLIENT_AWAY, HandleAway );
REGISTER_HANDLER_FN( CLIENT_BACK, HandleBack );

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
	{
		std::string sMessage = user->GetName() + " has gone away.";
		ChatPacket msg( WALL_MESSAGE, "_", sMessage );
		server->Broadcast( &msg );
	}

	user->SetAway( true );

	return true;
}

bool HandleBack( ChatServer *server, User *user, const ChatPacket *packet )
{
	// abuse potential or unnecessary message
	if( user->IsMuted() || !user->IsAway() )
		return false;

	// broadcast a status change packet
	ChatPacket back( CLIENT_BACK, user->GetName(), "Bad Wolf" );
	server->Broadcast( &back );

	// broadcast a message
	std::string sMessage = user->GetName() + " has returned.";
	ChatPacket msg( WALL_MESSAGE, "_", sMessage );
	server->Broadcast( &msg );

	return true;
}
