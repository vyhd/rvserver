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
		server->Broadcast( ChatPacket(CLIENT_BACK, user->GetName(), BLANK) );

	// remove away status and reset the idle timer.
	// even if the packet isn't handled, it's still activity.
	user->PacketSent();

	// try to find a handler for this packet's code
	HandlerMap::iterator it = GetMap()->find( MessageCode(packet->iCode) );

	if( it != GetMap()->end() )
		return it->second( server, user, packet );

	/* no handler found, so warn about it. */
	Logger::DebugLog( "Code %u requested, but no handler found!", packet->iCode );

	Logger::DebugLog( "Unhandled packet data:" );
	Logger::DebugLog( "\tCode: %d", packet->iCode );
	Logger::DebugLog( "\tUsername: %s", packet->sUsername.c_str() );
	Logger::DebugLog( "\tMessage: %s", packet->sMessage.c_str() );
	Logger::DebugLog( "\tRGB: %d/%d/%d", packet->r, packet->g, packet->b );

	return false;
}

void PacketHandler::DebugDump()
{
	for( HandlerMap::iterator it = GetMap()->begin(); it != GetMap()->end(); it++ )
		Logger::DebugLog( "%d handled at %p", it->first, it->second );
}

/* 
 * Copyright (c) 2009-10 Mark Cannon ("Vyhd")
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301, USA.
 */
