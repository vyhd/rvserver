#include <cerrno>
#include <cstdlib>

#include <unistd.h>
#include <signal.h>

#include "ChatServer.h"
#include "logger/Logger.h"
#include "model/User.h"
#include "packet/ChatPacket.h"
#include "packet/PacketUtil.h"
#include "packet/PacketHandler.h"

using namespace std;

/* wait 0.15 seconds between update cycles */
const unsigned SLEEP_MICROSECONDS = 1000*150;

const int SERVER_PORT = 7005;

/* On caught signal, add a message and flush logs before exiting. */
void sig_handler( int signum )
{
	Logger::SystemLog( "Caught code %d (%s): exiting.", signum, strsignal(signum) );
	Logger::Flush();
	exit(signum);
}

ChatServer::ChatServer()
{
	m_pListener = new SocketListener;
	m_pListener->Connect( SERVER_PORT );
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

	Logger::SystemLog( "Added new client on socket %d, from IP %s", iSocket, GetUserIP(pUser) );
}

void ChatServer::RemoveUser( User *user )
{
	const int iSocket = user->GetSocket();

	Logger::SystemLog( "Client (%p) on socket %d, IP %s removed.", user, iSocket, GetUserIP(user) );

	shutdown( iSocket, SHUT_RDWR );
	close( iSocket );

	// broadcast a quitting message if they were logged in
	if( user->IsLoggedIn() )
	{
		user->SetLoggedIn( false );
		ChatPacket msg( USER_PART, user->GetName(), "_" );
		Broadcast( &msg );
	}

	// take this user out of the update loop
	m_Users.erase( user );

	delete user;
}

void ChatServer::MainLoop()
{
	while( 1 )
	{
		// see if the SocketListener has any new connections.
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
			for( it; it != m_UsersToDelete.end(); it++ )
				RemoveUser( *it );

			m_UsersToDelete.clear();
		}

		// flush all the logs to disk on update
		Logger::Flush();

		// give a bunch of time to other processes
		usleep( SLEEP_MICROSECONDS );
	}
}

void ChatServer::UpdateUser( User *user )
{
	memset( m_sReadBuffer, 0, BUFFER_SIZE );

	// no new data to operate on, or an error occurred
	if( Read(m_sReadBuffer, BUFFER_SIZE, user) == -1 )
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
		PacketUtil::SplitPacket( sBuffer, vPackets );

		for( unsigned i = 0; i < vPackets.size(); i++ )
			HandleUserPacket( user, vPackets[i] );
	}
}

void ChatServer::HandleUserPacket( User *user, const std::string &buf )
{
	// create user-specific log prefix, e.g. "Fire_Adept@127.0.0.1"
	string sUserPrefix( user->GetName() );
	sUserPrefix.push_back( '@' );
	sUserPrefix.append( GetUserIP(user) );

	// try to make a packet out of the data we've gotten
	ChatPacket packet( buf );

	// if the packet can't be parsed, drop the client.
	if( !packet.IsValid() )
	{
		Logger::SystemLog( "invalid packet from %s! %s", sUserPrefix.c_str(), buf.c_str() );
		Condemn( user );
		return;
	}

	// broadcast a returned message if the user was away before
	if( user->IsAway() && packet.iCode != CLIENT_AWAY )
	{
		ChatPacket msg( CLIENT_BACK, user->GetName(), "_" );
		Broadcast( &msg );
	}

	// update idle/away and last message timestamp
	user->PacketSent();

	// don't log packets that weren't actually handled
	if( !PacketHandler::Handle(this, user, &packet) )
		return;

	// If we have a login packet, wipe the password from the log.
	if( packet.iCode == USER_JOIN )
		packet.sParam2 = "[censored]";

	/* write this packet to the log, including the user prefix, e.g.
	 * Fire_Adept@192.168.1.1	3`_`hey sup d00dz`3`13`37
	 */
	string sLogLine( sUserPrefix );
	sLogLine.push_back( '\t' );
	sLogLine.append( packet.ToString() );

	Logger::ChatLog( sLogLine.c_str() );
}	

User* ChatServer::GetUserByName( const std::string &sName ) const
{
	/* XXX: can we do better than a linear search? */
	for( set<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
		if( (*it)->GetName().compare(sName) == 0 )
			return (*it);

	// no match found
	return NULL;
}

/* helper function to get an IP address from a user */
const char* ChatServer::GetUserIP( const User *user ) const
{
	struct sockaddr_in ClientData;
	socklen_t ClientLength = sizeof( ClientData );

	// if the socket has been disconnected already by the OS, we
	// aren't gonna get the IP (which sucks a bit).
	if( getpeername(user->GetSocket(), (sockaddr *)&ClientData, &ClientLength) < 0 )
		return "<nil>";

	return inet_ntoa(ClientData.sin_addr);
}

int ChatServer::Read( char *buffer, unsigned len, User *user )
{
	int iRead = recv( user->GetSocket(), buffer, len, MSG_DONTWAIT );

	if( iRead <= 0 )
	{
		// not ready for reading: ignore.
		if( errno == EWOULDBLOCK || errno == EAGAIN )
			return -1;

		// encounted an unknown error. log it and drop the client.
		Logger::SystemLog( "Error %i reading data from %u: %s", errno, user->GetSocket(), strerror(errno) );
		Condemn( user );

		return -1;
	}

	return iRead;
}

int ChatServer::Write( const std::string &str, User *user )
{
	// HACK: append a newline to all given lines to write
	const std::string sMessage = str + "\n";

	int iSent = send( user->GetSocket(), sMessage.c_str(), sMessage.size(), MSG_DONTWAIT );

	if( iSent <= 0 )
	{
		Logger::SystemLog( "Error sending line to IP %s: %s", GetUserIP(user), strerror(errno) );
		Condemn( user );

		return -1;
	}

	return iSent;
}

void ChatServer::Broadcast( const ChatPacket *packet, const std::string *sRoom )
{
	// optimization: instead of using Send(), cache the packet string and
	// Write(). we only need ToString (which is expensive) once this way.
	const std::string sPacketData = packet->ToString();

	// if no room is given, or this user is in the room, send it!
	for( set<User*>::const_iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		// don't bother broadcasting to users who won't see it
		if( !(*it)->IsLoggedIn() )
			continue;

		// no given room is a sentinel for all rooms
		if( !sRoom || (*it)->IsInRoom(*sRoom) )
			Write( sPacketData, (*it) );
	}
}

void ChatServer::Send( const ChatPacket *packet, User *user )
{
	Write( packet->ToString(), user );
}

void ChatServer::WallMessage( const std::string &sMessage )
{
	const ChatPacket packet( WALL_MESSAGE, "_", sMessage );
	Broadcast( &packet );
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
