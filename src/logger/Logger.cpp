#include "Logger.h"
#include <ctime>
#include <cstdarg>

const char* LOG_FILE = "log.txt";
const char* SYSTEM_FILE = "system.txt";

/* Like, but not quite a singleton. */
static Logger g_Logger( LOG_FILE, SYSTEM_FILE );

/* writes a line into the given log file with timestamp and newline. */
static void WriteLine( const char *str, FILE *pFile, Spinlock *spinlock )
{
	if( pFile == NULL )
		return;

	spinlock->Lock();

	/* write a timestamp (e.g. [11:22:33]) with trailing space */
	char timestamp[12];
	time_t now = time(NULL);
	struct tm *timeval = localtime( &now );
	strftime( timestamp, 12, "[%X] ", timeval );

	fputs( timestamp, pFile );

	/* now, put the original string and a newline. */
	fputs( str, pFile );
	fputc( '\n', pFile );
	spinlock->Unlock();
}

Logger::Logger( const char *szLogPath, const char *szSystemPath )
{
	/* append to the log file, but create a new system log. */
	m_pLogFile = fopen( szLogPath, "a" );

	if( !m_pLogFile )
		perror( "error error danger will robinson" );

	m_pSystemFile = fopen( szSystemPath, "w" );

	if( !m_pSystemFile )
		perror( "oh dear god the system's down we are doomed" );

	if( !m_pLogFile || !m_pSystemFile )
	{
		printf( "Oh, bother. Something went dreadfully wrong." );
		return;
	}

	m_pLock = new Spinlock;

	/* write date/time headers for logging purposes */
	char time_header[32];

	time_t now = time(NULL);
	struct tm *timeval = localtime( &now );

	strftime( time_header, 32, "*** Log started on %x ***", timeval );

	ChatLog( time_header );
	SystemLog( time_header );
}

Logger::~Logger()
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
	fclose( m_pLogFile );
	fclose( m_pSystemFile );

	delete m_pLock;
	m_pLock = NULL;
}

void Logger::ChatLog( const char *str )
{
	if( !g_Logger.m_pLogFile )
		return;

	WriteLine( str, g_Logger.m_pLogFile, g_Logger.m_pLock );
	puts( str );
}

void Logger::SystemLog( const char *fmt, ... )
{
	if( !g_Logger.m_pSystemFile )
		return;

	/* to cut down on processing, we write once and output twice. */
	char buf[256];

	va_list args;
	va_start( args, fmt );
	vsnprintf( buf, 256, fmt, args );
	va_end( args );

	/* write the string to the system file and stdout. */
	WriteLine( buf, g_Logger.m_pSystemFile, g_Logger.m_pLock );
	puts( buf );
}

void Logger::Flush()
{
	fflush( g_Logger.m_pLogFile );
	fflush( g_Logger.m_pSystemFile );
}
