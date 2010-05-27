/* ChatServer: the main server file. Handles all users, message broadcasting,
 * that sort of thing. */

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <list>
#include <vector>
#include <string>
#include "network/SocketListener.h"
#include "model/RoomList.h"

class ChatPacket;
class Config;
class DatabaseConnector;
class User;

// 1024 = 1 KB, so 4 KB
const unsigned BUFFER_SIZE = 1024*4;

class ChatServer
{
public:
	ChatServer();
	~ChatServer();

	/* sets configuration */
	void SetConfig( Config *cfg )	{ m_pConfig = cfg; }

	/* loads preferences and starts up the network server */
	void Start();

	/* tells the main server loop to stop running */
	void Stop();

	// returns a reference to the user with the given name
	User* GetUserByName( const std::string &sName ) const;

	/* returns a std::string expressing the user's current state */
	std::string GetUserState( const User *user ) const;

	/* sends a system message to all mods on the server */
	void WallMessage( const std::string &sMessage );

	/* sends a packet to all users on the server */
	void Broadcast( const ChatPacket &packet );

	/* main processing loop */
	void MainLoop();

	/* returns true if we're listening for clients */
	bool IsListening() const { return m_pListener != NULL && m_pListener->IsConnected(); }

	// no non-const version because no functions should need it
	const std::list<User*>* GetUserList() const	{ return &m_Users; }

	RoomList* GetRoomList()	{ return m_pRooms; }
	const RoomList* GetRoomList() const { return m_pRooms; }

	/* retrieves a pointer to the database connector for login/prefs */
	DatabaseConnector* GetConnection() { return m_pConnector; }

protected:
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

private:
	/* true as long as the server is running */
	bool m_bRunning;

	/* instance buffer for reading packet data from a user */
	char m_sReadBuffer[BUFFER_SIZE];

	/* handles verifying accounts and config save/load */
	DatabaseConnector *m_pConnector;

	/* listens for connections on the given port */
	SocketListener *m_pListener;

	/* handles configuration for server logic */
	Config *m_pConfig;

	/* handles all room logic */
	RoomList *m_pRooms;

	/* set of all users being updated */
	std::list<User*> m_Users;

	/* set of muted users that should stay muted between logins. */
	std::vector<std::string> m_MutedUsers;

	/* list of all user levels that define moderators. Stored
	 * as a string since that's easier (find_first_of)... */
	std::string m_sModLevels;

	/* user level that defines a banned user */
	char m_cBanLevel;
};

#endif // CHAT_SERVER_H

/* 
 * Copyright (c) 2009 Mark Cannon ("Vyhd")
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
