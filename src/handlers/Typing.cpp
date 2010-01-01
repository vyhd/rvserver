#include "packet/PacketHandler.h"

namespace Typing
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( START_TYPING, Typing );
REGISTER_HANDLER( STOP_TYPING, Typing );
REGISTER_HANDLER( RESET_TYPING, Typing );

bool Typing::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// never send any of these messages if the user shouldn't
	if( user->IsMuted() )
		return false;

	// try to find the target, ignore the packet if they don't exist
	User *recipient = server->GetUserByName( packet->sParam1 );

	if( recipient == NULL )
		return false;

	// create a packet with the sender's name and send the handled code
	ChatPacket notification( packet->iCode, user->GetName(), "_" );
	server->Send( &notification, recipient );

	return true;
}
