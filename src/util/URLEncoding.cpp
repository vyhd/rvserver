#include <cstdio>
#include "URLEncoding.h"

using namespace std;

// returns true if the given character must be encoded.
static bool unsafe( char c )
{
	// 0 - 47 is are control and reserved characters
	// 123-255 are reserved, control, and non-ASCII
	if( c < 48 || c > 122 )
		return true;

	// 58-64 and 91-96 are reserved characters
	if( (c >= 58 && c <= 64) || (c >= 91 && c <= 96) )
		return true;

	return false;
}

string URLEncoding::Encode( const string &str )
{
	string ret;

	// buffer for printing hex values
	char buf[4];

	for( string::const_iterator it = str.begin(); it != str.end(); it++ )
	{
		const char &ch = *it;

		if( !unsafe(ch) )
		{
			ret.push_back( ch );
			continue;
		}

		// this character must be unsafe, so fix it
		snprintf( buf, 4, "%%%02X", ch );
		ret.append( buf );
	}

	return ret;
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
