#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

class Config
{
public:
	/* loads keys/values from a file path. */
	bool Load( const char *path );

	/* returns this key's entry, NULL if none exists. */
	const char* Get( const char *key );

	/* returns an integer value for this key, if available. */
	int GetInt( const char *key );

private:
	/* key/value configuration pairs */
	std::map<const std::string, const std::string> m_Entries;
};

#endif // CONFIG_H
