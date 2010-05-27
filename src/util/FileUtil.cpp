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

	// shortcut for convenience
	if( sDir.find_first_of('/') == string::npos )
		return mkdir( szDir, DEFAULT_MODE ) == 0;

	vector<string> vsDirs;
	StringUtil::Split( sDir, vsDirs, '/' );

	string sCurrentPath = "/";

	// <=, so we make sure we get the last element
	for( unsigned i = 0; i <= vsDirs.size(); ++i )
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

const char* FileUtil::GetConfigFilePath()
{
	/* We cache this, since it'll never change and so we can
	 * simply return a char* instead of a new std::string. */

	static string s_sConfigFilePath;

	if( !s_sConfigFilePath.empty() )
		return s_sConfigFilePath.c_str();

	const char* szHome = getenv( "HOME" );

	if( szHome )
	{
		s_sConfigFilePath = string(szHome) + "/.rvserver";
	}
	else
	{
		LOG->Stdout( "Warning: could not get $HOME!" );
		s_sConfigFilePath = "";
	}


	return GetConfigFilePath();
}
