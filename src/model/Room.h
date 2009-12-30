/* Room: a set of Users that can interact with one another. */

#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <set>

class ChatServer;
class User;

class Room
{
public:
	const std::string& GetName()	{ return m_sName; }

	/* sends a message to all users in this room */
	void 
private:
	std::string m_sName;
	

#endif // ROOM_H
