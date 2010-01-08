/* ChatServer: the main server file. Handles all users, message broadcasting,
 * that sort of thing. */

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <set>
#include <list>
#include <string>
#include "network/SocketListener.h"

class User;
class ChatPacket;

// 1024 = 1 KB, so 4 KB
const unsigned BUFFER_SIZE = 1024*4;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();

	/* returns a reference to the user with the given name */
	User* GetUserByName( const std::string &sName ) const;

	/* returns a char* to a static buffer containing the client's IP */
	const char* GetUserIP( const User *user ) const;

	/* returns a std::string expressing the user's current state */
	std::string GetUserState( const User *user ) const;

	/* returns true if this user meets the server criteria for idle */
	bool IsIdle( const User *user ) const;

	/* returns a const reference to the Users set */
	const std::set<User*>* GetUserList() const	{ return &m_Users; }

	/* returns a const reference to the Rooms list */
	const std::list<std::string>* GetRoomList() const	{ return &m_Rooms; }

	/* a few self-explanatory room functions */
	bool RoomExists( const std::string &str ) const;
	void AddRoom( const std::string &str );
	void RemoveRoom( const std::string &str );

	/* sends a system message to all mods on the server */
	void WallMessage( const std::string &sMessage );

	/* sends a packet to all users (in a specific room, if given) */
	void Broadcast( const ChatPacket *packet, const std::string *sRoom = NULL );

	/* sends a packet to the specified user. */
	void Send( const ChatPacket *packet, User *user );
	void Send( const std::string &str, User *user );

	/* main processing loop */
	void MainLoop();

	/* Deleting users in the middle of an update loop can lead to confusing
	 * segfaults. We take the easy way out and delete after the loop. */
	void Condemn( User *user );

private:
	/* given iSocket, creates a user with that socket */
	void AddUser( unsigned iSocket );

	/* disconnects the given user from the server */
	void RemoveUser( User *user );

	/* performs an update cycle on the given user */
	void UpdateUser( User *user );

	/* handles a packet received from user */
	void HandleUserPacket( User *user, const std::string &in );

	/* checks the idle statistics of a user, broadcasts if needed */
	void CheckIdleStatus( User *user );

	/* reads from this user into the given bufer, up to len bytes */
	int Read( char *buffer, unsigned len, User *user );

	/* writes a data packet to the given user. */
	int Write( const std::string &str, User *user );

	/* instance buffer for reading packet data from a user */
	char m_sReadBuffer[BUFFER_SIZE];

	/* listens for connections on the given port */
	SocketListener *m_pListener;

	/* set of all users being updated */
	std::set<User*> m_Users;

	/* set of all users to delete after this update */
	std::set<User*> m_UsersToDelete;

	/* list of all rooms on the server */
	std::list<std::string> m_Rooms;
};

#endif // CHAT_SERVER_H
