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

class User
{
public:
	User( unsigned iSocket );
	~User();

	int GetSocket() const	{ return m_iSocket; }

	/* get away/muted/logged in */
	bool IsMod() const	{ return m_bIsMod; }
	bool IsAway() const	{ return m_bAway; }
	bool IsMuted() const	{ return m_bMuted; }
	bool IsLoggedIn() const	{ return m_bLoggedIn; }

	/* set idle/away/muted/logged in */
	void SetIsMod( bool b )		{ m_bIsMod = b; }
	void SetAway( bool b )		{ m_bAway = b; }
	void SetMuted( bool b )		{ m_bMuted = b; }
	void SetLoggedIn( bool b )	{ m_bLoggedIn = b; }

	/* get name/away/room */
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
	bool IsIdle() const;
	unsigned GetIdleTime() const;

	/* convenience function */
	bool IsInRoom( const std::string &str ) const	{ return m_sRoom.compare(str) == 0; }

	/* resets idle/away statuses and the last message timer */
	void PacketSent();

private:
	/* Socket descriptor for this user's connection */
	unsigned m_iSocket;

	/* Idle/away status and message, if applicable */
	bool m_bAway, m_bIdle;
	std::string m_sMessage;

	/* basic user details */
	std::string m_sName;
	std::string m_sRoom;
	std::string m_sPrefs;

	bool m_bLoggedIn;
	bool m_bIsMod;
	bool m_bMuted;

	time_t m_LastMessage;
};

#endif // USER_H
