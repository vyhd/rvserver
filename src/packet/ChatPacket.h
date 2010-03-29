/* ChatPacket: contains the separated pieces of data from a data packet. */

#ifndef CHAT_PACKET_H
#define CHAT_PACKET_H

#include <string>
#include <stdint.h>

const uint16_t INVALID_CODE = 0xFFFF;

// defines a blank packet field
const std::string BLANK = "_";

struct ChatPacket
{
public:

	/* Default constructor. */
	ChatPacket();

	/* Copy constructor. */
	ChatPacket( const ChatPacket &other );

	/* Constructor for basic creation (no args). */
	ChatPacket( uint16_t iCode );

	/* Constructor for partial creation (no colours). */
	ChatPacket( uint16_t iCode, const std::string &sUsername,
		const std::string &sMessage );

	/* Constructor for full packet creation. */
	ChatPacket( uint16_t iCode, const std::string &sUsername,
		const std::string &sMessage, uint8_t r, uint8_t g, uint8_t b );

	/* Shortcut constructor for ChatPacket() and FromString(str). */
	ChatPacket( const std::string &sData );

	/* Loads packet data from a string, returns true if well-formed */
	bool FromString( const std::string &sData );

	/* Returns a network-formatted string from the packet */
	std::string ToString() const;

	/* if IsValid, contains valid packet data. */
	bool IsValid() const	{ return iCode != INVALID_CODE; }

public:
	uint16_t iCode;
	std::string sUsername;
	std::string sMessage;
	uint8_t	r, g, b;

};

#endif // CHAT_PACKET_H

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
