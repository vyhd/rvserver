#include <cerrno>

#include "DatabaseConnector.h"
#include "DatabaseWorker.h"
#include "model/User.h"
#include "logger/Logger.h"

using namespace std;

DatabaseConnector::DatabaseConnector( const string &server, const string &auth, const string &config )
{
	m_pWorker = new DatabaseWorker( server, auth, config );
}

DatabaseConnector::~DatabaseConnector()
{
	m_pWorker->Stop();
	delete m_pWorker;
	m_pWorker = NULL;
}

void DatabaseConnector::Login( User *user, const string &passwd )
{
	m_pWorker->Login( user, passwd );
}

void DatabaseConnector::SavePrefs( const User *user )
{
	m_pWorker->SavePrefs( user );
}
