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
	server->Broadcast( ChatPacket(USER_PART, user->GetName(), BLANK) );

//	ChatServer needs to do this so it can save configuration
//	user->SetLoggedIn( false );

	user->Kill();

	return true;
}
