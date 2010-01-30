#include <cerrno>

#include "DatabaseConnector.h"
#include "model/User.h"
#include "logger/Logger.h"
#include "Socket.h"

#include "util/Base64.h"
#include "util/URLEncoding.h"
#include "util/StringUtil.h"

using namespace std;
using namespace StringUtil;

DatabaseConnector::DatabaseConnector( const string &server, const string &auth, const string &config )
	: m_Worker( server, auth, config )
{
	// this space left blank
}

DatabaseConnector::~DatabaseConnector()
{
	m_Worker.Stop();
}

void DatabaseConnector::Login( User *user, const string &passwd )
{
	m_Worker.Login( user, passwd );
}
