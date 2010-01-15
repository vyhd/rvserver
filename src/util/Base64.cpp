#include "Base64.h"
#include "logger/Logger.h"
#include "libb64/cencode.h"
#include <cstdio>

using namespace std;

string Base64::Encode( const string &str )
{
	base64_encodestate state;
	base64_init_encodestate( &state );

	// I don't think anyone's liable to have more than ~300 bytes of password...
	char out[512];

	int bytes = base64_encode_block( str.c_str(), str.length(), out, &state );
	bytes += base64_encode_blockend( out + bytes, &state );
	out[bytes-1] = '\0';

	return string(out);
}
