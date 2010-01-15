#include "logger/Logger.h"
#include "model/Room.h"
#include "model/User.h"
#include "network/Socket.h"
#include "packet/ChatPacket.h"

using namespace std;

void Room::AddUser( User *user )
{
	// do these here, for sanity's sake
	if( user->GetRoom() != NULL )
		user->GetRoom()->RemoveUser( user );

	user->SetRoom( this );

	m_Users.insert( user );
}

void Room::RemoveUser( User *user )
{
	m_Users.erase( user );
}

bool Room::HasUser( User *user ) const
{
	return m_Users.find(user) != m_Users.end();
}

void Room::Broadcast( const ChatPacket *packet )
{
	// cache this: we only need to call it once
	const string msg = packet->ToString();

	for( set<User*>::iterator it = m_Users.begin(); it != m_Users.end(); it++ )
	{
		User *user = (*it);

		// ignore users who aren't logged in
		if( !user->IsLoggedIn() )
			continue;

		user->Write( msg );
	}
}
