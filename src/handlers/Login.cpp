#include "packet/PacketHandler.h"
#include "util/StringUtil.h"
#include "network/DatabaseConnector.h"
#include "model/RoomList.h"
#include "model/Room.h"
#include "logger/Logger.h"
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

	// dispatch a message to the connector to check login status.
	DatabaseConnector *conn = server->GetConnection();
	conn->Login( user, packet->sMessage );

	return true;
}
