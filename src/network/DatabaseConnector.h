/* We can't directly access the database, so we've got a proxy script that
 * does the work for us...ugly, but safe-ish. If, in the future, the database
 * interface changes, we can change this class and keep the other calls.
 */

#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H

#include <string>
#include "Socket.h"

enum LoginResult
{
	LOGIN_SUCCESS,		// authorized
	LOGIN_ERROR,		// invalid username/password
	LOGIN_ERROR_ATTEMPTS,	// too many failed attempts
	LOGIN_SERVER_DOWN	// can't reach login server
};

class User;

class DatabaseConnector
{
public:
	/* uses the given auth/config scripts on server to handle data */
	DatabaseConnector( const std::string &server, const std::string &auth, const std::string &config );
	~DatabaseConnector();

	bool Connect();

	/* returns true if there is a valid connection */
	bool IsConnected() const	{ return m_Socket.IsOpen(); }

	/* authenticates username/password and sets logged in/config */
	LoginResult Login( const std::string &name, const std::string &passwd, User *user );

	void LoadPrefs( User *user );
	void SavePrefs( const User *user );

private:
	/* sends a request with URL-encoded parameters, returns response */
	std::string SendPOST( const std::string &url, const std::string &params );

	std::string m_sServer, m_sAuthPage, m_sConfigPage;

	Socket m_Socket;
};

#endif // DATABASE_CONNECTOR_H
