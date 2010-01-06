#include "User.h"
#include "logger/Logger.h"
#include "packet/ChatPacket.h"	// XXX
#include <cerrno>

User::User( unsigned iSocket ) : m_iSocket(iSocket), m_sName("<no name>"),
	m_sRoom(DEFAULT_ROOM)
{
	m_bLoggedIn = m_bMuted = m_bAway = m_bIsMod = false;
	m_LastMessage = time_t(NULL);
}

User::~User()
{

}

/* returns idle time, in seconds */
static unsigned GetElapsed( time_t then )
{
	time_t now = time(NULL);
	double diff = difftime( now, then );
	return unsigned(diff);
}

unsigned User::GetIdleTime() const
{
	return GetElapsed( m_LastMessage );
}

unsigned User::GetLastIdleBroadcast() const
{
	return GetElapsed( m_LastIdleBroadcast );
}

void User::PacketSent()
{
	if( m_bAway )
	{
		m_bAway = false;
		m_sMessage.clear();
	}

	// update last packet time
	m_LastMessage = time(NULL);
}
