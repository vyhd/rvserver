#ifndef PACKET_UTIL_H
#define PACKET_UTIL_H

#include <string>
#include <vector>

/* XXX: can we find a better place to declare this? */
void split( const std::string& in, std::vector<std::string>& add, const char delim );

namespace PacketUtil
{
	/* returns true if there's more than one packet in the string. */
	bool NeedsSplit( const std::string& in );

	/* splits a string into one string per packet. */
	void Split( const std::string& in, std::vector<std::string>& add );
}

#endif // PACKET_UTIL_H

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
