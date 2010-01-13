#include "User.h"
#include "logger/Logger.h"
#include "packet/ChatPacket.h"	// XXX
#include <cerrno>

User::User( unsigned iSocket ) : m_iSocket(iSocket), m_sName("<no name>"),
	m_sRoom(DEFAULT_ROOM)
{
	m_cLevel = '_';
	m_bLoggedIn = m_bMuted = m_bAway = false;
	m_LastMessage = time_t(NULL);
}

User::~User()
{

}

bool User::IsMod() const
{
	for( unsigned i = 0; i < NUM_MOD_LEVELS; i++ )
		if( m_cLevel == MOD_LEVELS[i] )
			return true;

	return false;
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
	return GetElapsedSeconds( m_LastMessage );
}

unsigned User::GetIdleBroadcastSeconds() const
{
	return GetElapsedSeconds( m_LastIdleBroadcast );
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
