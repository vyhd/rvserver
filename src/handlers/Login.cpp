#include "packet/PacketHandler.h"
#include "logger/Logger.h"
#include "verinfo.h"

namespace Login
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_JOIN, Login );

std::string GetCode( const User* user, const ChatPacket *packet )
{
	std::string ret;

	ret.assign( user->GetRoom() );
	ret.push_back( '|' );

	// display user level. TODO: user level.
	if( user->IsMod() )
		ret.push_back( 'c' );
	else
		ret.push_back( '_' );

	// display muted status (M for muted, _ for not)
	ret.push_back( user->IsMuted() ? 'M' : '_' );

	ret.push_back( user->IsIdle() ? 'i' : '_' );

	if( user->IsIdle() )
	{
		char sIdleTime[4];
		sprintf( sIdleTime, "%04u", user->GetIdleTime() );
		ret.append( sIdleTime );
	}

	// display away status, appending the message if away
	ret.push_back( user->IsAway() ? 'a' : '_' );
	if( user->IsAway() )
		ret.append( user->GetMessage() );

	return ret;
}	

bool Login::HandlePacket( ChatServer *server, User* const user, const ChatPacket *packet )
{
	// don't take this packet from a user that's already in
	if( user->IsLoggedIn() )
		return false;

	// if this name is already logged in, don't let it log in again.
	if( server->GetUserByName(packet->sParam1) != NULL )
	{
		ChatPacket response( ACCESS_DENIED );
		server->Send( &response, user );
		server->Condemn( user );
		return true;
	}

	// set the user's name from the login packet
	user->SetName( packet->sParam1 );

	// create and send a response packet.
	ChatPacket response( ACCESS_GRANTED );
	server->Send( &response, user );

	user->SetLoggedIn( true );

	// HAAAACK
	if( !user->GetName().compare("Fire_Adept") && !packet->sParam2.compare("password") )
		user->SetIsMod( true );

	// send the new guy a nice little message about the server version
	char buffer[128];
	sprintf( buffer, "Server build %lu, compiled %s", BUILD_VERSION, BUILD_DATE );
	std::string sBuffer( buffer );

	ChatPacket debug( WALL_MESSAGE, "_", sBuffer );
	server->Send( &debug, user );

	// new guy's here! let everyone know!
	// XXX WHAT: calling GetCode() inexplicably changes the
	// User* address, even though it can't actually reach it.
	// it won't cause problems here, but be warned.
	ChatPacket msg( USER_JOIN, user->GetName(), GetCode(user,packet) );
	server->Broadcast( &msg );

	return true;
}
