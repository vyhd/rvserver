/* Linux specific calls, stuck here for abstraction. */

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include "StringUtil.h"
#include "FileUtil.h"
#include "logger/Logger.h"

using namespace std;

// the mode we apply to our created directories
// (read for all, rw for group, rwx for owner)
#define DEFAULT_MODE (S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH)

bool FileUtil::PathExists( const char *szPath )
{
	struct stat st;
	return stat(szPath, &st) == 0;
}

bool FileUtil::CreateDir( const char *szDir )
{
	string sDir( szDir );

	LOG->Stdout( "sDir: %s", sDir.c_str() );	

	// shortcut for convenience
	if( sDir.find_first_of('/') == string::npos )
	{
		LOG->Stdout( "sDir: no slash found" );
		return mkdir( szDir, DEFAULT_MODE ) == 0;
	}

	vector<string> vsDirs;
	StringUtil::Split( sDir, vsDirs, '/' );

	string sCurrentPath = "/";

	for( unsigned i = 0; i < vsDirs.size(); i++ )
		LOG->Stdout( "vsDirs[%i]: %s", i, vsDirs[i].c_str() );

	// <=, so we make sure we get the last element
	for( unsigned i = 0; i <= vsDirs.size(); i++ )
	{
		if( PathExists(sCurrentPath.c_str()) )
		{
			LOG->Stdout( "Path exists, continuing: %s", sCurrentPath.c_str() );
			sCurrentPath += vsDirs[i] + "/";
			continue;
		}

		if( mkdir(sCurrentPath.c_str(), DEFAULT_MODE) != 0 )
		{
			LOG->Stdout( "Making dir: %s", sCurrentPath.c_str() );
			printf( "mkdir failed: %s\n", strerror(errno) );
			return false;
		}
	}

	return true;
}
