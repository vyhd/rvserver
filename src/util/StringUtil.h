#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>

/* The STL is fairly lacking in some functions, so we make our own. */

namespace StringUtil
{
	/* transforms a string in place to a lowercase string */
	void ToLower( std::string &in );

	/* compares two strings, up to len characters, case insensitively */
	int CompareNoCase( const std::string &s1, const std::string &s2,
		unsigned len = 0 );

	/* returns a std::string with the given format/printf() style args */
	std::string Format( const char *format, ... );

	/* splits in with delim into tokens, which are inserted into add. */
	void Split( const std::string &in, std::vector<std::string> &add, 
		const char delim );

	/* splits in with delim into chars, which are inserted into add. */
	void Split( const std::string &in, std::vector<char> &add,
		const char delim );
}

#endif // STRING_UTIL_H

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
