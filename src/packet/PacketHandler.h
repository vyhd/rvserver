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

/* Use this to define a handler for a given code.
 * The function that gets passed is HandlePacket in the given namespace. */
#define REGISTER_HANDLER( code, namespace ) \
	static RegisterHandler g_##namespace##Handler##code( code, &namespace::HandlePacket )

/* Use this to define a specific function to handle a code. */
#define REGISTER_HANDLER_FN( code, fn ) \
	static RegisterHandler g_Handler##code( code, &fn )

#endif // PACKET_HANDLER_H
