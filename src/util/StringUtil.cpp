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

/* multipliers to represent the given amount of time in seconds */
const unsigned MINUTE = 60;
const unsigned HOUR = MINUTE * 60;
const unsigned DAY = HOUR * 24;
const unsigned WEEK = DAY * 7;
const unsigned YEAR = WEEK * 52;

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
		case 'w': case 'W': weeks = value * WEEK; break;
		case 'd': case 'D': days = value * DAY; break;
		case 'h': case 'H': hours = value * HOUR; break;
		case 'm': case 'M': minutes = value * MINUTE; break;
		case 's': case 'S': seconds = value; break;
		}
	}

	time_t total = seconds + minutes + hours + days + weeks;

	// protection against overflow and negative times
	if( total < 0 )
		return 0;

	return total;
}

string print( unsigned val, const char *name )
{
	static char buffer[16];

	snprintf( buffer, 16, "%d %s%c", val, name, val == 1 ? '\0' : 's' );
	return string(buffer);
}

const char* NAMES[5] = { "week", "day", "hour", "minute", "second" };

/* Returns a string that describes the given amount of time as
 * succinctly as possible (ignoring months: too much processing). */
string StringUtil::TimeToString( time_t time )
{
	if( time > YEAR )
		return "a really long time";

	unsigned times[5] = { 0, 0, 0, 0, 0 };
	
	times[0] = (time / WEEK);	time %= WEEK;
	times[1] = (time / DAY);	time %= DAY;
	times[2] = (time / HOUR);	time %= HOUR;
	times[3] = (time / MINUTE);	time %= MINUTE;
	times[4] = time;

	string ret;

	{
		vector<string> entries;

		for( unsigned i = 0; i < 5; ++i )
			if( times[i] != 0 )
				entries.push_back( print(times[i], NAMES[i]) );

		const unsigned len = entries.size();

		for( unsigned i = 0; i < len; ++i )
		{
			if( i == (len-1) && len != 1 )
				ret += "and ";

			ret += entries[i];

			if( i < (len-1) )
				ret += ", ";
		}
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
