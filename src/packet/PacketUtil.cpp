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

void PacketUtil::SplitPacket( const string& in, vector<string>& add )
{
	// packets are delimited by a newline.
	split( in, add, '\n' );
}
