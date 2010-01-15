#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <vector>

/* The STL is fairly lacking in some functions, so we make our own. */

namespace StringUtil
{
	/* transforms a string in place to a lowercase string */
	void ToLower( std::string &in );

	/* compares two strings, up to len characters, case insensitively */
	int CompareNoCase( const std::string &s1, const std::string &s2,
		unsigned len = 0 );

	/* returns a std::string with the given format/printf() style args */
	std::string Format( const char *format, ... );

	/* splits in with delim into tokens, which are inserted into add. */
	void Split( const std::string &in, std::vector<std::string> &add, 
		const char delim );
}

/* Various STL algorithms (find_if, count_if, etc.) require a single-argument 
 * call to determine what should be found, counted, etc...we can't do a single
 * argument for string comparison, so this is the workaround. Essentially,
 * we cache the compared string in the object and compare an argument to it.
*/

class StringComp
{
public:
	StringComp( const std::string &str ) : m_sPattern(str) { }
	virtual ~StringComp() { }
	virtual bool operator()( const std::string &check )
	{
		return check.compare( 0, m_sPattern.length(), m_sPattern ) == 0;
	}
protected:
	const std::string& m_sPattern;
};

// forces all comparisons to lower-case, i.e. case insensitive
class StringCompI: public StringComp
{
public:
	StringCompI( const std::string &str ) : StringComp(str) { }
	virtual ~StringCompI() { }
	virtual bool operator()( const std::string &check )
	{
		return StringUtil::CompareNoCase( check, m_sPattern, m_sPattern.length() );
	}
};

#endif // STRING_UTIL_H
