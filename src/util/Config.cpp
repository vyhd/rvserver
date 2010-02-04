/* Expects INI-style data, though without any sections.
 * See the example file. */

#include <cstdio>
#include <cstdlib>	// for atoi()
#include <cerrno>
#include <string>
#include <vector>
#include "Config.h"
#include "logger/Logger.h"
#include "util/StringUtil.h"

using namespace std;

bool Config::Load( const char *path )
{
	FILE *pFile = fopen( path, "r" );

	if( pFile == NULL )
	{
		Logger::SystemLog( "Failed to open \"%s\": %s", path, strerror(errno) );
		return false;
	}

	/* not the fastest method, but the easiest, and we're not
	 * liable for realtime processing at any point, so... */

	// get the file size for this configuration
	fseek( pFile, 0, SEEK_END );
	unsigned iFileSize = ftell( pFile );
	rewind( pFile );

	char *data = new char[iFileSize];

	// read the entire file into memory
	if( fread(data, sizeof(char), iFileSize, pFile) < 0 )
	{
		Logger::SystemLog( "Failed to load \"%s\": %s", path, strerror(errno) );
		fclose( pFile );
		return false;
	}

	/* we have our data now, so we can clean up. */
	string sData( data );
	delete data;
	data = NULL;

	fclose( pFile );


	/* and this is where the magic happens! */
	{
		m_Entries.clear();

		vector<string> vsLines;
		StringUtil::Split( sData, vsLines, '\n' );

		// read each line and split the keys and values apart
		for( unsigned i = 0; i < vsLines.size(); i++ )
		{
			unsigned iSignPos = vsLines[i].find_first_of('=');

			// invalid line, ignore
			if( iSignPos == string::npos )
				continue;

			// split key=value into key, value
			const string sKey = vsLines[i].substr( 0, iSignPos );
			const string sValue = vsLines[i].substr( iSignPos+1 );

			Logger::SystemLog( "%s -> %s", sKey.c_str(), sValue.c_str() );

			/* insert this key/value pair into the map */
			m_Entries.insert( pair<const string, const string>(sKey,sValue) );
		}
	}

	return true;
}

const char* Config::Get( const char *key )
{
	map<const string, const string>::const_iterator it = m_Entries.find( key );

	if( it == m_Entries.end() )
	{
		Logger::SystemLog( "%s not found", key );
		return NULL;
	}

	Logger::SystemLog( "found value \"%s\" for key %s", it->second.c_str(), key );
	return it->second.c_str();
}

int Config::GetInt( const char *key )
{
	const char *val = Get( key );

	if( val == NULL )
		return 0;

	return atoi( val );
}
