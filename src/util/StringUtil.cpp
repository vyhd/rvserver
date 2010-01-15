#include "StringUtil.h"
#include <algorithm>
#include <cstdarg>
#include <cctype>
#include <cstddef>

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
