#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <vector>	// for PacketUtil::Split
#include <string>	// for memset

#include <unistd.h>
#include <sys/time.h>	// for timestamping

#include "ChatServer.h"
#include "logger/Logger.h"
#include "model/Room.h"
#include "model/User.h"
#include "network/DatabaseConnector.h"
#include "packet/ChatPacket.h"
#include "packet/PacketUtil.h"
#include "packet/PacketHandler.h"
#include "util/Config.h"
#include "util/StringUtil.h"
#include "verinfo.h"	// for BUILD_DATE, BUILD_VERSION

using namespace std;

ChatServer::ChatServer() : m_pListener(NULL)
{
	m_pListener = new SocketListener;
}

ChatServer::~ChatServer()
{
	Stop();

	if( m_pListener )
	{
		delete m_pListener;
		m_pListener = NULL;
	}

	if( m_pConnector )
	{
		delete m_pConnector;
		m_pConnector = NULL;
	}
}

void ChatServer::Start()
{
	if( m_pConfig == NULL )
	{
		LOG->Debug( "ChatServer::Start: cannot start without configuration!" );
		return;
	}

	if( m_pListener->IsConnected() )
		m_pListener->Disconnect();

	// connect to the port specified in the configuration
	m_pListener->Connect( m_pConfig->GetInt("ServerPort") );

	// initialize the database connector if we haven't
	if( !m_pConnector )
		m_pConnector = new DatabaseConnector( m_pConfig );

	// Remove the RoomList, if it exists, and re-create it
	if( m_pRooms )
		delete m_pRooms;

	m_pRooms = new RoomList( m_pConfig );

	// check for, and initialize, additional rooms
	const char* EXTRA_ROOMS	= m_pConfig->Get( "AdditionalRooms", true );

	if( EXTRA_ROOMS )
	{
		vector<string> vsRooms;
		StringUtil::Split( EXTRA_ROOMS, vsRooms, ',' );

		for( unsigned i = 0; i < vsRooms.size(); ++i )
			m_pRooms->AddRoom( vsRooms[i] );
	}

	// set up server-side user level stuff.
	// TODO: synchronization mechanism between database and server?
	const char* sModLevels = m_pConfig->Get( "ModLevels" );
	const char* sBanLevel = m_pConfig->Get( "BanLevel" );

	m_sModLevels.assign( sModLevels );
	m_cBanLevel = sBanLevel[0];

	m_bRunning = true;
}

void ChatServer::Stop()
{
	m_bRunning = false;

	// remove all users
	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); ++it )
		RemoveUser( *it );

	m_Users.clear();

	// wipe all the rooms except the default room
	m_pRooms->ClearRooms();

	m_pListener->Disconnect();
}

void ChatServer::AddUser( unsigned iSocket )
{
	User *pUser = new User( iSocket );
	m_Users.push_back( pUser );

	LOG->Debug( "Added new client on socket %d, from IP %s", iSocket, pUser->GetIP() );
}

void ChatServer::RemoveUser( User *user )
{
	LOG->Debug( "Client (%p) at IP %s removed.", user, user->GetIP() );

	// broadcast a quitting message if they were logged in
	if( user->IsLoggedIn() )
	{
		user->SetLoggedIn( false );
		Broadcast( ChatPacket(USER_PART, user->GetName(), BLANK) );

		if( !user->GetName().empty() )
			LOG->Debug( "Saving prefs for %s", user->GetName().c_str() );

		// save this user's preferences
		m_pConnector->SavePrefs( user );
	}

	user->Kill();

	// take this user out of the RoomList
	m_pRooms->RemoveUser( user );

	delete user;
}

