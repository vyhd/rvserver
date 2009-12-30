#include "User.h"
#include "logger/Logger.h"
#include "packet/ChatPacket.h"	// XXX
#include <cerrno>

/* Number of seconds before 'idle' status is set. */
const time_t SECONDS_TO_IDLE = (60 * 30);

User::User( unsigned iSocket ) : m_iSocket(iSocket), m_sName("<no name>")
{
	m_bLoggedIn = m_bMuted = m_bIdle = m_bAway = m_bIsMod = false;

}

User::~User()
{

}

void User::SetBack()
{
	m_bAway = false;
	m_bIdle = false;

	m_sMessage.clear();
}
