#include <cstdarg>

#include "MySQLConnector.h"
#include "logger/Logger.h"

using namespace mysqlpp;

// m_Connection(false) = don't throw exceptions
MySQLConnector::MySQLConnector() : m_Connection(false)
{
	// this space intentionally left blank
}

MySQLConnector::~MySQLConnector()
{
	Disconnect();
}

bool MySQLConnector::Connect( const char *database , const char *host , const char *username , const char *password )
{
	// the caller can GetError() if they want an error message
	if( !m_Connection.connect(database, host, username, password) )
		return false;
	
	printf( "Connected to MySQL Database '%s' on '%s'.\n", database, host );
	return true;
}

void MySQLConnector::Disconnect()
{
	m_Connection.disconnect();
}

bool MySQLConnector::ExecuteQuery( char *fmt, ... )
{
	if( !IsConnected() )
		return false;

	// XXX: are we sure this can't overflow?
	char szQuery[256];

	va_list args;
	va_start( args, fmt );
	vsprintf( szQuery, fmt, args );
	va_end( args );

	Query q = m_Connection.query( szQuery );

	/* This is much faster, returns only bool for success status. */
	if( !q.exec() )
	{
		printf( "Error executing query: %s\n", q.error() );
		return false;
	}

	return true;
}

bool MySQLConnector::ExecuteQuery( std::string sQuery )
{
	if( !IsConnected() )
		return false;

	Query q = m_Connection.query( sQuery );

	if( !q.exec() )
	{
		printf( "Error executing query: %s\n", q.error() );
		       return false;
	}
	
	return true;
}

const StoreQueryResult MySQLConnector::GetQuery( char *fmt, ... )
{
	/* return a blank result set if we're not connected */
	if( !IsConnected() )
		return StoreQueryResult();

	char szQuery[256];

	va_list args;
	va_start( args, fmt );
	vsprintf( szQuery, fmt, args );
	va_end( args );
	
	Query q = m_Connection.query( szQuery );
	return q.store();
}
