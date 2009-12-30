#include "packet/PacketHandler.h"

namespace Login
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( USER_JOIN, Login );

bool Login::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
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

	// automatically join to Main
	user->SetRoom( "Main" );

	std::string sCode = "Main|";

	// HAAAACK
	if( !user->GetName().compare("Fire_Adept") && 
		!packet->sParam2.compare("password") ||
		!user->GetName().compare("Judge") )
	{
		user->SetIsMod( true );
		sCode.append( "c___" );
	}
	else
	{
		sCode.append( "____" );
	}

	// create and send a response packet.
	ChatPacket response( ACCESS_GRANTED );
	server->Send( &response, user );

	user->SetLoggedIn( true );

	// new guy's here! let everyone know!
	ChatPacket msg( USER_JOIN, user->GetName(), sCode );
	server->Broadcast( &msg );

	return true;
}
