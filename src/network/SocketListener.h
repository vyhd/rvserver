/* SocketListener: Listens for incoming connections and passes them to an
 * external handler (with the GetConnection() call) for processing. */

#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

class ChatServer;

class SocketListener
{
public:
	SocketListener();
	~SocketListener();

	/* Attempt to start listening on iPort, passing clients to pServer. */
	bool Connect( int iPort );
	void Disconnect();

	bool IsConnected() const { return m_iServerSocket > 0; }

	/* Returns a socket fd, or -1 if none is available (no new connections). */
	int GetConnection();

private:
	/* Server socket IDs */
	int m_iServerSocket;

	/* port we're listening on */
	int m_iPort;

};

#endif // SOCKETLISTENER_H

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
