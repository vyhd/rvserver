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
