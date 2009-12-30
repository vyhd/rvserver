#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace UserKick
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}


REGISTER_HANDLER( USER_KICK, UserKick );

bool UserKick::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// if this user can't kick, don't let them
	if( !user->IsMod() )
		return false;

	// find the intended kickee
	User *victim = server->GetUserByName( packet->sParam2 );

	// no match, or can't be kicked, so ignore
	if( victim == NULL || !victim->IsLoggedIn() )
		return false;

	// broadcast the kicking message. do this first, so we're sure
	// we can still get the username off of the kicked.
	std::string sMessage = victim->GetName() + " has been kicked by " + user->GetName();

	ChatPacket msg( WALL_MESSAGE, "_", sMessage );
	Logger::SystemLog( "Wall message: %s", msg.ToString().c_str() );
	server->Broadcast( &msg );

	// formally kick the client, then force a server-side logout
	ChatPacket boot( USER_KICK );
	server->Send( &boot, victim );
	server->Condemn( victim );

	Logger::SystemLog( "victim pointer: %p", victim );
	return true;
}
