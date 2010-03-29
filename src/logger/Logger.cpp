#include "Logger.h"
#include "util/Config.h"
#include "util/Thread.h"
#include <cstdio>
#include <cerrno>
#include <ctime>
#include <cstdarg>
#include <string>

using namespace std;

const char* CHAT_LOG = "log.txt";
const char* SYSTEM_LOG = "system.txt";
const char* DEBUG_LOG = "debug.txt";

namespace
{
	/* Used to make logging thread safe. Printing data runs on the order
	 * of microseconds, so we can avoid kernel context switches here. */
	Mutex g_PrintLock;

	/* This is kind of ugly, but eh... */
	FILE *g_pChatLog, *g_pSystemLog, *g_pDebugLog;
}

/* writes a line into the given log file with timestamp and newline. */
static void WriteLine( const char *str, FILE *pFile )
{
	if( pFile == NULL )
		return;

	/* write a timestamp (e.g. [11:22:33]) with trailing space */
	char timestamp[12];
	time_t now = time(NULL);
	struct tm *timeval = localtime( &now );
	strftime( timestamp, 12, "[%X] ", timeval );

	g_PrintLock.Lock();
	fputs( timestamp, pFile );

	/* now, put the original string and a newline. */
	fputs( str, pFile );
	fputc( '\n', pFile );

	g_PrintLock.Unlock();
}

bool Logger::Open( const char *szLogDir )
{
	string sDir( szLogDir );

	/* check each error separately */
	int errno1 = 0, errno2 = 0, errno3 = 0;

	/* append to the chat and system logs, make new debug logs. */
	g_pChatLog = fopen( (sDir + CHAT_LOG).c_str(), "a" );

	if( g_pChatLog == NULL )
		errno1 = errno;

	g_pSystemLog = fopen( (sDir + SYSTEM_LOG).c_str(), "a" );

	if( g_pSystemLog == NULL )
		errno2 = errno;

	g_pDebugLog = fopen( (sDir + DEBUG_LOG).c_str(), "w" );

	if( g_pDebugLog == NULL )
		errno3 = errno;

	if( !g_pChatLog || !g_pSystemLog || !g_pDebugLog )
	{
		printf( "Oh, bother. Encountered problems opening:\n" );

		if( g_pChatLog == NULL )	printf( "\t%s (%s)\n", CHAT_LOG, strerror(errno1) );
		if( g_pSystemLog == NULL )	printf( "\t%s (%s)\n", SYSTEM_LOG, strerror(errno2) );
		if( g_pDebugLog == NULL )	printf( "\t%s (%s)\n", DEBUG_LOG, strerror(errno3) );

		return false;
	}

	/* write date/time headers for logging purposes */
	char time_header[32];

	time_t now = time(NULL);
	struct tm *timeval = localtime( &now );

	strftime( time_header, 32, "*** Log started on %x ***", timeval );

	ChatLog( time_header );
	SystemLog( time_header );
	DebugLog( time_header );

	return true;
}

void Logger::Close()
{
	/* write the ending date/time header */
	char time_header[36];

	time_t now = time(NULL);
	struct tm *timeval = localtime( &now );

	strftime( time_header, 36, "*** Log ended on %x ***\n\n", timeval );

	ChatLog( time_header );
	SystemLog( time_header );

	/* flush the file buffers and close the handles */
	Flush();
	fclose( g_pChatLog );
	fclose( g_pSystemLog );
	fclose( g_pDebugLog );
}

static char s_VABuffer[2048];

static const char* FormatVA( const char *fmt, va_list args )
{
	vsnprintf( s_VABuffer, 2048, fmt, args );
	return s_VABuffer;
}

void Logger::ChatLog( const char *str )
{
	if( !g_pChatLog )
		return;

	// HACK: remove the ending newline, so we don't \n\n.
	const string sLine = string(str);
	const string sToWrite = sLine.substr(0,sLine.find_last_of("\n"));
	WriteLine( sToWrite.c_str(), g_pChatLog );
}

void Logger::SystemLog( const char *fmt, ... )
{
	if( !g_pSystemLog )
		return;

	va_list args;
	va_start( args, fmt );
	const char *buf = FormatVA( fmt, args );
	va_end( args );

	/* write the string to the system file. */
	WriteLine( buf, g_pSystemLog );
	puts( buf );
}

void Logger::DebugLog( const char *fmt, ... )
{
	if( !g_pDebugLog )
		return;

	va_list args;
	va_start( args, fmt );
	const char *buf = FormatVA( fmt, args );
	va_end( args );

	/* write the string to the debug file and stdout. */
	WriteLine( buf, g_pDebugLog );
	puts( buf );
}

void Logger::Flush()
{
	fflush( g_pChatLog );
	fflush( g_pSystemLog );
	fflush( g_pDebugLog );
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
