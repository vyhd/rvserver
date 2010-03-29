#include <cerrno>

#include "network/DatabaseWorker.h"
#include "model/User.h"
#include "util/Base64.h"
#include "util/Config.h"
#include "util/StringUtil.h"
#include "util/URLEncoding.h"
#include "logger/Logger.h"

#include <unistd.h>	// for usleep()

// we'll be sleeping the vast majority of the time, but we
// want to be responsive when we do have something to process.
const unsigned SLEEP_MICROSECONDS = 25*1000;

/* Set this configuration if none could be found. Better than
 * causing the client to throw an exception, I think... */
const char* DEFAULT_PREFS = "theme|Classic|red|0|green|0|blue|0|freezeChat|false|ignoreColors|false|timeStamp|false|beep|true|bleep|true|audio|true";

using namespace std;
using namespace StringUtil;

enum RequestType
{
	REQ_LOGIN,
	REQ_SAVE_PREFS,
	REQ_BAN,
	REQ_UNBAN
};

struct Request
{
	Request( RequestType type_, User *user_, const string &data_ ) :
		type(type_), user(user_), data( data_.c_str() ) { }

	RequestType type;
	User *user;
	string data;
};

DatabaseWorker::DatabaseWorker( const Config *cfg )
{
	const char* DATABASE_HOST 	= cfg->Get( "DatabaseHost" );
	const char* LOGIN_PAGE 		= cfg->Get( "LoginPage" );
	const char* CONFIG_PAGE		= cfg->Get( "ConfigPage" );
	const char* BAN_PAGE		= cfg->Get( "BanPage" );

	if( !DATABASE_HOST || !LOGIN_PAGE || !CONFIG_PAGE || !BAN_PAGE )
	{
		Logger::SystemLog( "Could not find necessary database config!" );
		return;
	}

	m_sServer.assign( DATABASE_HOST );
	m_sAuthPage.assign( LOGIN_PAGE );
	m_sConfigPage.assign( CONFIG_PAGE );
	m_sBanPage.assign( BAN_PAGE );

	m_bRunning = true;
	m_Thread.Start( &Start, this );
}

DatabaseWorker::~DatabaseWorker()
{
	// stop the worker thread
	Stop();

	// remove all of the unhandled requests
	m_QueueLock.Lock();

	while( m_Requests.size() )
	{
		Request *req = m_Requests.front();
		m_Requests.pop();
		delete req;
	}

	m_QueueLock.Unlock();
}

bool DatabaseWorker::Connect()
{
	if( !m_Socket.OpenHost(m_sServer, 80) )
		Logger::SystemLog( "Failed to connect to %s!", m_sServer.c_str() );

	return m_Socket.IsOpen();
}

void DatabaseWorker::Disconnect()
{
	m_Socket.Close();
}

void DatabaseWorker::Stop()
{
	m_bRunning = false;
	m_Thread.Stop();
}

void DatabaseWorker::Login( User *user, const string &passwd )
{
	user->SetLoginState( LOGIN_CHECKING );

	Request *req = new Request( REQ_LOGIN, user, passwd );

	m_QueueLock.Lock();
	m_Requests.push( req );
	m_QueueLock.Unlock();
}

void DatabaseWorker::Ban( const string &username )
{
	// TODO
}

void DatabaseWorker::Unban( const string &username )
{
	// TODO
}

void DatabaseWorker::SavePrefs( const User *user )
{
	Logger::DebugLog( "SavePrefs( %p )", user );

	/* Form request here; the user needs to be reaped ASAP,
	 * but we want to save the preferences on our own time. */

	const string sUsername = URLEncoding::Encode( user->GetName() );
	const string sPrefs = URLEncoding::Encode( user->GetPrefs() );

	// it's okay for the user to be reaped now

	const string sMessage = Format( "username=%s&chatconfig=%s",
		sUsername.c_str(), sPrefs.c_str() );

	Request *req = new Request( REQ_SAVE_PREFS, NULL, sMessage );

	m_QueueLock.Lock();
	m_Requests.push( req );
	m_QueueLock.Unlock();
}

void DatabaseWorker::HandleRequests()
{
	while( m_bRunning )
	{
		// until we've got a request, sleep.
		while( m_Requests.empty() )
			usleep( SLEEP_MICROSECONDS );

		m_QueueLock.Lock();
		Request *req = m_Requests.front();
		m_Requests.pop();
		m_QueueLock.Unlock();

		switch( req->type )
		{
		case REQ_LOGIN:
			DoLogin( req );		break;
		case REQ_SAVE_PREFS:
			DoSavePrefs( req );	break;
		case REQ_BAN:
			DoBan( req, true );	break;
		case REQ_UNBAN:
			DoBan( req, false );	break;
		default:
			Logger::SystemLog( "??? Unknown DBWorker request %d", req->type );
		}

		Logger::DebugLog( "Request( %i, %p, %s ) handled",
			req->type, req->user, 
			(req->type == REQ_LOGIN ? "<censored>" : req->data.c_str())
		);

		delete req;
	}
}

