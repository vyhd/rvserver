#ifndef FILE_UTIL_H
#define FILE_UTIL_H

namespace FileUtil
{
	// returns true if the given path exists
	bool PathExists( const char *szDir );

	// attempts to create a directory path recursively
	bool CreateDir( const char *szDir );
}

#endif // FILE_UTIL
