#include "packet/PacketHandler.h"

namespace UserLogout
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_PART, UserLogout );

bool UserLogout::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsLoggedIn() )
		return false;

	// let everyone know this user has logged out (including that user)
	ChatPacket notification( USER_PART, user->GetName(), "_" );
	server->Broadcast( &notification );

	user->SetLoggedIn( false );
	server->Condemn( user );

	return true;
}
