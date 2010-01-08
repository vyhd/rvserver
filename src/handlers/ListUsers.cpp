#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

void GetCode( ChatServer *server, User *user, std::string& sCode )
{
	sCode.assign( user->GetRoom() );
	sCode.push_back( '|' );

	// display user level. TODO: user level.

	if( user->IsMod() )
		sCode.push_back( 'c' );
	else
		sCode.push_back( '_' );

	// display muted status (M for muted, _ for not)
	sCode.push_back( user->IsMuted() ? 'M' : '_' );

	// display idle status, including time if needed
	if( server->IsIdle(user) )
	{
		Logger::SystemLog( "Server says %s is idle for %u minutes", user->GetName().c_str(), user->GetIdleMinutes() );
		sCode.push_back( 'i' );
		char sIdleTime[4];
		snprintf( sIdleTime, 4, "%04u", user->GetIdleMinutes() );
		sCode.append( sIdleTime );
	}
	else
	{
		sCode.push_back( '_' );
	}

	// display away status, appending the message if away
	sCode.push_back( user->IsAway() ? 'a' : '_' );
	if( user->IsAway() )
		sCode.append( user->GetMessage() );
}	

bool ListUsers::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	const std::set<User*>* users = server->GetUserList();

	for( std::set<User*>::const_iterator it = users->begin(); it != users->end(); it++ )
	{
		std::string sCode;
		GetCode( server, (*it), sCode );
		ChatPacket packet( USER_LIST, (*it)->GetName(), sCode );
		server->Send( &packet, user );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, BLANK, "done" );
	server->Send( &finish, user );
	return true;
}
