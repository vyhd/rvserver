#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>	// for memset, strcasecmp
#include <vector>	// for PacketUtil::Split

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>	// for timestamping

#include "ChatServer.h"
#include "logger/Logger.h"
#include "model/Room.h"
#include "model/User.h"
#include "network/DatabaseConnector.h"
#include "packet/ChatPacket.h"
#include "packet/PacketUtil.h"
#include "packet/PacketHandler.h"
#include "util/StringUtil.h"
#include "verinfo.h"	// for BUILD_DATE, BUILD_VERSION

using namespace std;

/* wait 0.15 seconds between update cycles by default */
const unsigned SLEEP_DEFAULT = 1000*150;

/* Default settings for the chat server */
//const int SERVER_PORT = 7005;
//const char* const DATABASE_HOST = "www.runevillage.com";

/* HTML pages relative to DATABASE_HOST */
// TODO: softcode this!
//const char* const LOGIN_PAGE = "/ThePub/authenticate.php";
//const char* const CONFIG_PAGE = "/ThePub/chatconfig.php";

ChatServer* g_pServer = NULL;

/* statistics */
unsigned g_iBytesSent = 0;
unsigned g_iBytesRead = 0;
unsigned g_iBytesBroadcast = 0;

/* TODO: stuff most of this logic into a ::Start routine. */
ChatServer::ChatServer( const char *config ) : m_pListener(NULL)
{
	// direct global error handling callbacks to this server
	g_pServer = this;

	if( !m_Config.Load(config) )
	{
		Logger::SystemLog( "Failed to load configuration! Server can't start..." );
		return;
	}

	int iPort = m_Config.GetInt( "ServerPort" );

	if( iPort != 0 )
	{
		m_pListener = new SocketListener;
		m_pListener->Connect( iPort );
	}

	const char* DATABASE_HOST 	= m_Config.Get( "DatabaseHost" );
	const char* LOGIN_PAGE 		= m_Config.Get( "LoginPage" );
	const char* CONFIG_PAGE		= m_Config.Get( "ConfigPage" );

	if( !DATABASE_HOST || !LOGIN_PAGE || !CONFIG_PAGE )
	{
		Logger::SystemLog( "Could not find necessary database config!" );
		return;
	}

	m_pConnector = new DatabaseConnector( DATABASE_HOST, LOGIN_PAGE, CONFIG_PAGE );

	const char* EXTRA_ROOMS		= m_Config.Get( "AdditionalRooms" );

	if( EXTRA_ROOMS )
	{
		vector<string> vsRooms;
		StringUtil::Split( EXTRA_ROOMS, vsRooms, ',' );

		for( unsigned i = 0; i < vsRooms.size(); i++ )
			m_Rooms.AddRoom( vsRooms[i] );
	}
}

ChatServer::~ChatServer()
{
	if( m_pListener )
	{
		m_pListener->Disconnect();
		delete m_pListener;
		m_pListener = NULL;
	}
}

void ChatServer::AddUser( unsigned iSocket )
{
	User *pUser = new User( iSocket );
	m_Users.push_back( pUser );

	Logger::SystemLog( "Added new client on socket %d, from IP %s", iSocket, pUser->GetIP() );
}

void ChatServer::RemoveUser( User *user )
{
	Logger::SystemLog( "Client (%p) at IP %s removed.", user, user->GetIP() );

	// broadcast a quitting message if they were logged in
	if( user->IsLoggedIn() )
	{
		user->SetLoggedIn( false );
		Broadcast( ChatPacket(USER_PART, user->GetName(), BLANK) );

		// save this user's preferences
		m_pConnector->SavePrefs( user );
	}

	// take this user out of the RoomList
	m_Rooms.RemoveUser( user );

	delete user;
}

