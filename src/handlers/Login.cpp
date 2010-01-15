#include "packet/PacketHandler.h"
#include "network/DatabaseConnector.h"
#include "model/RoomList.h"
#include "model/Room.h"
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

		// you...you DOUBLE HACK: change the other user
		// name so it won't kill us when we log in.
		other->SetName( other->GetName() + " (dead)" );
		other->Kill();
	}

	// set the user's name from the login packet
	user->SetName( packet->sUsername );

	// attempt to verify against the database
	DatabaseConnector *conn = server->GetConnection();
	LoginResult result = conn->Login( packet->sUsername, packet->sMessage, user );

	MessageCode code = ACCESS_DENIED;

	// set the response packet based on our response
	switch( result )
	{
		case LOGIN_SUCCESS:		code = ACCESS_GRANTED; break;
		case LOGIN_ERROR:		code = ACCESS_DENIED; break;
		case LOGIN_ERROR_ATTEMPTS:	code = LIMIT_REACHED; break;
		case LOGIN_SERVER_DOWN:		code = SERVER_DOWN; break;
	}

	// create and send a response packet.
	ChatPacket response( code );
	user->Write( response.ToString() );

	// if the client can't verify, disconnect them
	if( code != ACCESS_GRANTED )
	{
		user->Kill();
		return true;
	}

	server->GetRoomList()->GetDefaultRoom()->AddUser( user );
	user->SetLoggedIn( true );

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
