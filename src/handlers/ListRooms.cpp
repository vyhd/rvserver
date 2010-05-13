#include "packet/PacketHandler.h"
#include "model/Room.h"
#include "model/RoomList.h"

using namespace std;

static bool CmdListRooms( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( ROOM_LIST, CmdListRooms );

bool CmdListRooms( ChatServer *server, User *user, const ChatPacket *packet )
{
	const map<string,Room*> *rooms = server->GetRoomList()->GetRooms();

	for( map<string,Room*>::const_iterator it = rooms->begin(); it != rooms->end(); ++it )
	{
		ChatPacket room( ROOM_LIST, BLANK, it->first );
		user->Write( room.ToString() );
	}

	return true;
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
