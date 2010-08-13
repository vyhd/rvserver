/* TimedList: a class that contains a central set of names and timestamps,
 * referenced in two other structures that sort by each. We're not using very
 * much memory overall, so we can afford more memory to make lookups faster.
 *
 * In the event of idiotic moderators adding garbage to this, we'll simply
 * rebuild the list if it reaches an unreasonably high amount of names.
 */

#ifndef TIMED_LIST_H
#define TIMED_LIST_H

#include <string>
#include <ctime>
#include <map>
#include <list>

/* Contains a list of names, a timestamp, and a deletion flag. */
struct ListEntry
{
	std::string name;
	time_t time;

	ListEntry( std::string name_, time_t time_ ) : name(name_), time(time_) { }
};

// convenience aliases
typedef std::map<std::string,ListEntry*> NameMap;
typedef std::pair<std::string,ListEntry*> NameEntry;

typedef std::map<time_t,ListEntry*> TimeMap;
typedef std::pair<time_t,ListEntry*> TimeEntry;

class TimedList
{
public:
	TimedList();
	~TimedList();

	void Add( const ListEntry &entry );
	void Remove( const std::string &name );

	// removes entries for which time has expired
	void Update();

private:
	/* Inserts the entry into the appropriate map. */
	void InsertName( ListEntry *entry );
	void InsertTime( ListEntry *entry );

	/* Main entry list */
	std::list<ListEntry> m_Entries;

	/* Sorted entry lists */
	NameMap	m_NameEntries;
	TimeMap	m_TimeEntries;

public:
	void DumpNames();
	void DumpTimes();
};

#endif // TIMED_LIST_H
