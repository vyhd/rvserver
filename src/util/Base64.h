#ifndef BASE_64_H
#define BASE_64_H

#include <string>

namespace Base64
{
	std::string Encode( const std::string &str );
	std::string Decode( const std::string &str );
}

#endif // BASE_64_H
