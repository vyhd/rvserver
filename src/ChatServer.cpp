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
#include "model/User.h"
#include "network/DatabaseConnector.h"
#include "packet/ChatPacket.h"
#include "packet/PacketUtil.h"
#include "packet/PacketHandler.h"

using namespace std;

/* wait 0.15 seconds between update cycles */
const unsigned SLEEP_MICROSECONDS = 1000*150;

/* this is the server port unless otherwise set */
const int SERVER_PORT = 7005;

/* number of seconds to full user idle status */
const unsigned MINUTES_TO_IDLE = 5;

/* number of seconds to idle kick */
const unsigned MINUTES_TO_IDLE_KICK = 90;

ChatServer* g_pServer = NULL;

/* statistics */
unsigned g_iBytesSent = 0;
unsigned g_iBytesRead = 0;
unsigned g_iBytesBroadcast = 0;

/* On caught signal, add a message and flush logs before exiting. */
void sig_handler( int signum )
{
	Logger::SystemLog( "Caught code %d (%s): exiting.", signum, strsignal(signum) );
	Logger::SystemLog( "%u bytes sent, %u bytes received, %u bytes broadcast", g_iBytesSent, g_iBytesRead, g_iBytesBroadcast );

	if( g_pServer )
	{
//		ChatPacket msg( WALL_MESSAGE, BLANK, "AAAA! EL SERVIDOR ESTA TERMINANDO! VAMOS A MORIR EN 5 SEGUNDOS!" );
//		g_pServer->Broadcast( &msg );

//		sleep( 5 );

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

ChatServer::ChatServer()
{
	g_pServer = this;

	m_pListener = new SocketListener;
	m_pListener->Connect( SERVER_PORT );

	// TODO: softcode this!
	m_pConnector = new DatabaseConnector( "www.runevillage.com",
		"/ThePub/authenticate.php", "/ThePub/chatconfig.php" );

	if( !m_pConnector->Connect() )
		Logger::SystemLog( "Login connection failed!" );

	m_Rooms.AddRoom( "Spam Room" );
}

ChatServer::~ChatServer()
{
	m_pListener->Disconnect();
	delete m_pListener;
}

void ChatServer::AddUser( unsigned iSocket )
{
	User *pUser = new User( iSocket );
	m_Users.insert( pUser );

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
	}

	// take this user out of the RoomList
	m_Rooms.RemoveUser( user );

	// take this user out of the update loop
	m_Users.remove( user );

	delete user;
}

void ChatServer::MainLoop()
{
	struct timeval tv_start, tv_end;
	while( 1 )
	{
		gettimeofday( &tv_start, NULL );

		// see if the SocketListener has any new connections and add them.
		{
			int iSocket = m_pListener->GetConnection();

			if( iSocket > 0 )
				AddUser( iSocket );
		}

		// loop over all the clients and update them as needed
		for( set<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
			UpdateUser( *it );

		// disconnect and remove dead clients
		if( !m_UsersToDelete.empty() )
		{
			set<User*>::iterator it = m_UsersToDelete.begin();
			for( ; it != m_UsersToDelete.end(); it++ )
				RemoveUser( *it );

			m_UsersToDelete.clear();
		}

		// send off any messages for users who are idle
		for( set<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
			CheckIdleStatus( *it );

		// flush all the logs to disk on update
		Logger::Flush();

		gettimeofday( &tv_end, NULL );

		unsigned iDiff = 1000000 * (tv_end.tv_sec-tv_start.tv_sec) + (tv_end.tv_usec-tv_start.tv_usec);

		if( iDiff >= 150 )
			printf( "[MainLoop took %u usecs to execute.]\n", iDiff );

		// give a bunch of time to other processes
		usleep( SLEEP_MICROSECONDS );
	}
}

void ChatServer::UpdateUser( User *user )
{
	memset( m_sReadBuffer, 0, BUFFER_SIZE );

	// no new data to operate on, or an error occurred
	if( user->Read(m_sReadBuffer, BUFFER_SIZE) == -1 )
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
	string sUserPrefix( user->GetName() );
	sUserPrefix.push_back( '@' );
	sUserPrefix.append( user->GetIP() );

	// try to make a packet out of the data we've gotten
	ChatPacket packet( buf );

	// if the packet can't be parsed, drop the client.
	if( !packet.IsValid() )
	{
		Logger::SystemLog( "invalid packet from %s! %s", sUserPrefix.c_str(), buf.c_str() );
		user->Kill();
		return;
	}

	// broadcast a returned message if the user was idle or away before.
	// don't broadcast if the user just changed their away message, though.
#if 0
	if( user->IsLoggedIn() && ( IsIdle(user) ||
		(user->IsAway() && packet.iCode != CLIENT_AWAY)) )
	{
		Broadcast( ChatPacket(CLIENT_BACK, user->GetName(), BLANK) );
	}
#endif

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
	string sLogLine( sUserPrefix );
	sLogLine.push_back( '\t' );
	sLogLine.append( packet.ToString() );

	Logger::ChatLog( sLogLine.c_str() );
}	

void ChatServer::CheckIdleStatus( User *user )
{
	// never bother with users who aren't logged in
	if( !user->IsLoggedIn() )
		return;

	// no sense forcing a recalculation for every branch
	unsigned iIdleMinutes = user->GetIdleMinutes();

	// nothing to do if the idle time is below the threshold
	if( iIdleMinutes < MINUTES_TO_IDLE )
		return;

	// if the user is above our kick threshold, give 'em the boot
	if( iIdleMinutes >= MINUTES_TO_IDLE_KICK )
	{
		user->Write( ChatPacket(IDLE_KICK).ToString() );
		user->Kill();
		return;
	}

	// do we need to broadcast an update message? (every minute)
	if( user->GetIdleBroadcastSeconds() < 60 )
		return;

	// print the idle time into a string, broadcast it
	string sIdleTime = StringUtil::Format( "%04u", iIdleMinutes );
	Broadcast( ChatPacket(CLIENT_IDLE, user->GetNAme(), sIdleTime) );

	// update the user's last idle broadcast timestamp
	user->UpdateIdleBroadcast();
}

bool ChatServer::IsIdle( const User *user ) const
{
	return user->GetIdleMinutes() >= MINUTES_TO_IDLE;
}

User* ChatServer::GetUserByName( const std::string &sName ) const
{
	for( list<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
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
	if( IsIdle(user) )
	{
		ret.push_back( 'i' );

		// print idle time in 4 digits for the client
		char sIdleTime[5];
		snprintf( sIdleTime, 5, "%04u", user->GetIdleMinutes() );
		ret.append( sIdleTime );
	}
	else
	{
		ret.push_back( '_' );
	}

	// display away status, appending the message if away
	ret.push_back( user->IsAway() ? 'a' : '_' );
	if( user->IsAway() )
		ret.append( user->GetMessage() );

	return ret;
}

void ChatServer::Broadcast( const ChatPacket &packet )
{
	// optimization: instead of using Send(), cache the packet string and
	// Write(). we only need ToString (which is expensive) once this way.
	const std::string sPacketData = packet.ToString();

	// send to every single user on the server
	for( list<User*>::const_iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		const User *user = (*it);

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

	for( list<User*>::const_iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		const User *user = (*it);
		if( !user->IsMod() )
			continue;

		user->Write( sPacket );
	}
}

void ChatServer::Condemn( User *user )
{
	Logger::SystemLog( "Condemning %s (%p) to the scrap heap.", user->GetName().c_str(), user );

	// delete this user after the update loop is done
	m_UsersToDelete.insert( user );
}

int main( int argc, char **argv )
{
	// ignore SIGPIPE. we don't want the program stopping because
	// a client disconnected in an unexpected way.
	sigignore( SIGPIPE );

	// intercept SIGINT, SIGSEGV, and SIGTERM with our own version, 
	// which will flush all of the server logs before exiting.
	signal( SIGINT, sig_handler );
	signal( SIGTERM, sig_handler );
	signal( SIGSEGV, sig_handler );

	ChatServer server;
	server.MainLoop();
	return 0;
}
