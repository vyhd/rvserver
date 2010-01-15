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
	: m_sServer(server), m_sAuthPage(auth), m_sConfigPage(config)
{
	// fix up paths for HTML encoding
	if( m_sAuthPage[0] != '/' )
		m_sAuthPage.insert( 0, 1, '/' );

	if( m_sConfigPage[0] != '/' )
		m_sConfigPage.insert( 0, 1, '/' );
}

DatabaseConnector::~DatabaseConnector()
{
	m_Socket.Close();
}

bool DatabaseConnector::Connect()
{
	// already connected
	if( m_Socket.IsOpen() )
		return true;

	// attempt an HTTP connection for the server
	return m_Socket.OpenHost( m_sServer, 80 );
}

LoginResult DatabaseConnector::Login( const string &username, const string &passwd, User *user )
{
	// URLEncode the username, to escape any weird characters
	string sUsername = URLEncoding::Encode( username );

	// Base64, to obscure/escape weird characters, and URLEncode, to escape characters
	string sPassSafe = URLEncoding::Encode( Base64::Encode(passwd) );

	const string sAuth = Format( "username=%s&password=%s", sUsername.c_str(), sPassSafe.c_str() );

	string response = SendPOST( m_sAuthPage, sAuth );

	m_Socket.Close();

	if( response.empty() )
	{
		Logger::SystemLog( "POST for user %s failed!", username.c_str() );
		return LOGIN_ERROR;
	}

	Logger::SystemLog( "Response: %s", response.c_str() );

	// find the response code and determine the result
	unsigned start = response.find("LOGIN_");

	if( start == string::npos )
		return LOGIN_SERVER_DOWN;

	unsigned delim = response.find_first_of( '`', start );
	if( delim == string::npos )
		return LOGIN_SERVER_DOWN;

	const string code = response.substr( start, (delim-start) );

	// default to this in the absence of any other codes
	LoginResult result = LOGIN_ERROR;

	if( !code.compare("LOGIN_SUCCESS") )
		result = LOGIN_SUCCESS;
	else if( !code.compare("LOGIN_ERROR") )
		result = LOGIN_ERROR;
	else if( !code.compare("LOGIN_ERROR_ATTEMPTS") )
		result = LOGIN_ERROR_ATTEMPTS;
	else
		Logger::SystemLog( "Unknown response: %s", code.c_str() );

	// if we have us some success, set the user's level (char after delim)
	if( result == LOGIN_SUCCESS )
		user->SetLevel( response[delim+1]  );

	return result;
}

string DatabaseConnector::SendPOST( const string &sForm, const string &params )
{
	if( !Connect() )
		return string();

	string msg;
	msg.append( Format("POST %s HTTP/1.1\r\n", sForm.c_str()) );
	msg.append( Format("Host: %s\r\n", m_sServer.c_str()) );
	msg.append( "User-Agent: RVServer/1.0\r\n" );
	msg.append( Format("Content-Length: %u\r\n", params.length()) );
	msg.append( "Content-Type: application/x-www-form-urlencoded\r\n" );
	msg.append( "\r\n" );
	msg.append( params );

	Logger::SystemLog( "POST: %s", msg.c_str() );

	int iSent = m_Socket.Write( msg );

	// shut up, gcc
	if( iSent != (int)msg.length() )
	{
		Logger::SystemLog( "Write failed: returned %i/%u (%s)", iSent,
			msg.length(), strerror(errno) );

		return string();
	}

	// read the response data into a buffer and return it as a string
	char buffer[1024];

	// force blocking mode. XXX: we need a better solution!
	int iRead = m_Socket.Read( buffer, 1024, false );
	m_Socket.Close();
	buffer[iRead] = '\0';

	return string( buffer );
}
