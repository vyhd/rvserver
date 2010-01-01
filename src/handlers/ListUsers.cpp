#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

static std::string GetCode( User *user )
{
	std::string ret( user->GetRoom() );

	ret.push_back( '|' );

	// display user level. TODO: user level.
	if( user->IsMod() )
		ret.push_back( 'c' );
	else
		ret.push_back( '_' );

	// display muted status (M for muted, _ for not)
	ret.push_back( user->IsMuted() ? 'M' : '_' );

	// display idle status, including time if needed
	// TODO: add time
	ret.push_back( user->IsIdle() ? 'i' : '_' );

	// display away status, appending the message if away
	ret.push_back( user->IsAway() ? 'a' : '_' );
	if( user->IsAway() )
		ret.append( user->GetMessage() );

	return ret;
}	

bool ListUsers::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	const std::set<User*> *users = server->GetUserSet();

	for( std::set<User*>::const_iterator it = users->begin(); it != users->end(); it++ )
	{
		ChatPacket packet( USER_LIST, (*it)->GetName(), GetCode(*it) );
		Logger::SystemLog( "ListUsers: %s", packet.ToString().c_str() );
		server->Send( &packet, user );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, "_", "done" );
	server->Send( &finish, user );
}
