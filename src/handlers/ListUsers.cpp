#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

bool ListUsers::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// we intentionally don't check for login status because we want
	// external processes to see who's where and doing what.

	const std::list<User*>* users = server->GetUserList();

	for( std::list<User*>::const_iterator it = users->begin(); it != users->end(); it++ )
	{
		if( !user->IsLoggedIn() )
			continue;

		ChatPacket packet( USER_LIST, (*it)->GetName(), server->GetUserState(*it) );
		user->Write( packet.ToString() );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, BLANK, "done" );
	user->Write( finish.ToString() );

	return true;
}
