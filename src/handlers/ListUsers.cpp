#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace ListUsers
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_LIST, ListUsers );

void GetCode( User *user, std::string& sCode )
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
//	sCode.push_back( user->IsIdle() ? 'i' : '_' );
	sCode.push_back( '_' );

/*
	if( user->IsIdle() )
	{
		char sIdleTime[4];
		sprintf( sIdleTime, "%04u", user->GetIdleTime() );
		sCode.append( sIdleTime );
	}
*/
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
		GetCode( (*it), sCode );
		ChatPacket packet( USER_LIST, (*it)->GetName(), sCode );
		Logger::SystemLog( "ListUsers: %s", packet.ToString().c_str() );
		server->Send( &packet, user );
	}

	// signify that the user update is done
	ChatPacket finish( USER_LIST, "_", "done" );
	server->Send( &finish, user );
}
