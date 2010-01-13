#include "packet/PacketHandler.h"
#include "logger/Logger.h"
#include "verinfo.h"
#include <cstdlib>

namespace Login
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_JOIN, Login );

bool Login::HandlePacket( ChatServer *server, User* const user, const ChatPacket *packet )
{
	// don't take this packet from a user that's already in
	if( user->IsLoggedIn() )
		return false;

	// if this name is already logged in, don't let it log in again.
	if( server->GetUserByName(packet->sUsername) != NULL )
	{
		// HACK: sometimes, it doesn't get killed users fast enough.
		// if this user has the same IP, boot the original.
		// work around it until we get a proper solution (keepalive?) in place.
		User *other = server->GetUserByName(packet->sUsername);

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
	user->SetName( packet->sUsername );

	// create and send a response packet.
	ChatPacket response( ACCESS_GRANTED );
	server->Send( &response, user );

	user->SetLoggedIn( true );

	// temporary testing code
	if( packet->sMessage.compare("gimmemod") == 0 )
	{
		char code = MOD_LEVELS[rand() % NUM_MOD_LEVELS];
		user->SetLevel( code );
	}
	else
	{
		const char USER_LEVELS[] = { 'B', 'F', '!' };
		char code = USER_LEVELS[rand() % 3];
		user->SetLevel( code );
	}

	if( !packet->sUsername.compare("Sammy") )
		user->SetLevel( 'C' );

	// send the new guy a nice little message about the server version
	char buffer[128];
	snprintf( buffer, 128, "Server build %u, compiled %s", BUILD_VERSION, BUILD_DATE );
	std::string sBuffer( buffer );

	ChatPacket debug( WALL_MESSAGE, BLANK, sBuffer );
	server->Send( &debug, user );

	// new guy's here! let everyone know!
	ChatPacket msg( USER_JOIN, user->GetName(), server->GetUserState(user) );
	server->Broadcast( &msg );

	return true;
}
