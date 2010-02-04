/* We can't directly access the database, so we've got a proxy script that
 * does the work for us...ugly, but safe-ish. If, in the future, the database
 * interface changes, we can change this class and keep the other calls.
 */

#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H

#include <string>
#include "Socket.h"

class User;
class DatabaseWorker;

class DatabaseConnector
{
public:
	/* uses the given auth/config scripts on server to handle data */
	DatabaseConnector( const std::string &server, const std::string &auth, const std::string &config );
	~DatabaseConnector();

	/* authenticates username/password and sets logged in/config */
	void Login( User *user, const std::string &passwd );
	void SavePrefs( const User *user );

private:
	DatabaseWorker* m_pWorker;
};

#endif // DATABASE_CONNECTOR_H
