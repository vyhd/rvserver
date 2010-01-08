#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

bool ListUsers::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	const std::set<User*>* users = server->GetUserList();

	for( std::set<User*>::const_iterator it = users->begin(); it != users->end(); it++ )
	{
		ChatPacket packet( USER_LIST, (*it)->GetName(), server->GetUserState(*it) );
		server->Send( &packet, user );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, BLANK, "done" );
	server->Send( &finish, user );
	return true;
}
