/* PacketHandler: defines a base class that includes a static registration
 * for packet handlers. I like it because it's easier than doing manual
 * branching (and probably faster), plus it's easier to modify later. */

#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <map>
#include <stdint.h>

// All handlers will need these headers, so we may as well include them
#include "ChatServer.h"
#include "model/User.h"
#include "packet/ChatPacket.h"
#include "packet/MessageCodes.h"

// function pointer for a packet handler, taking pointers
// to ChatServer, a User, and a ChatPacket for handling
typedef bool (*HandlerFn)(ChatServer*,User*,const ChatPacket*);

// aliases, for convenience
typedef std::pair<MessageCode,HandlerFn> HandlerEntry;
typedef std::map<MessageCode,HandlerFn> HandlerMap;

namespace PacketHandler
{
	/* gets the static map that holds handler entries */
	HandlerMap* GetMap();

	/* finds the handler for this packet and handles it. returns false
	 * if no handler exists or if the packet could not be handled. */
	bool Handle( ChatServer *server, User *user, const ChatPacket *packet );

	/* dumps the entire map in terms of code to function pointer */
	void DebugDump();
}

struct RegisterHandler
{
	RegisterHandler( MessageCode iCode, HandlerFn fn )
	{
		PacketHandler::GetMap()->insert( HandlerEntry(iCode,fn) );
	}
};

/* Use this to define a specific function to handle a chat code. */
#define REGISTER_HANDLER( code, func ) \
	static RegisterHandler g_Handler##code( code, &func )

#endif // PACKET_HANDLER_H

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
