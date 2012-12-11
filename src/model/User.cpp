#include "User.h"
#include "Room.h"
#include "logger/Logger.h"
#include <cerrno>
#include <cstring>

unsigned User::s_iIdleMinutes;
unsigned User::s_iKickMinutes;

User::User( unsigned iSocket ) : m_Socket(iSocket), m_sName("<no name>")
{
	m_pRoom = NULL;
	m_cLevel = '_';
	m_bLoggedIn = m_bMuted = m_bAway = false;
	m_LastActive = time_t(NULL);
	m_LoginState = LOGIN_NONE;
}

User::~User()
{
	m_Socket.Close();
}

/* returns time from then to now, in seconds */
static unsigned GetElapsedSeconds( time_t then )
{
	time_t now = time(NULL);
	double diff = difftime( now, then );
	return unsigned(diff);
}

unsigned User::GetIdleSeconds() const
{
	return GetElapsedSeconds( m_LastActive );
}

void User::PacketSent()
{
	if( m_bAway )
	{
		m_bAway = false;
		m_sMessage.clear();
	}

	// update last packet time
	m_LastActive = time(NULL);
}

int User::Write( const std::string &str )
{
	if( !m_Socket.IsOpen() )
		return -1;

	int iSent = m_Socket.Write( str );

	if( iSent < 0 )
	{
		LOG->System( "Write failed for %s (%s): killing.", m_sName.c_str(), strerror(errno) );
		m_Socket.Close();
		return -1;
	}

	return iSent;
}

int User::Read( char *buffer, unsigned len )
{
	if( !m_Socket.IsOpen() )
		return -1;

	int iRead = m_Socket.Read( buffer, len );

	if( iRead < 0 )
	{
		LOG->System( "Read failed for %s (%s): killing.", m_sName.c_str(), strerror(errno) );
		m_Socket.Close();
		return -1;
	}

	return iRead;
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
