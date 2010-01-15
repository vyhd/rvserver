#include "packet/PacketHandler.h"
#include "model/Room.h"
#include "model/RoomList.h"

using namespace std;

static bool CmdListRooms( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( ROOM_LIST, CmdListRooms );

bool CmdListRooms( ChatServer *server, User *user, const ChatPacket *packet )
{
	const map<string,Room*> *rooms = server->GetRoomList()->GetRooms();

	for( map<string,Room*>::const_iterator it = rooms->begin(); it != rooms->end(); it++ )
	{
		ChatPacket room( ROOM_LIST, BLANK, it->first );
		user->Write( room.ToString() );
	}

	return true;
}
