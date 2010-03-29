/* Expects INI-style data. You know the type. */

#include <cstdio>
#include <cstdlib>	// for atoi()
#include <cerrno>
#include <string>
#include <csignal>
#include <vector>
#include <map>
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
		delete[] data;
		return false;
	}

	/* we have our data now, so we can clean up. */
	string sData( data );
	delete[] data;
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

			Logger::DebugLog( "%s -> %s", sKey.c_str(), sValue.c_str() );

			/* insert this key/value pair into the map */
			m_Entries.insert( KeyValPair(sKey,sValue) );
		}
	}

	return true;
}

const char* Config::Get( const char *key, bool bOptional ) const
{
	KeyValMap::const_iterator it = m_Entries.find( key );

	if( it == m_Entries.end() )
	{
		if( bOptional )
			return NULL;

		// we're lacking vital configuration: force termination.
		Logger::SystemLog( "ERROR: required config option \"%s\" not found", key );
		raise( SIGTERM );
		return NULL;
	}

	Logger::DebugLog( "found value \"%s\" for key %s", it->second.c_str(), key );
	return it->second.c_str();
}

int Config::GetInt( const char *key, bool bOptional, int def ) const
{
	const char *val = Get( key, bOptional );

	if( val == NULL )
		return def;

	return atoi( val );
}

bool Config::GetBool( const char *key, bool bOptional, bool def ) const
{
	const char *val = Get( key, bOptional );

	if( val == NULL )
		return def;

	if( atoi(val) == 0 )
		return false;

	return true;
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
