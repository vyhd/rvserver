#include "packet/PacketHandler.h"
#include "logger/Logger.h"

HandlerMap* PacketHandler::GetMap()
{
	static HandlerMap g_HandlerMap;
	return &g_HandlerMap;
}

bool PacketHandler::Handle( ChatServer *server, User *user, const ChatPacket *packet )
{
	// if the user who sent this was away, send a notification
	if( user->IsAway() )
	{
		ChatPacket notification( CLIENT_BACK, user->GetName(), "_" );
		server->Broadcast( &notification );
	}

	// remove away status and reset the idle timer.
	// even if the packet isn't handled, it's still activity.
	user->PacketSent();

	// try to find a handler for this packet's code
	HandlerMap::iterator it = GetMap()->find( MessageCode(packet->iCode) );

	if( it != GetMap()->end() )
		return it->second( server, user, packet );

	/* no handler found, so warn about it. */
	Logger::SystemLog( "Code %u requested, but no handler found!", packet->iCode );

	Logger::SystemLog( "Unhandled packet data:" );
	Logger::SystemLog( "\tCode: %d", packet->iCode );
	Logger::SystemLog( "\tParam 1: %s", packet->sParam1.c_str() );
	Logger::SystemLog( "\tParam 2: %s", packet->sParam2.c_str() );
	Logger::SystemLog( "\tRGB: %d/%d/%d", packet->r, packet->g, packet->b );

	return false;
}

void PacketHandler::DebugDump()
{
	for( HandlerMap::iterator it = GetMap()->begin(); it != GetMap()->end(); it++ )
		Logger::SystemLog( "%d handled at %p", it->first, it->second );
}
