/* Logger: logs chat and diagnostics to disk. It's a singleton, but
 * done in a namespace, in order to make dependencies more obvious. */

/* Note: LOG->Stdout is always safe to call, even if LOG is NULL. */

#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>

class Spinlock;
class Config;

class Logger
{
public:
	Logger( Config *cfg );

	/* closes all files, cleans up, and stops logging */
	~Logger();

	/* opens files for logging from this directory */
	bool Open( const char *szLogPath );

	/* writes a string, as is, to the chat log */
	void Chat( const char *str );

	/* writes a formatted string to the system log */
	void System( const char *fmt, ... );

	/* writes a formatted string to the debug log */
	void Debug( const char *fmt, ... );

	/* writes a string to stdout (basically, printf).
	 * this is safe even if we haven't opened. */
	void Stdout( const char *fmt, ... );

	/* flushes all log output */
	void Flush();

private:
	// opens a file path, sets pFile on success
	bool OpenFile( const char *szFilePath, const char *szMode, FILE **pFile );

	FILE* m_pChatLog;
	FILE* m_pSystemLog;
	FILE* m_pDebugLog;

	// is debugging enabled?
	bool m_bDebugLog;

	// make sure only one call writes to any given file at a time.
	// this is slower than it could be, but the benefit of having
	// a spinlock per file is probably negated by it complexity...
	Spinlock *m_pFileLock;
};

// declare an extern for the LOG singleton
extern Logger* LOG;

#endif // LOGGER_H

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
