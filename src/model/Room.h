/* Room: a collection of users that get messages from each other. */

#ifndef ROOM_H
#define ROOM_H

#include <set>
#include <string>

/* Defined here so User doesn't need to pull in RoomList. */
const std::string DEFAULT_ROOM = "RV Chat";

class ChatPacket;
class User;

class Room
{
public:
	// sends a packet to all members of this room
	void Broadcast( const ChatPacket &packet );

	void AddUser( User *user );
	void RemoveUser( User *user );

	// returns true if the given User is in this Room
	bool HasUser( User *user ) const;

	const std::set<User*>* GetUserSet() const { return &m_Users; }

private:
	std::set<User*> m_Users;
};

#endif // ROOM_H

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
