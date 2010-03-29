/* Since the majority of moderation actions share the same basic structure,
 * we can stuff most of them into one file and share routines to save code. */

#include "packet/PacketHandler.h"
#include "model/Room.h"

bool HandleKick( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleDisable( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleBan( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleUnban( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleMute( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleUnmute( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleQuery( ChatServer *server, User *user, const ChatPacket *packet );
bool HandleClear( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER_FN( USER_KICK, HandleKick );
REGISTER_HANDLER_FN( USER_DISABLE, HandleDisable );
REGISTER_HANDLER_FN( USER_BAN, HandleBan );
REGISTER_HANDLER_FN( USER_UNBAN, HandleUnban );
REGISTER_HANDLER_FN( USER_MUTE, HandleMute );
REGISTER_HANDLER_FN( USER_UNMUTE, HandleUnmute );
REGISTER_HANDLER_FN( IP_QUERY, HandleQuery );
REGISTER_HANDLER_FN( FORCE_CLEAR, HandleClear );

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
inline User* GetTarget( ChatServer *server, User* user, const std::string &sName )
{
	User* target = server->GetUserByName( sName );

	// if not logged in, we don't mess with them
	if( target && !target->IsLoggedIn() )
		return NULL;

	return target;
}

/* a removal action results in the disconnection of the targeted client. */
/* Intentionally re-broken 01/08/2010 to re-enable 'kickspeak' bug. */
bool HandleRemoveAction( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	/* send a global message about this action */
	std::string sMessage = packet->sMessage + " was " +
		GetAction(packet->iCode) + " by " + user->GetName();

	server->WallMessage( sMessage );

	User *target = GetTarget( server, user, packet->sMessage );

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
	return true;
//	return HandleRemoveAction( server, user, packet );
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
	std::string sMessage = target->GetName() + " was " +
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
