/* DatabaseWorker: handles network requests in a separate thread. */

#include <string>
#include <queue>

#include "network/Socket.h"
#include "util/Thread.h"

class Config;
class User;
struct Request;

const unsigned HTTP_BUFFER_SIZE = 1024;

class DatabaseWorker
{
	// We don't allow any direct access to these methods.
	// They must go through the Connector only.
	friend class DatabaseConnector;
protected:
	/* We pass a config object from which the worker can load data */
	DatabaseWorker( const Config *cfg );
	~DatabaseWorker();

	// kill the worker thread
	void Stop();

	bool IsConnected() const { return m_Socket.IsOpen(); }

	/* makes a new Request* and pushes it onto the queue. */
	void Login( User *user, const std::string &passwd );
	void SavePrefs( const User *user );

	// if they're banned, we don't have the name, so...
	void Ban( const std::string &username );
	void Unban( const std::string &username );

private:
	// shortcuts for handling the connection
	bool Connect();
	void Disconnect();

	// thread-safe calls for request manipulation
	void AddRequest( Request *req );
	Request* PopRequest();

	// internal handlers for requests
	void DoLogin( Request *req );
	void DoSavePrefs( Request *req );
	void DoBan( Request *req, bool bBan );

	/* gets configuration for this user. Called during Login. */
	void DoLoadPrefs( User *user );

	/* sends a request with URL-encoded parameters, returns response */
	const char* SendPOST( const std::string &url, const std::string &params );

	// workaround for member function and normal function thread
	static void *Start( void *p ) { ((DatabaseWorker*)p)->HandleRequests(); return NULL; }
	void HandleRequests();

	// paths for the POST recipients we use for verification
	std::string m_sServer, m_sAuthPage, m_sConfigPage, m_sBanPage;

	// default configuration to be loaded if the server can't find any
	std::string m_sDefaultConfig;

	/* requests that are handled by the request thread */
	std::queue<Request*> m_Requests;

	/* stores HTTP read data */
	char m_sBuffer[HTTP_BUFFER_SIZE];

	/* Only allow one system to access the queue at a time. */
	Spinlock m_QueueLock;

	/* true while the thread is still running. */
	bool m_bRunning;

	/* read and write timeouts for database connections */
	int m_iReadTimeout, m_iWriteTimeout;

	Socket m_Socket;
	Thread m_Thread;
};

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