void ChatServer::MainLoop()
{
	// no listener means no connection; no connection means no server.
	if( m_pListener == NULL )
		return;

	/* allow sleep time as a configuration, but provide a default */
	unsigned iSleepTime = m_Config.GetInt( "SleepMicroseconds" );
	const unsigned SLEEP_MICROSECONDS = (iSleepTime != 0) ? iSleepTime : 150*1000;

	struct timeval tv_start, tv_end;
	while( true )
	{
		gettimeofday( &tv_start, NULL );

		// see if the SocketListener has any new connections and add them.
		{
			int iSocket = m_pListener->GetConnection();

			if( iSocket > 0 )
				AddUser( iSocket );
		}

		// loop over all the clients and update them as needed
		for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
		{
			User *user = (*it);

			// if the user isn't logged in, check login status
			if( !user->IsLoggedIn() )
			{
				const LoginState state = user->GetLoginState();

				// we're not expecting any data just yet.
				if( state == LOGIN_CHECKING )
					continue;

				// if the user's login completed, check it.
				if( state != LOGIN_NONE )
					HandleLoginState( user );
			}

			// update this user (which means checking for and
			// handling packets) regardless of login status.
			UpdateUser( user );

			// users can't be idle unless they're logged in...
			if( user->IsLoggedIn() )
				CheckIdleStatus( user );

			// if this user is dead, erase/reposition the iterator
			// and properly remove the user from their other stuff.
			if( user->IsDead() )
			{
				it = m_Users.erase( it );
				RemoveUser( user );
			}

		}

		// flush all the logs to disk on update
		Logger::Flush();

		gettimeofday( &tv_end, NULL );

		unsigned iDiff = 1000000 * (tv_end.tv_sec-tv_start.tv_sec) + (tv_end.tv_usec-tv_start.tv_usec);

		if( iDiff >= 150 )
			printf( "[MainLoop took %u usecs to execute.]\n", iDiff );

		// give a bunch of time to other processes
		usleep( SLEEP_MICROSECONDS );
	}

	Logger::SystemLog( "The impossible happened! :(" );
}

void ChatServer::UpdateUser( User *user )
{
	unsigned iPos = 0;

	memset( m_sReadBuffer, 0, BUFFER_SIZE );

	// no new data to operate on, or an error occurred
	if( (iPos = user->Read(m_sReadBuffer, BUFFER_SIZE)) <= 0 )
		return;

	// the vast majority of cases won't need split, so we
	// can optimize the branching logic around that case.
	std::string sBuffer( m_sReadBuffer );

	if( !PacketUtil::NeedsSplit(sBuffer) )
	{
		// only one packet: handle and be on our way.
		HandleUserPacket( user, sBuffer );
	}
	else
	{
		// we have multiple packets. split them and handle each.
		vector<std::string> vPackets;
		PacketUtil::Split( sBuffer, vPackets );

		for( unsigned i = 0; i < vPackets.size(); i++ )
			HandleUserPacket( user, vPackets[i] );
	}
}

void ChatServer::HandleUserPacket( User *user, const std::string &buf )
{
	// create user-specific log prefix, e.g. "Fire_Adept@127.0.0.1"
	string sUserPrefix = StringUtil::Format( "%s@%s", user->GetName().c_str(), user->GetIP() );

	// try to make a packet out of the data we've gotten
	ChatPacket packet( buf );

	// if the packet can't be parsed, drop the client.
	if( !packet.IsValid() )
	{
		Logger::SystemLog( "invalid packet from %s! %s", sUserPrefix.c_str(), buf.c_str() );
		Logger::SystemLog( "packet data: %s", buf.c_str() );
		user->Kill();
		return;
	}

	// broadcast a returned message if the user was idle or away before.
	if( user->IsLoggedIn() && (user->IsIdle() || user->IsAway()) )
		Broadcast( ChatPacket(CLIENT_BACK, user->GetName(), BLANK) );

	// update idle/away and last message timestamp
	user->PacketSent();

	// don't log packets that weren't actually handled
	if( !PacketHandler::Handle(this, user, &packet) )
		return;

	// If we have a login packet, wipe the password from the log.
	if( packet.iCode == USER_JOIN )
		packet.sMessage = "[censored]";

	/* write this packet to the log, including the user prefix, e.g.
	 * Fire_Adept@192.168.1.1	3`_`hey sup d00dz`3`13`37
	 */
	string sLogLine = StringUtil::Format( "%s\t%s",
		sUserPrefix.c_str(), packet.ToString().c_str() );

	Logger::ChatLog( sLogLine.c_str() );
}	

void ChatServer::CheckIdleStatus( User *user )
{
	// no need to update idle status here
	if( !user->IsIdle() )
		return;

	// user has been away for too long, so kick 'em
	if( user->IsInert() )
	{
		user->Write( ChatPacket(IDLE_KICK).ToString() );
		user->Kill();
		return;
	}

	const unsigned iIdleMinutes = user->GetIdleMinutes();

	// do we need to broadcast an update message? (every minute)
	if( user->GetLastIdleMinute() == iIdleMinutes )
		return;

	// print the idle time into a string, broadcast it
	string sIdleTime = StringUtil::Format( "%04u", iIdleMinutes );
	Broadcast( ChatPacket(CLIENT_IDLE, user->GetName(), sIdleTime) );

	// update the user's last idle broadcast timestamp
	user->UpdateLastIdle();
}

