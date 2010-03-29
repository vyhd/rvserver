/* Logger: logs chat and diagnostics to disk. It's a singleton, but
 * done in a namespace, in order to make dependencies more obvious. */

#ifndef LOGGER_H
#define LOGGER_H

namespace Logger
{
	/* opens files for logging from this directory */
	bool Open( const char *szLogDir );

	/* closes all files, cleans up, and stops logging */
	void Close();

	/* writes a string, as is, to the chat log */
	void ChatLog( const char *str );

	/* writes a formatted string to the system log */
	void SystemLog( const char *fmt, ... );

	/* writes a formatted string to the debug log */
	void DebugLog( const char *fmt, ... );

	/* flushes all log output */
	void Flush();
}

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
