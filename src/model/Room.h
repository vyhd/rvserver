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
