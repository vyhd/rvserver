#ifndef MYSQL_CONNECTOR_H
#define MYSQL_CONNECTOR_H

// Look for the MySQL headers in /usr/include/mysql/.
#define MYSQLPP_MYSQL_HEADERS_BURIED 1

#include <mysql++/mysql++.h>

class MySQLConnector
{
public:
	MySQLConnector();
	~MySQLConnector();

	/* Attempts to connect to the database. */
	bool Connect( const char *database, const char *host, const char *username, const char *password );

	/* Disconnects from the current database. */
	void Disconnect();

	/* Executes a query, true if successful */
	bool ExecuteQuery( char *fmt, ... );
	bool ExecuteQuery( std::string sQuery );

	/* Checks to see if the database is connected. */
	bool IsConnected() const	{ return m_Connection.connected(); }

	/* Gets an error code from the connection */
	const char *GetError() const	{ return m_Connection.error(); }

	/* Returns the result set for the query. */
	const mysqlpp::StoreQueryResult GetQuery( char *fmt, ... );

private:
	/* Our connection */
	mysqlpp::Connection m_Connection;

	/* Result set */
	mysqlpp::StoreQueryResult m_storeResult;
};

#endif // MYSQL_CONNECTOR_H
