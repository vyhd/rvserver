#include "StringUtil.h"
#include <algorithm>
#include <cstdarg>
#include <cctype>
#include <cstddef>

/* XXX: Unix-specific header for strcasecmp and strncasecmp */
extern "C"
{
#include <strings.h>
}

using namespace std;

void StringUtil::ToLower( std::string &in )
{
	transform( in.begin(), in.end(), in.begin(), tolower );
}

/* Lowercases two strings, performing an essentially case-insensitive match. */
int StringUtil::CompareNoCase( const string &s1, const string &s2, unsigned len )
{
	// compare up to len characters, if given
	if( len != 0 )
		return strncasecmp( s1.c_str(), s2.c_str(), len );

	return strcasecmp( s1.c_str(), s2.c_str() );
}

string StringUtil::Format( const char *fmt, ... )
{
	char buffer[256];
	memset( buffer, 0, sizeof(buffer) );

	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, 256, fmt, args );
	va_end( args );

	return string(buffer);
}

/* Splits components of in, separated by delim, and inserts copies into add. */
void StringUtil::Split( const string &in, vector<string> &add, const char delim )
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

void StringUtil::Split( const string &in, vector<char> &add, const char delim )
{
	// XXX: this is kind of ugly. Can we genericize this?
	vector<string> vsData;
	Split( in, vsData, delim );

	// insert the first character of each string (truncate the rest)
	for( unsigned i = 0; i < vsData.size(); ++i )
		add.push_back( vsData[i][0] );
}

/* Parses a string and returns the total elapsed seconds of the given tokens.
 * Supported tokens: w (week), d (day), h (hour), m (minute), s (second).
 * Usage is e.g. "1w 4d 8m" for the time spanned by 1 week, 4 days, 8 minutes.
 */
time_t StringUtil::ParseTime( const std::string &sMessage )
{
	time_t weeks, days, hours, minutes, seconds;
	weeks = days = hours = minutes = seconds = 0;

	vector<string> parts;
	Split( sMessage, parts, ' ' );

	for( unsigned i = 0; i < parts.size(); ++i )
	{
		unsigned value = 0;
		sscanf( parts[i].c_str(), "%u", &value );

		char cLastChar = parts[i].at( parts[i].length()-1 );

		switch( cLastChar )
		{
		case 'w': case 'W': weeks = value; break;
		case 'd': case 'D': days = value; break;
		case 'h': case 'H': hours = value; break;
		case 'm': case 'M': minutes = value; break;
		case 's': case 'S': seconds = value; break;
		}
	}

	time_t total = seconds;

	if( minutes )	total += minutes*60;
	if( hours )	total += hours*60*60;
	if( days )	total += days*24*60*60;
	if( weeks )	total += weeks*7*24*60*60;

	// protection against overflow and negative times
	if( total < 0 )
		return 0;

	return total;
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
