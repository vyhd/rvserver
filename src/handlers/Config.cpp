#include "packet/PacketHandler.h"

namespace Config
{
	bool HandlePacket( ChatServer *server, User *user, const ChatPacket *packet );
}

REGISTER_HANDLER( CLIENT_CONFIG, Config );

bool Config::HandlePacket( ChatServer *server, User *user, const ChatPacket *packet )
{
	user->SetPrefs( packet->sMessage );
	return true;
}