void ChatServer::HandleLoginState( User *user )
{
	/* dispatches messages to the user and/or server, as appropriate */
	switch( user->GetLoginState() )
	{
	case LOGIN_ERROR:
		user->Write( ChatPacket(ACCESS_DENIED).ToString() );
		break;
	case LOGIN_ERROR_ATTEMPTS:
		user->Write( ChatPacket(LIMIT_REACHED).ToString() );
		break;
	case LOGIN_SERVER_DOWN:
		user->Write( ChatPacket(SERVER_DOWN).ToString() );
		break;
	case LOGIN_SUCCESS:
	{
		// write 'accepted' response
		user->Write( ChatPacket(ACCESS_GRANTED).ToString() );

		// write configuration (which was set by the login request)
		ChatPacket prefs(CLIENT_CONFIG, BLANK, user->GetPrefs() );
		user->Write( prefs.ToString() );

		m_Rooms.GetDefaultRoom()->AddUser( user );
		user->SetLoggedIn( true );

		// send the new guy a nice little version message
		std::string ver = StringUtil::Format( "Server build %u, "
			"compiled %s", BUILD_VERSION, BUILD_DATE );

		user->Write( ChatPacket(WALL_MESSAGE, BLANK, ver).ToString() );

		// tell everyone that this user joined
		ChatPacket msg( USER_JOIN, user->GetName(), GetUserState(user) );
		Broadcast( msg );

		break;
	}
	case LOGIN_NONE:
	case LOGIN_CHECKING:
		Logger::SystemLog( "Aaaaah! Shouldn't be here! State %i", user->GetLoginState() );
		break;
	}

	// unless the user successfully logged in, kill the connection.
	if( user->GetLoginState() != LOGIN_SUCCESS )
		user->Kill();
}

User* ChatServer::GetUserByName( const std::string &sName ) const
{
	for( list<User*>::const_iterator it = m_Users.begin(); it != m_Users.end(); it++ )
		if( !StringUtil::CompareNoCase((*it)->GetName(), sName) )
			return (*it);

	// no match found
	return NULL;
}

std::string ChatServer::GetUserState( const User *user ) const
{
	std::string ret;
	ret.assign( m_Rooms.GetName(user->GetRoom()) );
	ret.push_back( '|' );

	// display user level.
	ret.push_back( user->GetLevel() );

	// display muted status (M for muted, _ for not)
	ret.push_back( user->IsMuted() ? 'M' : '_' );

	// display idle status, including time if needed
	if( user->IsIdle() )
		ret.append( StringUtil::Format( "i%04u", user->GetIdleMinutes()) );
	else
		ret.push_back( '_' );

	// display away status, appending the message if away
	if( user->IsAway() )
		ret.append( "a%s", user->GetMessage().c_str() );
	else
		ret.push_back( '_' );

	return ret;
}

void ChatServer::Broadcast( const ChatPacket &packet )
{
	// optimization: instead of using Send(), cache the packet string and
	// Write(). we only need ToString (which is expensive) once this way.
	const std::string sPacketData = packet.ToString();

	// send to every single user on the server
	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		User *user = (*it);

		// don't bother broadcasting to users who won't see it
		if( !user->IsLoggedIn() )
			continue;

		user->Write( sPacketData );
		g_iBytesBroadcast += sPacketData.length();
	}
}

void ChatServer::WallMessage( const std::string &sMessage )
{
	const std::string sPacket = ChatPacket(WALL_MESSAGE, BLANK, sMessage).ToString();

	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		User *user = (*it);

		if( !user->IsMod() )
			continue;

		user->Write( sPacket );
	}
}

/* On caught signal, add a message and flush logs before exiting. */
static void sig_handler( int signum )
{
	Logger::SystemLog( "Caught code %d (%s): exiting.", signum, strsignal(signum) );

	if( g_pServer )
	{
		// cleanly remove all users
		list<User*>::const_iterator it = g_pServer->GetUserList()->begin();

		for( ; it != g_pServer->GetUserList()->end(); it++ )
		{
			ChatPacket kill( USER_PART, (*it)->GetName(), "_" );
			g_pServer->Broadcast( kill );
		}
	}

	exit(signum);
}

int main( int argc, char **argv )
{
	// ignore SIGPIPE. we don't want the program stopping because
	// a client disconnected in an unexpected way.
	sigignore( SIGPIPE );

	// intercept these normally-fatal signals with our own version, 
	// which will flush all of the server logs before exiting.
	signal( SIGINT, sig_handler );
	signal( SIGTERM, sig_handler );
	signal( SIGSEGV, sig_handler );
	signal( SIGABRT, sig_handler );

	ChatServer server( "config.txt" );
	server.MainLoop();
	return 0;
}
