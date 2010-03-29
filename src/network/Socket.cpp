#include <cerrno>

#include <arpa/inet.h>
#include <netdb.h>

#include "Socket.h"
#include "logger/Logger.h"

const char* Socket::GetIP() const
{
	struct sockaddr_in sin;
	socklen_t len = sizeof( sin );

	// if the socket has been disconnected already by the OS, we
	// aren't gonna get the IP (which sucks a bit).
	if( getpeername( m_iSocket, (sockaddr *)&sin, &len) < 0 )
		return "<nil>";

	return inet_ntoa(sin.sin_addr);
}

bool Socket::OpenHost( const std::string &host, int port )
{
	const struct hostent *entry = gethostbyname( host.c_str() );

	if( entry == NULL )
	{
		Logger::SystemLog( "Lookup of \"%s\" failed: %u\n", host.c_str(), h_errno );
		return false;
	}

	/* just dereference the entry and call the IP socket maker. */
	const std::string ip = inet_ntoa( *(struct in_addr*)entry->h_addr );

	return Open( ip, port );
}

bool Socket::Open( const std::string &ip, int port )
{
	m_iSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if( m_iSocket < 0 )
	{
		Logger::SystemLog( "Open: Failed to open socket!\n" );
		return false;
	}

	sockaddr_in sin;
	memset( &sin, 0, sizeof(sin) );

	sin.sin_family = AF_INET;
	sin.sin_port = htons( port );

	// convert the IP string to our in_addr
	inet_aton( ip.c_str(), &sin.sin_addr );

	if( connect(m_iSocket, (sockaddr*)&sin, sizeof(sin)) == -1 )
	{
		printf( "MakeSocket: Failed to connect to %s!\n", ip.c_str() );
		return false;
	}

	return true;
}

void Socket::Close()
{
	shutdown( m_iSocket, SHUT_RDWR );
	close( m_iSocket );
	m_iSocket = -1;
}

int Socket::Read( char *buffer, unsigned len, bool bDontWait )
{
	int flags = (bDontWait) ? MSG_DONTWAIT : 0;
	int iRead = recv( m_iSocket, buffer, len, flags );

	if( iRead <= 0 )
	{
		// ignore and return as an error
		if( errno == EAGAIN || errno == EWOULDBLOCK )
			return 0;

		Logger::DebugLog( "Read( %u, %p, %u, %d ) failed: %i (%s)",
			m_iSocket, buffer, len, int(bDontWait), errno, strerror(errno) );

		return -1;
	}

	return iRead;
}

int Socket::Write( const char *buffer, unsigned len, bool bDontWait )
{
	const int flags = bDontWait ? MSG_DONTWAIT : 0;
	int iSent = send( m_iSocket, buffer, len, flags );

	if( iSent <= 0 )
	{
		// ignore and return
		if( errno == EAGAIN || errno == EWOULDBLOCK )
			return 0;

		Logger::DebugLog( "Write( %u, %p, %u, %d ) failed: %i (%s)",
			m_iSocket, buffer, len, int(bDontWait), errno, strerror(errno) );

		return -1;	
	}

	return iSent;
}

int Socket::Write( const std::string &str, bool bDontWait )
{
	return Write( str.c_str(), str.length(), bDontWait );
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
