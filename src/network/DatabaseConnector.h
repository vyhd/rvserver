/* We can't directly access the database, so we've got a proxy script that
 * does the work for us...ugly, but safe-ish. If, in the future, the database
 * interface changes, we can change this class and keep the other calls.
 */

#ifndef DATABASE_CONNECTOR_H
#define DATABASE_CONNECTOR_H

#include <string>
#include "Socket.h"

class User;
class Config;
class DatabaseWorker;

class DatabaseConnector
{
public:
	/* uses the given Config file to load preferences */
	DatabaseConnector( const Config *cfg );
	~DatabaseConnector();

	/* authenticates username/password and sets logged in/config */
	void Login( User *user, const std::string &passwd );
	void SavePrefs( const User *user );

	/* self-explanatory, I think */
	void Ban( const std::string &username );
	void Unban( const std::string &username );

private:
	DatabaseWorker* m_pWorker;
};

#endif // DATABASE_CONNECTOR_H

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
