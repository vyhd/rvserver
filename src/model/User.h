/* User: contains information on a user, including current status,
 * idle/away time, current room, and network socket (for convenience).
 */

#ifndef USER_H
#define USER_H

/* This is susceptible to the Year 2038 Problem. Oh well.
 * We'll hopefully have replaced it by then, don't you think? */
#include <ctime>
#include <string>
#include "network/Socket.h"

class Room;

/* primitive synchronization state: users have a LoginState. We can use
 * this to (indirectly) communicate between the ChatServer and the database
 * worker. TODO: make this a better architecture.
 */
enum LoginState
{
	LOGIN_NONE,		/* login not attempted yet */
	LOGIN_CHECKING,		/* login checking: don't reap! */
	LOGIN_SUCCESS,		/* user has been verified. */	
	LOGIN_ERROR,		/* verification failed: bad credentials */
	LOGIN_ERROR_ATTEMPTS,	/* login failed: too many attempts. */
	LOGIN_SERVER_DOWN	/* login failed: no response from server. */
};

/* idle time limits */
const unsigned MINUTES_TO_IDLE = 5;
const unsigned MINUTES_TO_IDLE_KICK = 90;

class User
{
	// We only let Room call SetRoom(), for consistency.
	friend class Room;

public:
	User( unsigned iSocket );
	~User();

	// force the user to quit, e.g. failed validation or kicked.
	void Kill() { m_Socket.Close(); }

	// if this is true, reap the user when possible.
	bool IsDead() const
	{
		return !m_Socket.IsOpen() && m_LoginState != LOGIN_CHECKING;
	}

	// one part convenience, one part error detection
	int Read( char *buffer, unsigned len );
	int Write( const std::string &str );
	const char* GetIP() const { return m_Socket.GetIP(); }

	/* get/set login state */
	LoginState GetLoginState() const	{ return m_LoginState; }
	void SetLoginState( LoginState s )	{ m_LoginState = s; }

	/* set/get user properties */
	bool IsAway() const	{ return m_bAway; }
	bool IsMuted() const	{ return m_bMuted; }
	bool IsLoggedIn() const	{ return m_bLoggedIn; }

	void SetAway( bool b )		{ m_bAway = b; }
	void SetMuted( bool b )		{ m_bMuted = b; }
	void SetLoggedIn( bool b )	{ m_bLoggedIn = b; }

	char GetLevel() const	{ return m_cLevel; }
	void SetLevel( char c )	{ m_cLevel = c; }
	bool IsMod() const	{ return m_bIsMod; }
	void SetMod( bool b )	{ m_bIsMod = b; }

	Room* GetRoom() const	{ return m_pRoom; }

	/* get name/away/room/prefs */
	const std::string& GetName() const	{ return m_sName; }
	const std::string& GetMessage() const	{ return m_sMessage; }
	const std::string& GetPrefs() const	{ return m_sPrefs; }

	/* set name/away/room */
	void SetName( const std::string &str )	{ m_sName.assign( str ); }
	void SetMessage(const std::string &str)	{ m_sMessage.assign( str ); }
	void SetPrefs( const std::string &str )	{ m_sPrefs.assign( str ); }

	/* get idle status and time from last message */
	unsigned GetIdleSeconds() const;
	unsigned GetIdleMinutes() const	{ return GetIdleSeconds() / 60; }

	/* resets idle/away statuses and the last message timer */
	void PacketSent();

	// last time the user's idle status was acknowledged
	unsigned GetLastIdleMinute() const { return m_iLastIdleMinute; }
	void UpdateLastIdle() { m_iLastIdleMinute = GetIdleMinutes(); }

	bool IsIdle() const { return GetIdleMinutes() >= MINUTES_TO_IDLE; }
	// *boot*
	bool IsInert() const { return GetIdleMinutes() >= MINUTES_TO_IDLE_KICK; }

private:
	void SetRoom( Room* p )	{ m_pRoom = p; }

	/* Socket descriptor for this user's connection */
	Socket m_Socket;

	/* Room this user is suscribed to */
	Room *m_pRoom;

	/* away status and message, if applicable */
	bool m_bAway;
	std::string m_sMessage;

	/* basic user details */
	std::string m_sName, m_sPrefs;
	char m_cLevel;
	bool m_bLoggedIn, m_bMuted, m_bIsMod;

	unsigned m_iLastIdleMinute;
	time_t m_LastActive;

	LoginState m_LoginState;
};

#endif // USER_H

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
