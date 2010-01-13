/* User: contains information on a user, including current status,
 * idle/away time, current room, and network socket (for convenience).
 */

#ifndef USER_H
#define USER_H

/* This is susceptible to the Year 2038 Problem. Oh well.
 * We'll hopefully have replaced it by then, don't you think? */
#include <ctime>
#include <string>

class ChatServer;

/* room to which all users are automatically joined */
const std::string DEFAULT_ROOM = "RV Chat";

/* character codes indicating moderators and the number thereof */
const char MOD_LEVELS[] = { 'A', 'C', 'b', 'f', 'c' };
const unsigned NUM_MOD_LEVELS = sizeof(MOD_LEVELS)/sizeof(MOD_LEVELS[0]);

class User
{
public:
	User( unsigned iSocket );
	~User();

	int GetSocket() const	{ return m_iSocket; }

	bool IsAway() const	{ return m_bAway; }
	bool IsMuted() const	{ return m_bMuted; }
	bool IsLoggedIn() const	{ return m_bLoggedIn; }

	void SetAway( bool b )		{ m_bAway = b; }
	void SetMuted( bool b )		{ m_bMuted = b; }
	void SetLoggedIn( bool b )	{ m_bLoggedIn = b; }

	char GetLevel() const	{ return m_cLevel; }
	void SetLevel( char c )	{ m_cLevel = c; }
	bool IsMod() const;

	/* get name/away/room/prefs */
	const std::string& GetName() const	{ return m_sName; }
	const std::string& GetMessage() const	{ return m_sMessage; }
	const std::string& GetRoom() const	{ return m_sRoom; }
	const std::string& GetPrefs() const	{ return m_sPrefs; }

	/* set name/away/room */
	void SetName( const std::string &str )	{ m_sName.assign( str ); }
	void SetMessage(const std::string &str)	{ m_sMessage.assign( str ); }
	void SetRoom( const std::string &str )	{ m_sRoom.assign( str ); }
	void SetPrefs( const std::string &str )	{ m_sPrefs.assign( str ); }

	/* get idle status and time from last message */
	unsigned GetIdleSeconds() const;
	unsigned GetIdleMinutes() const	{ return GetIdleSeconds() / 60; }

	/* keep the time of the last idle broadcast and let the server
	 * figure out exactly how it wants to handle idle users. */
	/* XXX: this is kind of ugly. can we do better than this somehow? */
	unsigned GetIdleBroadcastSeconds() const;
	void UpdateIdleBroadcast()	{ m_LastIdleBroadcast = time(NULL); }

	/* convenience function */
	bool IsInRoom( const std::string &str ) const	{ return m_sRoom.compare(str) == 0; }

	/* resets idle/away statuses and the last message timer */
	void PacketSent();

private:
	/* Socket descriptor for this user's connection */
	unsigned m_iSocket;

	/* away status and message, if applicable */
	bool m_bAway;
	std::string m_sMessage;

	/* basic user details */
	std::string m_sName;
	std::string m_sRoom;
	std::string m_sPrefs;

	char m_cLevel;

	bool m_bLoggedIn;
	bool m_bMuted;

	// m_LastIdleBroadcast = time that idle status was last broadcast
	time_t m_LastMessage, m_LastIdleBroadcast;
};

#endif // USER_H
