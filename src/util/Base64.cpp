#include "Base64.h"
#include "logger/Logger.h"
#include "libb64/cencode.h"
#include <cstdio>

using namespace std;

const unsigned BUFFER_SIZE = 512;

string Base64::Encode( const string &str )
{
	base64_encodestate state;
	base64_init_encodestate( &state );

	// I don't think anyone's liable to have more than ~300 bytes of password...
	char out[BUFFER_SIZE];

	// sanity check
	if( str.length() > BUFFER_SIZE )
	{
		LOG->System( "Base64::Encode cannot encode! Too many characters (%u/%u)", str.length(), BUFFER_SIZE );
		return string();
	}

	int bytes = base64_encode_block( str.c_str(), str.length(), out, &state );
	bytes += base64_encode_blockend( out + bytes, &state );
	out[bytes-1] = '\0';

	return string(out);
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
