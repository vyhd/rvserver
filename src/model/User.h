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

/* character codes indicating moderators and the number thereof */
const char MOD_LEVELS[] = { 'A', 'C', 'b', 'f', 'c', '!' };
const unsigned NUM_MOD_LEVELS = sizeof(MOD_LEVELS)/sizeof(MOD_LEVELS[0]);

/* idle time limits */
const unsigned MINUTES_TO_IDLE = 5;
const unsigned MINUTES_TO_IDLE_KICK = 90;

class User
{
public:
	User( unsigned iSocket );
	~User();

	// one part convenience, one part error detection
	int Write( const std::string &str );
	int Read( char *buffer, unsigned len );
	const char* GetIP() const	{ return m_Socket.GetIP(); }

	// if this is true, the socket closed due to bad communication.
	// we'll reap the user after the main update loop is done.
	bool IsDead() const	{ return !m_Socket.IsOpen(); }

	// force the user to quit, e.g. failed validation or kicked
	void Kill()		{ m_Socket.Close(); }

	/* set/get user properties */

	bool IsAway() const	{ return m_bAway; }
	bool IsMuted() const	{ return m_bMuted; }
	bool IsLoggedIn() const	{ return m_bLoggedIn; }

	void SetAway( bool b )		{ m_bAway = b; }
	void SetMuted( bool b )		{ m_bMuted = b; }
	void SetLoggedIn( bool b )	{ m_bLoggedIn = b; }

	char GetLevel() const	{ return m_cLevel; }
	void SetLevel( char c )	{ m_cLevel = c; }
	bool IsMod() const;

	Room* GetRoom() const	{ return m_pRoom; }
	void SetRoom( Room* p )	{ m_pRoom = p; }

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

	bool IsIdle() const { return GetIdleMinutes() >= 5; }
	

	/* resets idle/away statuses and the last message timer */
	void PacketSent();

private:
	/* Socket descriptor for this user's connection */
	Socket m_Socket;

	/* Room this user is suscribed to */
	Room *m_pRoom;

	/* away status and message, if applicable */
	bool m_bAway;
	std::string m_sMessage;

	/* basic user details */
	std::string m_sName;
	std::string m_sPrefs;

	char m_cLevel;

	bool m_bLoggedIn;
	bool m_bMuted;

	time_t m_LastActive;
};

#endif // USER_H
