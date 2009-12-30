/* Logger: logs chat and diagnostics to disk. It's kind of a singleton, but
 * done in a namespace, in order to make dependencies more obvious. */

#ifndef LOGGER_H
#define LOGGER_H

/* XXX: can we move this into the .cpp file? */
#include <cstdio>

class Logger
{
public:
	Logger( const char *szLogFile, const char *szSystemFile );
	~Logger();

	static void ChatLog( const char *str );

	/* writes a formatted string to the system log */
	static void SystemLog( const char *fmt, ... );

	static void Flush();

private:
	FILE*	m_pLogFile;
	FILE*	m_pSystemFile;
};

#endif // LOGGER_H
