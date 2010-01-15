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
