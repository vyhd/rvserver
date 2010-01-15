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
	User *recipient = server->GetUserByName( packet->sUsername );

	// user not found. bizarre, huh?
	if( recipient == NULL )
		return false;

	// copy the packet code and message
	ChatPacket msg( USER_PM, user->GetName(), packet->sMessage );

	// send this packet to the recipient
	recipient->GetSocket()->Write( msg.ToString() );

	return true;
}
