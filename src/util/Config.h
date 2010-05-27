#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

// convenience aliases
typedef std::map<const std::string, const std::string> KeyValMap;
typedef std::pair<const std::string, const std::string> KeyValPair;

class Config
{
public:
	/* loads keys/values from a file path. */
	bool Load( const char *path );

	/* Returns this key's entry, NULL if none exists. If
	 * bOptional is false, log an error and abort. */
	const char* Get( const char *key, bool bOptional = false, const char *def = 0 ) const;

	/* returns a specialized value; if not found, and marked optional,
	 * then we'll fall back on the default value given in the argument. */
	int GetInt( const char *key, bool bOptional = false, int def = 0 ) const;
	bool GetBool( const char *key, bool bOptional = false, bool def = false ) const;

private:
	/* contains key/value configuration values */
	KeyValMap m_Entries;
};

#endif // CONFIG_H

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