void ChatServer::MainLoop()
{
	// no listener means no connection; no connection means no server.
	if( m_pListener == NULL )
		return;

	// set some optional configuration: SleepTime is the amount of usecs to wait
	// between updates, LagSpikeTime is the amount of usecs required to pass within
	// an update to be considered a spike (which results in a message to stdout).

	const unsigned iSleepTime = m_pConfig->GetInt( "SleepTime", true, 1000*150 );	// 150 ms
	const unsigned iLagSpikeTime = m_pConfig->GetInt( "LagSpikeTime", true, 250 );	// 250 us

	struct timeval tv_start, tv_end;

	while( true )
	{
		// If we're not running, then keep looping (lazily) until we are.
		// We don't actually want to stop this function; it's only called once.
		if( !m_bRunning )
		{
			sleep( 1 );
			continue;
		}

		gettimeofday( &tv_start, NULL );

		// see if the SocketListener has any new connections and add them.
		{
			int iSocket = m_pListener->GetConnection();

			if( iSocket > 0 )
				AddUser( iSocket );
		}

		// loop over all the clients and update them as needed
		for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); ++it )
		{
			User *user = (*it);

			// if the user isn't logged in, check login status
			if( !user->IsLoggedIn() )
			{
				const LoginState state = user->GetLoginState();

				// we're not expecting any data just yet.
				if( state == LOGIN_CHECKING )
					continue;

				// if the user's login completed, check it.
				if( state != LOGIN_NONE )
					HandleLoginState( user );
			}

			// update this user (which means checking for and
			// handling packets) regardless of login status.
			UpdateUser( user );

			// users can't be idle unless they're logged in...
			if( user->IsLoggedIn() )
				CheckIdleStatus( user );

			// if this user is dead, erase/reposition the iterator
			// and properly remove the user from rooms, etc.
			if( user->IsDead() )
			{
				it = m_Users.erase( it );
				RemoveUser( user );
			}

		}

		// flush all the logs to disk on update
		LOG->Flush();

		// run some basic lag-detection logic
		{
			gettimeofday( &tv_end, NULL );
			unsigned iDiff = 1000000 * (tv_end.tv_sec-tv_start.tv_sec) + (tv_end.tv_usec-tv_start.tv_usec);

			if( iDiff >= iLagSpikeTime )
				LOG->Debug( "[MainLoop took %u usecs to execute.]\n", iDiff );
		}

		// give a bunch of time to other processes
		usleep( iSleepTime );
	}

	LOG->System( "The impossible happened! :(" );
}

void ChatServer::UpdateUser( User *user )
{
	unsigned iPos = 0;

	memset( m_sReadBuffer, '\0', BUFFER_SIZE );

	// no new data to operate on, or an error occurred
	if( (iPos = user->Read(m_sReadBuffer, BUFFER_SIZE)) <= 0 )
		return;

	// the vast majority of cases won't need split, so we
	// can optimize the branching logic around that case.
	std::string sBuffer( m_sReadBuffer );

	if( !PacketUtil::NeedsSplit(sBuffer) )
	{
		// only one packet: handle and be on our way.
		HandleUserPacket( user, sBuffer );
	}
	else
	{
		// we have multiple packets. split them and handle each.
		vector<std::string> vPackets;
		PacketUtil::Split( sBuffer, vPackets );

		for( unsigned i = 0; i < vPackets.size(); ++i )
			HandleUserPacket( user, vPackets[i] );
	}
}

void ChatServer::HandleUserPacket( User *user, const std::string &buf )
{
	// create user-specific log prefix, e.g. "Fire_Adept@127.0.0.1"
	const string sUserPrefix = StringUtil::Format( "%s@%s", user->GetName().c_str(), user->GetIP() );

	// try to make a packet out of the data we've gotten
	ChatPacket packet( buf );

	// if the packet can't be parsed, drop the client.
	if( !packet.IsValid() )
	{
		LOG->Debug( "invalid packet from %s! %s", sUserPrefix.c_str(), buf.c_str() );
		LOG->Debug( "packet data: %s", buf.c_str() );
		user->Kill();
		return;
	}

	// broadcast a returned message if the user was idle or away before.
	if( user->IsLoggedIn() && (user->IsIdle() || user->IsAway()) )
		Broadcast( ChatPacket(CLIENT_BACK, user->GetName(), BLANK) );

	// update idle/away and last message timestamp
	user->PacketSent();

	// don't log packets that weren't actually handled
	if( !PacketHandler::Handle(this, user, &packet) )
		return;

	// If we have a login packet, wipe the password from the log.
	if( packet.iCode == USER_JOIN )
		packet.sMessage = "[censored]";

	/* write this packet to the log, including the user prefix, e.g.
	 * Fire_Adept@192.168.1.1	3`_`hey sup d00dz`3`13`37
	 */
	string sLogLine = StringUtil::Format( "%s\t%s",
		sUserPrefix.c_str(), packet.ToString().c_str() );

	LOG->Chat( sLogLine.c_str() );
}	

