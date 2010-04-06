/* RoomList: maintains a set of Rooms in the server. */

#ifndef ROOM_LIST_H
#define ROOM_LIST_H

#include <map>
#include <string>

class Room;
class User;
class Config;

class RoomList
{
public:
	RoomList( Config *cfg );
	~RoomList();

	Room* GetDefaultRoom() { return m_pDefaultRoom; }
	const Room* GetDefaultRoom() const { return m_pDefaultRoom; }

	/* gets a room by its name, or NULL if not there */
	Room* GetRoom( const std::string &name ) const;

	/* gets a room name by its pointer */
	std::string GetName( const Room *room ) const;

	/* does a room exist with the given name? */
	bool RoomExists( const std::string &name ) const;

	/* adds a room to the list */
	void AddRoom( const std::string &name );

	/* removes a room name from the list */
	void RemoveRoom( const std::string &name );

	/* removes this user from all rooms on the server */
	void RemoveUser( User *user );

	/* returns a const pointer to the internal room map */
	const std::map<std::string,Room*>* GetRooms() const { return &m_Rooms; }

private:
	std::map<std::string,Room*> m_Rooms;
	Room *m_pDefaultRoom;
};

#endif // ROOM_LIST_H

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
