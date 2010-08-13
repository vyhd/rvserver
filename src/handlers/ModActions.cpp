/* Since the majority of moderation actions share the same basic structure,
 * we can stuff most of them into one file and share routines to save code. */

#include "packet/PacketHandler.h"
#include "model/Room.h"
#include "network/DatabaseConnector.h"

bool ModAction( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( USER_KICK, ModAction );
REGISTER_HANDLER( USER_DISABLE, ModAction );
REGISTER_HANDLER( USER_BAN, ModAction );
REGISTER_HANDLER( USER_UNBAN, ModAction );
REGISTER_HANDLER( USER_MUTE, ModAction );
REGISTER_HANDLER( USER_UNMUTE, ModAction );
REGISTER_HANDLER( IP_QUERY, ModAction );
REGISTER_HANDLER( FORCE_CLEAR, ModAction );

using namespace std;

const char* GetAction( uint16_t iCode )
{
	switch( iCode )
	{
	case USER_KICK:		return "kicked";
	case USER_DISABLE:	return "disabled";
	case USER_BAN:		return "banned";
	case USER_UNBAN:	return "unbanned";
	case USER_MUTE:		return "muted";
	case USER_UNMUTE:	return "unmuted";
	case IP_QUERY:		return "IP queried";
	default:		return "eaten";
	}
}

/* returns a valid target if 'user' if sName references a user. */
inline User* GetTarget( ChatServer *server, User* user, const string &sName )
{
	User* target = server->GetUserByName( sName );

	// if not logged in, we don't mess with them
	if( target && !target->IsLoggedIn() )
		return NULL;

	return target;
}

bool ModAction( ChatServer *server, User *user, const ChatPacket *packet )
{
	return false; /* no op */
}

/* a removal action results in the disconnection of the targeted client. */
/* Intentionally re-broken 01/08/2010 to re-enable 'kickspeak' bug. */
bool HandleRemoveAction( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	User *target = GetTarget( server, user, packet->sMessage );

	const string sMessage = packet->sMessage + " was "
		+ GetAction(packet->iCode) + " by " + user->GetName();

	/* HACK: don't modchat for mute, unmute, or ban. Every time those
	 * are called, we incur a lookup penalty in the ban/mute list, so
	 * we want them to mean it. */
	switch( packet->iCode )
	{
		case USER_BAN:
		case USER_MUTE:
		case USER_UNMUTE:
			if( target == NULL )
				return false;
	};

	// handle database connectivity
	if( packet->iCode == USER_BAN )
		server->GetConnection()->Ban( target->GetName() );

	server->WallMessage( sMessage );

	// notify the target about the taken action, if they exist
	if( target != NULL )
	{
		ChatPacket notify( packet->iCode );
		target->Write( notify.ToString() );
		target->Kill();
	}

	return true;
}

/* A mute action is simply the handler for a mute/unmute command.
 * the client handles the message, so we just broadcast the packet. */
bool HandleMuteAction( ChatServer *server, User *user, const ChatPacket *packet, bool bMute )
{
	if( !user->IsMod() )
		return false;

	User* target = GetTarget( server, user, packet->sMessage );

	if( target == NULL )
		return false;

	target->SetMuted( bMute );

	// pass on the mute (or unmute), who it affected, and who did it
	ChatPacket msg( packet->iCode, target->GetName(), user->GetName() );
	server->Broadcast( msg );

	return true;
}

bool HandleKick( ChatServer *server, User *user, const ChatPacket *packet )
{
	return HandleRemoveAction( server, user, packet );
}

bool HandleDisable( ChatServer *server, User *user, const ChatPacket *packet )
{
	return HandleRemoveAction( server, user, packet );
}

// XXX: doesn't actually ban right now
bool HandleBan( ChatServer *server, User *user, const ChatPacket *packet )
{
	return HandleRemoveAction( server, user, packet );
}

// XXX: won't work right now
bool HandleUnban( ChatServer *server, User *user, const ChatPacket *packet )
{
	server->GetConnection()->Unban( packet->sMessage );
	return true;
}

bool HandleMute( ChatServer *server, User *user, const ChatPacket *packet )
{
	return HandleMuteAction( server, user, packet, true );
}

bool HandleUnmute( ChatServer *server, User *user, const ChatPacket *packet )
{
	return HandleMuteAction( server, user, packet, false );
}

bool HandleQuery( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	User *target = GetTarget( server, user, packet->sUsername );

	if( target == NULL )
		return false;

	// send a packet back to the caller giving the IP address
	ChatPacket query( IP_QUERY, target->GetName(), user->GetIP() );
	user->Write( query.ToString() );

	/* send a global message about this action */
	string sMessage = target->GetName() + " was " +
		GetAction(packet->iCode) + " by " + user->GetName();

	server->WallMessage( sMessage );

	return true;
}

bool HandleClear( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() || !user->GetRoom() )
		return false;

	// broadcast a message to clients so they blank their screens
	ChatPacket msg( FORCE_CLEAR, user->GetName(), BLANK );
	user->GetRoom()->Broadcast( msg );

	return true;
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
