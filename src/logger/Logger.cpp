#include "Logger.h"
#include "util/FileUtil.h"
#include "util/Config.h"
#include "util/Thread.h"
#include <cstdio>
#include <cerrno>
#include <ctime>
#include <cstdarg>
#include <string>

using namespace std;
using namespace FileUtil;

Logger* LOG;

#define DO_IF_OPEN(x,func) if(x) func(x);

Spinlock g_FileLock;

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

	// make sure only one file is written to at a time */
	g_FileLock.Lock();

	/* put the timestamp, the original string, and a newline. */
	fputs( timestamp, pFile );
	fputs( str, pFile );
	fputc( '\n', pFile );

	g_FileLock.Unlock();
}

static void WriteTimeHeader( const char *str, FILE *pFile )
{
	char timestamp[16];
	char header[128];

	time_t now = time(NULL);
	strftime( timestamp, 16, "%x", localtime(&now) );

	snprintf( header, 128, "\n*** %s - %s ***\n", str, timestamp );

	g_FileLock.Lock();
	fputs( header, pFile );
	g_FileLock.Unlock();
}

Logger::Logger( Config *cfg )
{
	// use debugging info unless explicitly disabled
	m_bDebugLog = cfg->GetBool( "DebugMode", true, true );

	m_pChatLog = m_pSystemLog = m_pDebugLog = NULL;
}

Logger::~Logger()
{
	/* write the ending date/time header */

	WriteTimeHeader( "Log ended", m_pChatLog );
	WriteTimeHeader( "Log ended", m_pSystemLog );
	WriteTimeHeader( "Log ended", m_pDebugLog );

	/* flush the file buffers and close the handles */
	Flush();

	DO_IF_OPEN( m_pChatLog, fclose );
	DO_IF_OPEN( m_pSystemLog, fclose );
	DO_IF_OPEN( m_pDebugLog, fclose );
}

bool Logger::Open( const char *szLogDir )
{
	if( !PathExists(szLogDir) && !CreateDir(szLogDir) )
	{
		Stdout( "Cannot create log file directory: %s", szLogDir );
		return false;
	}

	const string sDir( szLogDir );

	const string sChatLog = sDir + "chatlog.txt";
	const string sSystemLog = sDir + "system.txt";
	const string sDebugLog = sDir + "debug.txt";

	bool bAllOpened = true;

	// "&=" will pull down to false if any open fails
	bAllOpened &= OpenFile( sChatLog.c_str(),	"a",	&m_pChatLog );
	bAllOpened &= OpenFile( sSystemLog.c_str(),	"w",	&m_pSystemLog );

	if( m_bDebugLog )
		bAllOpened &= OpenFile( sDebugLog.c_str(), "w", &m_pDebugLog );

	return bAllOpened;
}

bool Logger::OpenFile( const char *szFilePath, const char *szMode, FILE **pFile )
{
	*pFile = fopen( szFilePath, szMode );

	if( *pFile == NULL )
	{
		Stdout( "Failed to open %s: %s", szFilePath, strerror(errno) );
		return false;
	}

	Stdout( "Opened log file \"%s\"", szFilePath );
	WriteTimeHeader( "Log started", *pFile );

	return true;
}	

static char s_VABuffer[2048];

string FormatVA( const char *fmt, va_list args )
{
	vsnprintf( s_VABuffer, 2048, fmt, args );
	return string(s_VABuffer);
}

void Logger::Chat( const char *str )
{
	if( !m_pChatLog )
		return;

	// HACK: strip any newlines added by ChatPacket::ToString.
	const string sStr( str );
	const string sLine( sStr, 0, sStr.find_first_of("\n") );

	WriteLine( sLine.c_str(), m_pChatLog );
}

void Logger::System( const char *fmt, ... )
{
	if( !m_pSystemLog )
		return;

	va_list args;
	va_start( args, fmt );

	string sData = FormatVA( fmt, args );
	const char *buf = sData.c_str();

	va_end( args );

	/* write the string to the system file. */
	WriteLine( buf, m_pSystemLog );
	Stdout( buf );
}

void Logger::Debug( const char *fmt, ... )
{
	if( !m_pDebugLog || !m_bDebugLog )
		return;

	va_list args;
	va_start( args, fmt );

	string sData = FormatVA( fmt, args );
	const char *buf = sData.c_str();

	va_end( args );

	/* write the string to the debug file and stdout. */
	WriteLine( buf, m_pDebugLog );
	puts( buf );
}

void Logger::Stdout( const char *fmt, ... )
{
	va_list args;
	va_start( args, fmt );

	vprintf( fmt, args );
	printf( "\n" );

	va_end( args );
}

void Logger::Flush()
{
	DO_IF_OPEN( m_pChatLog, fflush );
	DO_IF_OPEN( m_pSystemLog, fflush );
	DO_IF_OPEN( m_pDebugLog, fflush );
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
