#include "packet/PacketUtil.h"
#include "logger/Logger.h"
#include <cstddef>

using namespace std;

void split( const string& in, vector<string>& add, const char delim )
{
	size_t start = 0;
	size_t next = 0;

	while( 1 )
	{
		start = in.find_first_not_of( delim, next );
		next = in.find_first_of( delim, start );

		// no more tokens to split, so stop looping
		if( start == string::npos && next == string::npos )
			break;

		add.push_back( in.substr(start, (next-start)) );
	}
}

/* a packet is suffixed with "\n"; we can use that
 * as a criterion for determining multiple packets. */
bool PacketUtil::NeedsSplit( const string& in )
{
	// if the first "\n" is not at the end, we have multiple packets.
	return in.length() - 1 != in.find_first_of("\n");
}

void PacketUtil::Split( const string& in, vector<string>& add )
{
	// packets are delimited by a newline.
	split( in, add, '\n' );
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
