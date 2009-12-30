#include "packet/PacketHandler.h"
#include "logger/Logger.h"

namespace UserDisable
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_DISABLE, UserDisable );

bool UserDisable::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	// ignore attempts by the unwashed masses
	if( !user->IsMod() )
		return false;

	// find the intended target
	User *victim = server->GetUserByName( packet->sParam2 );

	// can't touch 'em. not around, or not logged in
	if( victim == NULL || !victim->IsLoggedIn() )
		return false;

	// broadcast the disabling message
	std::string sMessage = victim->GetName() + " was disabled by " + user->GetName();

	ChatPacket msg( WALL_MESSAGE, "_", sMessage );
	Logger::SystemLog( "Wall message: %s", msg.ToString().c_str() );
	server->Broadcast( &msg );

	// formally disable the client.
	ChatPacket disable( USER_DISABLE );
	server->Send( &disable, victim );
	server->Condemn( victim );

	return true;
}
