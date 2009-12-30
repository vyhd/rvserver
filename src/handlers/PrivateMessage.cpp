#include "packet/PacketHandler.h"

namespace PrivateMessage
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_PM, PrivateMessage );

bool PrivateMessage::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// handled, but ignored
	if( !user->IsLoggedIn() || user->IsMuted() )
		return false;

	// find the user whose name is given in the first param
	User *recipient = server->GetUserByName( packet->sParam1 );

	// user not found. how bizarre, huh?
	if( recipient == NULL )
		return false;

	ChatPacket msg( *packet );

	// set the first param to this user's name
	msg.sParam1.assign( user->GetName() );

	// send the packet off to the target
	server->Send( &msg, recipient );

	return true;
}