void ChatServer::CheckIdleStatus( User *user )
{
	// no need to update idle status here
	if( !user->IsIdle() )
		return;

	// user has been away for too long, so kick 'em
	if( user->IsInert() )
	{
		user->Write( ChatPacket(IDLE_KICK).ToString() );
		user->Kill();
		return;
	}

	const unsigned iIdleMinutes = user->GetIdleMinutes();

	// do we need to broadcast an update message? (every minute)
	if( user->GetLastIdleMinute() == iIdleMinutes )
		return;

	// print the idle time into a string, broadcast it
	string sIdleTime = StringUtil::Format( "%04u", iIdleMinutes );
	Broadcast( ChatPacket(CLIENT_IDLE, user->GetName(), sIdleTime) );

	// update the user's last idle broadcast timestamp
	user->UpdateLastIdle();
}

void ChatServer::HandleLoginState( User *user )
{
	/* dispatches messages to the user and/or server, as appropriate */

	switch( user->GetLoginState() )
	{
	case LOGIN_ERROR:
		user->Write( ChatPacket(ACCESS_DENIED).ToString() );
		break;
	case LOGIN_ERROR_ATTEMPTS:
		user->Write( ChatPacket(LIMIT_REACHED).ToString() );
		break;
	case LOGIN_SERVER_DOWN:
		user->Write( ChatPacket(SERVER_DOWN).ToString() );
		break;
	case LOGIN_SUCCESS:
	{
		// if this user is banned, send them a message and cut the connection
		if( user->GetLevel() == m_cBanLevel )
		{
			user->Write( ChatPacket(USER_BAN, BLANK, BLANK).ToString() );
			user->Kill();
			return;
		}

		// if this user has a mod level, set them as mod
		if( m_sModLevels.find(user->GetLevel()) != string::npos )
			user->SetMod( true );

		// write 'accepted' response
		user->Write( ChatPacket(ACCESS_GRANTED).ToString() );

		// write configuration (which was set by the login request)
		ChatPacket prefs(CLIENT_CONFIG, BLANK, user->GetPrefs() );
		user->Write( prefs.ToString() );

		m_pRooms->GetDefaultRoom()->AddUser( user );
		user->SetLoggedIn( true );

		// send the new guy a nice little version message
		std::string ver = StringUtil::Format( "Server build %u, "
			"compiled %s", BUILD_VERSION, BUILD_DATE );

		user->Write( ChatPacket(WALL_MESSAGE, BLANK, ver).ToString() );

		// tell everyone that this user joined
		ChatPacket msg( USER_JOIN, user->GetName(), GetUserState(user) );
		Broadcast( msg );

		break;
	}
	// these states aren't handled here and should never be reached
	case LOGIN_NONE:
	case LOGIN_CHECKING:
		LOG->System( "I'a Dagon! State %i", user->GetLoginState() );
		break;
	}

	// unless the user successfully logged in, kill the connection.
	if( user->GetLoginState()  != LOGIN_SUCCESS )
		user->Kill();
}

User* ChatServer::GetUserByName( const std::string &sName ) const
{
	// XXX: always a linear search. Can we improve on that?
	// (probably not, we don't have a high enough user load to justify it)
	for( list<User*>::const_iterator it = m_Users.begin(); it != m_Users.end(); ++it )
		if( !StringUtil::CompareNoCase((*it)->GetName(), sName) )
			return (*it);

	// no match found
	return NULL;
}

std::string ChatServer::GetUserState( const User *user ) const
{
	const string sRoom = m_pRooms->GetName( user->GetRoom() );
	const char cLevel = user->GetLevel();
	const char cMuted = user->IsMuted() ? 'M' : '_';
	string sIdle( BLANK ), sAway( BLANK );

	if( user->IsIdle() )
		sIdle = StringUtil::Format( "%i04u", user->GetIdleMinutes() );
	if( user->IsAway() )
		sAway = StringUtil::Format( "a%s", user->GetMessage().c_str() );

	return StringUtil::Format( "%s|%c%c%s%s", sRoom.c_str(),
		cLevel, cMuted, sIdle.c_str(), sAway.c_str() );
}

void ChatServer::Broadcast( const ChatPacket &packet )
{
	// optimization: instead of using Send(), cache the packet string and
	// Write(). we only need ToString (which is expensive) once this way.
	const std::string sPacketData = packet.ToString();

	// send to every single user on the server
	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); ++it )
	{
		User *user = (*it);

		// don't bother broadcasting to users who won't see it
		if( !user->IsLoggedIn() )
			continue;

		user->Write( sPacketData );
	}
}

void ChatServer::WallMessage( const std::string &sMessage )
{
	const std::string sPacket = ChatPacket(WALL_MESSAGE, BLANK, sMessage).ToString();

	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); ++it )
	{
		User *user = (*it);

		if( !user->IsMod() )
			continue;

		user->Write( sPacket );
	}
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
