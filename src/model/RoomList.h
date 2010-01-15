/* RoomList: maintains a set of Rooms in the server. */

#ifndef ROOM_LIST_H
#define ROOM_LIST_H

#include <map>
#include <string>

class Room;
class User;

class RoomList
{
public:
	RoomList();
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
