#include <cstdio>
#include <cstring>	// XXX: needed?
#include <cstdlib>	// XXX: needed?
#include <cerrno>

// networking types
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>	// for close()
#include <fcntl.h>	// for fcntl()

#include "network/SocketListener.h"
#include "logger/Logger.h"

SocketListener::SocketListener()
{
	m_iServerSocket = -1;
}

SocketListener::~SocketListener()
{
	Logger::DebugLog( "SocketListener::~SocketListener" );
	Disconnect();
}

void SocketListener::Disconnect()
{		
	// this can be called safely whether it's open or not.
	// if it is, we'll close; if not, we ignore the error.
	shutdown( m_iServerSocket, SHUT_RDWR );
	close( m_iServerSocket );
	m_iServerSocket = -1;
}

bool SocketListener::Connect( int iPort )
{
	Logger::DebugLog( "SocketListener::Connect( %d )", iPort );

	// Let Listen() know what port we're running on.
	m_iPort = iPort;

	// Create the server socket.
	if( ( m_iServerSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
	{
		Logger::SystemLog( "Error creating ServerSocket: %s\n", strerror(errno) );
		return false;
	}

	// set the socket non-blocking, so accept doesn't block. we need this
	// in order to manually poll for new clients.
	fcntl( m_iServerSocket, F_SETFL, O_NONBLOCK );

	// Set up the server socket
	struct sockaddr_in m_SockAddr;

	memset( &m_SockAddr, 0, sizeof(m_SockAddr) );
	m_SockAddr.sin_family = AF_INET;

	// TODO: Make this dynamic (Bind to a specific address, etc)
	m_SockAddr.sin_addr.s_addr = inet_addr( "0.0.0.0" );
	m_SockAddr.sin_port = htons( iPort );
	
	/* allow us to reuse this address even if the socket isn't immediately cleaned up. */
	int iSetOpt = 1;
	setsockopt( m_iServerSocket, SOL_SOCKET, SO_REUSEADDR, &iSetOpt, sizeof(int) );
	
	// Bind the ServerSocket
	if( bind( m_iServerSocket, (sockaddr *)&m_SockAddr, sizeof( m_SockAddr ) ) < 0 )
	{
		Logger::SystemLog( "Error binding ServerSocket: %s\n", strerror(errno) );
		return false;
	}
	
	// Set the ServerSocket to listen for a connection
	if( listen( m_iServerSocket, 5 ) < 0 )
	{
		Logger::SystemLog( "Error listening for users: %s\n", strerror(errno) );
		return false;
	}

	return true;
}

int SocketListener::GetConnection()
{
	struct sockaddr_in ClientData;
	socklen_t len = sizeof( ClientData );

	int iClientSocket = accept( m_iServerSocket, (sockaddr*)&ClientData, &len );

	if( iClientSocket < 0 )
	{
		// expected errors: ignore them and continue
		if( errno == EAGAIN || errno == EWOULDBLOCK )
			return -1;

		// unexpected: log a warning, then continue
		Logger::SystemLog( "Error accepting %s on port %d: %s\n", inet_ntoa(ClientData.sin_addr),
			m_iPort, strerror(errno) );

		return -1;
	}

	// we have a valid socket!
	return iClientSocket;
}

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
