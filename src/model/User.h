/* User: contains information on a user, including current status,
 * idle time, etc.
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

	/* get idle/away/muted/logged in */
	bool IsMod() const	{ return m_bIsMod; }
	bool IsIdle() const	{ return m_bIdle; }
	bool IsAway() const	{ return m_bAway; }
	bool IsMuted() const	{ return m_bMuted; }
	bool IsLoggedIn() const	{ return m_bLoggedIn; }

	/* set idle/away/muted/logged in */
	void SetIsMod( bool b )		{ m_bIsMod = b; }
	void SetIdle( bool b )		{ m_bIdle = b; }
	void SetAway( bool b )		{ m_bAway = b; }
	void SetMuted( bool b )		{ m_bMuted = b; }
	void SetLoggedIn( bool b )	{ m_bLoggedIn = b; }

	/* get name/away/room */
	const std::string& GetName() const	{ return m_sName; }
	const std::string& GetAway() const	{ return m_sMessage; }
	const std::string& GetRoom() const	{ return m_sRoom; }
	const std::string& GetPrefs() const	{ return m_sPrefs; }

	/* set name/away/room */
	void SetName( const std::string &str )	{ m_sName.assign( str ); }
	void SetAway( const std::string &str )	{ m_sMessage.assign( str ); }
	void SetRoom( const std::string &str )	{ m_sRoom.assign( str ); }
	void SetPrefs( const std::string &str )	{ m_sPrefs.assign( str ); }

	/* convenience function */
	bool IsInRoom( const std::string &str ) const	{ return m_sRoom.compare(str) == 0; }

	/* Resets the idle and away statuses when called. */
	void SetBack();

	/* broadcasts a quitting message */
	void Part();

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
};

#endif // USER_H
