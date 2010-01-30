/* ChatServer: the main server file. Handles all users, message broadcasting,
 * that sort of thing. */

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <list>
#include <string>
#include "network/SocketListener.h"
#include "model/RoomList.h"

class ChatPacket;
class DatabaseConnector;
class User;

// 1024 = 1 KB, so 4 KB
const unsigned BUFFER_SIZE = 1024*4;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();

	// returns a reference to the user with the given name
	User* GetUserByName( const std::string &sName ) const;

	/* returns a std::string expressing the user's current state */
	std::string GetUserState( const User *user ) const;

	// no non-const version because no functions should need it
	const std::list<User*>* GetUserList() const	{ return &m_Users; }

	RoomList* GetRoomList()	{ return &m_Rooms; }
	const RoomList* GetRoomList() const { return &m_Rooms; }

	/* sends a system message to all mods on the server */
	void WallMessage( const std::string &sMessage );

	/* sends a packet to all users on the server */
	void Broadcast( const ChatPacket &packet );

	/* main processing loop */
	void MainLoop();

	DatabaseConnector* GetConnection() { return m_pConnector; }

private:
	/* given iSocket, creates a user with that socket */
	void AddUser( unsigned iSocket );

	/* disconnects the given user from the server */
	void RemoveUser( User *user );

	/* performs an update cycle on the given user */
	void UpdateUser( User *user );

	/* handles a packet received from user */
	void HandleUserPacket( User *user, const std::string &in );

	/* handles the login status of a user */
	void HandleLoginState( User *user );

	/* checks the idle statistics of a user, broadcasts if needed */
	void CheckIdleStatus( User *user );

	/* instance buffer for reading packet data from a user */
	char m_sReadBuffer[BUFFER_SIZE];

	/* handles verifying accounts and config save/load */
	DatabaseConnector *m_pConnector;

	/* listens for connections on the given port */
	SocketListener *m_pListener;

	/* handles all room logic */
	RoomList m_Rooms;

	/* set of all users being updated */
	std::list<User*> m_Users;
};

#endif // CHAT_SERVER_H
