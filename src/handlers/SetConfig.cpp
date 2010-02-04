#include "packet/PacketHandler.h"

static bool Handle( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( CLIENT_CONFIG, Handle );

bool Handle( ChatServer *server, User *user, const ChatPacket *packet )
{
	user->SetPrefs( packet->sMessage );
	return true;
}