void DatabaseWorker::DoLogin( Request *req )
{
	User *user = req->user;
	const string &passwd = req->data;

	// URLEncode the username, to escape any weird characters
	const string sUsername = URLEncoding::Encode( user->GetName() );

	// Base64 and URLEncode, to obfuscate and to escape characters
	const string sPassSafe = URLEncoding::Encode( Base64::Encode(passwd) );

	const string sAuth = Format( "username=%s&password=%s", sUsername.c_str(), sPassSafe.c_str() );

	const char* response_buf = SendPOST( m_sAuthPage, sAuth );

	if( response_buf == NULL )
	{
		Logger::SystemLog( "POST for user %s failed!", user->GetName().c_str() );
		user->SetLoginState( LOGIN_SERVER_DOWN );
		return;
	}

	// default to this in the absence of any other codes
	LoginState state = LOGIN_ERROR;

	// find the response code and determine the result
	{
		string response( response_buf );

		unsigned start, delim = string::npos;
		start = response.find("LOGIN_");

		if( start != string::npos )
			delim = response.find_first_of( '`', start );

		// if we were sent invalid data, we can't authenticate.
		if( start == string::npos || delim == string::npos )
		{
			user->SetLoginState( LOGIN_SERVER_DOWN );
			return;
		}

		// get the login code
		const string code = response.substr( start, (delim-start) );

		if( !code.compare("LOGIN_SUCCESS") )
			state = LOGIN_SUCCESS;
		else if( !code.compare("LOGIN_ERROR") )
			state = LOGIN_ERROR;
		else if( !code.compare("LOGIN_ERROR_ATTEMPTS") )
			state = LOGIN_ERROR_ATTEMPTS;
		else
			Logger::SystemLog( "Unknown login response: %s", code.c_str() );

		// if we have us some success, set the user's level (char after delim)
		if( state == LOGIN_SUCCESS )
			user->SetLevel( response[delim+1] );
	}

	// now, get the user's preferences and save them to the user
	DoLoadPrefs( user );

	// tell the main thread this user is done
	user->SetLoginState( state );
}

void DatabaseWorker::DoLoadPrefs( User *user )
{
	string params = "username=" + URLEncoding::Encode( user->GetName() );
	const char* response_buf = SendPOST( m_sConfigPage, params );

	if( response_buf == NULL )
	{
		Logger::SystemLog( "LoadPrefs failed! Using default for %s", user->GetName().c_str() );
		user->SetPrefs( DEFAULT_PREFS );
		return;
	}

	{
		const string response( response_buf );

		unsigned start = string::npos, end = string::npos;

		// find the preference string and assign it to the user.
		// '\r' finds the first part of the ending "\r\n".
		start = response.find( "theme" );
		end = response.find_first_of( '\r', start );

		if( start == string::npos || end == string::npos )
		{
			Logger::SystemLog( "LoadPrefs failed! Using default "
				"for %s", user->GetName().c_str() );

			user->SetPrefs( DEFAULT_PREFS );
			return;
		}

		const string prefs = response.substr( start, (end-start) );
		user->SetPrefs( prefs );
	}
}

void DatabaseWorker::DoSavePrefs( Request *req )
{
	Logger::DebugLog( "DatabaseWorker::DoSavePrefs( %s )", req->data.c_str() );
	// This was already built in the SavePrefs call. Just send it.
	SendPOST( m_sConfigPage, req->data );

	// We hope the POST worked, but we can't guarantee it. Oh well.
}

void DatabaseWorker::DoBan( Request *req, bool bBan )
{

}

const char* DatabaseWorker::SendPOST( const string &sForm, const string &params )
{
	// Sigh. HTTP 1.1 doesn't guarantee a persistent connection.
	// This means we have to connect every time we want to send data.
	if( !Connect() )
		return NULL;

	Logger::DebugLog( "POST sent..." );

	/* Create a HTTP 1.1 POST packet and send it to the server */
	string msg;
	msg.append( Format("POST %s HTTP/1.1\r\n", sForm.c_str()) );
	msg.append( Format("Host: %s\r\n", m_sServer.c_str()) );
	msg.append( "User-Agent: RVServer/1.0\r\n" );
	msg.append( Format("Content-Length: %u\r\n", params.length()) );
	msg.append( "Content-Type: application/x-www-form-urlencoded\r\n" );
	msg.append( "\r\n" );
	msg.append( params );

	int iSent = m_Socket.Write( msg, false );

	// shut up, gcc
	if( iSent != (int)msg.length() )
	{
		Logger::DebugLog( "Write failed: returned %i/%u (%s)", iSent,
			msg.length(), strerror(errno) );

		return NULL;
	}

	// force blocking mode. we're in a thread, so we can do this safely.
	int iRead = m_Socket.Read( m_sBuffer, HTTP_BUFFER_SIZE, false );
	m_Socket.Close();
	m_sBuffer[iRead] = '\0';

	Logger::DebugLog( "Received response to POST." );

	// return a const pointer to the buffer
	return m_sBuffer;
}

/* 
 * Copyright (c) 2009-10 Mark Cannon ("Vyhd")
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301, USA.
 */
