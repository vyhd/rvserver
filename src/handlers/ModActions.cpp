/* Since the majority of moderation actions share the same basic structure,
 * we can stuff most of them into one file and share routines to save code. */

#include "packet/PacketHandler.h"
#include "network/DatabaseConnector.h"
#include "model/Room.h"
#include "logger/Logger.h"
#include "util/StringUtil.h"

/* A moderation action that involves a moderator and a user */
bool UserAction( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( USER_KICK, UserAction );
REGISTER_HANDLER( USER_DISABLE, UserAction );
REGISTER_HANDLER( USER_BAN, UserAction );
REGISTER_HANDLER( USER_UNBAN, UserAction );
REGISTER_HANDLER( USER_MUTE, UserAction );
REGISTER_HANDLER( USER_UNMUTE, UserAction );
REGISTER_HANDLER( IP_QUERY, UserAction );


/* These are handled separately because they don't share code paths. */
bool ForceClear( ChatServer *server, User *user, const ChatPacket *packet );
bool ModChat( ChatServer *server, User *user, const ChatPacket *packet );

REGISTER_HANDLER( FORCE_CLEAR, ForceClear );
REGISTER_HANDLER( MOD_CHAT, ModChat );


using namespace std;

/* Internally used functions */
bool Remove( User *target, uint16_t iCode );
bool Ban( ChatServer *server, const string &sName );
bool Unban( ChatServer *server, const string &sName );
bool Mute( ChatServer *server, User *user, User *target, const ChatPacket *packet );
bool Unmute( ChatServer *server, User *user, User *target, const ChatPacket *packet );
bool Query( ChatServer *server, User *user, User *target );

const string GetAction( uint16_t iCode )
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
	default:		return "eaten";		// debugging
	}
}

/* returns a valid target if sName references a user on the server. */
inline User* GetTarget( ChatServer *server, const string &sName )
{
	User* target = server->GetUserByName( sName );

	// if not logged in, we don't mess with them
	if( target && !target->IsLoggedIn() )
		return NULL;

	return target;
}

bool UserAction( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
	{
		LOG->System( "%s tried to use mod command (%d) without permission, ignoring.", user->GetName().c_str(), packet->iCode );
		return false;
	}

	User *target = GetTarget( server, packet->sMessage );

	/* if we're affecting a user, let the moderators know what's happening.
	 * MUTE and UNMUTE handle themselves, so we don't print in those cases. */
	if( packet->iCode != USER_MUTE && packet->iCode != USER_UNMUTE )
	{
		string sMessage = (target) ? target->GetName() : packet->sMessage;
		sMessage += " was " + GetAction(packet->iCode) + " by " + user->GetName();
		server->WallMessage( sMessage );
	}

	switch( packet->iCode )
	{
	case USER_KICK:
	case USER_DISABLE:	return Remove( target, packet->iCode );
	case USER_BAN:		Remove(target, USER_BAN); return Ban( server, packet->sMessage );
	case USER_UNBAN:	return Unban( server, packet->sMessage );
	case USER_MUTE:		return Mute( server, user, target, packet );
	case USER_UNMUTE:	return Unmute( server, user, target, packet );
	case IP_QUERY:		return Query( server, user, target );
	default:
		LOG->Debug( "Hit a ModAction I don't know how to handle! Action %d", packet->iCode );
		return false;
	}
}

/* a removal action results in the disconnection of the targeted client. */
bool Remove( User *target, uint16_t iCode )
{
	if( target != NULL )
	{
		/* Just pass on the code. The client will kick/disable/ban as needed. */
		ChatPacket notify( iCode );
		target->Write( notify.ToString() );
		target->Kill();
	}

	return true;
}

bool Ban( ChatServer *server, const string &sName )
{
	// allow banning people even if they're not in the chat room
	server->GetBanList()->Add( sName );
	server->GetConnection()->Ban( sName );

	return true;
}

bool Unban( ChatServer *server, const string &sName )
{
	// allow unbanning people even if they're not in the chat room
	server->GetBanList()->Remove( sName );
	server->GetConnection()->Unban( sName );

	return true;
}

bool Mute( ChatServer *server, User *user, User *target, const ChatPacket *packet )
{
	server->GetMuteList()->Add( packet->sMessage );

	if( target )
	{
		target->SetMuted( true );

		// pass on the mute, who it affected, and who did it
		ChatPacket msg( USER_MUTE, target->GetName(), user->GetName() );
		server->Broadcast( msg );
	}

	return true;
}

bool Unmute( ChatServer *server, User *user, User *target, const ChatPacket *packet )
{
	server->GetMuteList()->Remove( packet->sMessage );

	if( target )
	{
		target->SetMuted( false );

		// pass on the unmute, who it affected, and who did it
		ChatPacket msg( USER_UNMUTE, target->GetName(), user->GetName() );
		server->Broadcast( msg );
	}

	return true;
}

bool Query( ChatServer *server, User *user, User *target )
{
	if( target == NULL )
		return false;

	// send a packet back to the caller giving the IP address
	ChatPacket query( IP_QUERY, target->GetName(), user->GetIP() );
	user->Write( query.ToString() );

	return true;
}

bool ModChat( ChatServer *server, User *user, const ChatPacket *packet )
{
	if( !user->IsMod() )
		return false;

	// TODO: handle this in a more standard fashion, not as a hack.
	const string sMessage = StringUtil::Format( "{%s} %s",
		user->GetName().c_str(), packet->sMessage.c_str() );

	server->WallMessage( sMessage );

	return true;
}

bool ForceClear( ChatServer *server, User *user, const ChatPacket *packet )
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
