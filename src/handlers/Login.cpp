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

	// user can't be away or idle at this time. blank 'em.
	ret.append( "__" );

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
		// HACK: sometimes, it doesn't get killed users fast enough.
		// if this user has the same IP, boot the original.
		User *other = server->GetUserByName(packet->sParam1);

		std::string sTheirIP( server->GetUserIP(other) );

		if( sTheirIP.compare(server->GetUserIP(user)) != 0 )
		{
			ChatPacket response( ACCESS_DENIED );
			server->Send( &response, user );
			server->Condemn( user );
			return true;
		}
		else
		{
			// you...you DOUBLE HACK: change the other user
			// name so it won't kill us when we log in.
			other->SetName( other->GetName() + " (dead)" );
			server->Condemn( other );
		}
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
	sprintf( buffer, "Server build %u, compiled %s", BUILD_VERSION, BUILD_DATE );
	std::string sBuffer( buffer );

	ChatPacket debug( WALL_MESSAGE, "_", sBuffer );
	server->Send( &debug, user );

	// new guy's here! let everyone know!
	// BEWARE: with g++ 4.0.3 and -g -ggdb -O2, calling GetCode mysteriously
	// changes the pointer address of "user". We put it here, so it won't
	// cause problems, but now you know if something like it happens again.
	ChatPacket msg( USER_JOIN, user->GetName(), GetCode(user,packet) );
	server->Broadcast( &msg );

	return true;
}
