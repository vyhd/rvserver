/* Logger: logs chat and diagnostics to disk. It's kind of a singleton, but
 * done in a namespace, in order to make dependencies more obvious. */

#ifndef LOGGER_H
#define LOGGER_H

/* XXX: can we move this into the .cpp file? */
#include <cstdio>
#include "util/Thread.h"

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

	/* Used to make logging thread safe. Printing data runs on the order
	 * of microseconds, so we can avoid kernel context switches here. */
	Spinlock* m_pLock;
};

#endif // LOGGER_H
