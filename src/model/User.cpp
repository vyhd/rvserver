#include "User.h"
#include "Room.h"
#include "logger/Logger.h"
#include <cerrno>

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

bool User::IsMod() const
{
	switch( m_cLevel )
	{
		case 'A':
		case 'C':
		case 'c':
		case 'b':
		case 'f':
		case '!':
			return true;
		default:
			return false;
	}
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
		Logger::SystemLog( "Write failed for %s (%s): killing.", m_sName.c_str(), strerror(errno) );
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
		Logger::SystemLog( "Read failed for %s (%s): killing.", m_sName.c_str(), strerror(errno) );
		m_Socket.Close();
		return -1;
	}

	return iRead;
}
