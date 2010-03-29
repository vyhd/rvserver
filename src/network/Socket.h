/* Wrapper class for a socket and associated operations. This way, we can
 * abstract network operations and keep low-level stuff out of other files.
 */
#ifndef SOCKET_H
#define SOCKET_H

#include <string>

class Socket
{
public:
	Socket() : m_iSocket(-1) { }
	Socket( int socket ) : m_iSocket(socket) { }

	const char* GetIP() const;

	// name difference is needed: same arg types, can't overload
	bool OpenHost( const std::string &host, int port );
	bool Open( const std::string &ip, int port );
	void Close();

	bool IsOpen() const	{ return m_iSocket > 0; }

	int Read( char *buffer, unsigned len, bool bDontWait = true );
	int Write( const char *buffer, unsigned len, bool bDontWait = true );
	int Write( const std::string &str, bool bDontWait = true );

private:
	int m_iSocket;
};

#endif // SOCKET_H

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
