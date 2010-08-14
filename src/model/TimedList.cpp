#include <climits>
#include "TimedList.h"
#include "util/StringUtil.h"
#include "logger/Logger.h"

namespace
{
	#include <cstdio>
	#include <sys/time.h>
	#include <stdint.h>

	static int64_t getusecs()
	{
		struct timeval tv;
		gettimeofday( &tv, NULL );

		int64_t iRet = int64_t(tv.tv_sec) * 1000000 + int64_t(tv.tv_usec);
		return iRet;
	}
}

using namespace std;

TimedList::TimedList()
{
	m_Entries.clear();
	m_NameEntries.clear();
	m_TimeEntries.clear();
}

TimedList::~TimedList()
{
	m_Entries.clear();
	m_NameEntries.clear();
	m_TimeEntries.clear();
}

void TimedList::Add( const string &sName )
{
	LOG->Debug( "TimedList::Add( %s )", sName.c_str() );
	Add( ListEntry(sName,LONG_MAX) );
}

void TimedList::Add( const ListEntry &entry_ )
{
	LOG->Debug( "TimedList::Add( %s, %d )", entry_.name.c_str(), entry_.time );

	// lowercase the name so we can compare case insensitively
	ListEntry entry( entry_ );
	StringUtil::ToLower( entry.name );

	// if we have the entry already, just update it and return
	NameMap::iterator it = m_NameEntries.find( entry.name );

	if( it != m_NameEntries.end() )
	{
		it->second->time = entry.time;
		return;
	}

	m_Entries.push_back( entry );
	ListEntry *pEntry = &m_Entries.back();

	m_NameEntries.insert( NameEntry(entry.name, pEntry) );
	m_TimeEntries.insert( TimeEntry(entry.time, pEntry) );
}

void TimedList::Remove( const string &name_ )
{
	LOG->Debug( "TimedList::Remove( %s )", name_.c_str() );

	uint64_t start = getusecs();

	string name = name_;
	StringUtil::ToLower( name );

	ListEntry *pEntry = NULL;

	// check to see if this is in the name map
	{
		NameMap::iterator it = m_NameEntries.find(name);

		// if we have an entry, point to it
		if( it != m_NameEntries.end() )
			pEntry = it->second;
	}

	if( pEntry == NULL )
		return;

	// find this entry in the main list and erase it
	m_NameEntries.erase( name );
	m_TimeEntries.erase( pEntry->time );

	list<ListEntry>::iterator it = m_Entries.begin();

	// 
	for( it = m_Entries.begin(); it != m_Entries.end(); ++it )
		if( &*it == pEntry )
			break;

	if( it == m_Entries.end() )
	{
		LOG->System( "What? Coherency problem between main entry list and lookup lists..." );
		return;	// nothing we can do
	}

	m_Entries.erase( it );

	uint64_t end = getusecs();

	LOG->Debug( "Removing \"%s\" took %llu usecs.", name.c_str(), end - start );
}

bool TimedList::HasName( const string &name ) const
{
	bool ret = m_NameEntries.find(name) != m_NameEntries.end();

	LOG->Debug( "TimedList::HasName( %s ) returning %d", name.c_str(), int(ret) );
	return ret;
}

void TimedList::Update()
{
	const time_t now = time(NULL);

	for( TimeMap::iterator it = m_TimeEntries.begin(); it != m_TimeEntries.end(); ++it )
	{
		// entries are in order of timestamp, so we can break
		// on the first timestamp that's later than now.
		if( now < it->first )
			break;

		ListEntry *pEntry = it->second;
		LOG->Debug( "Removing entry for \"%s\" (now = %u, then = %u)\n", pEntry->name.c_str(), now, pEntry->time );
		Remove( pEntry->name );
	}
}

void TimedList::DumpNames()
{
	NameMap::const_iterator it = m_NameEntries.begin();
	int i = 0;

	for( ; it != m_NameEntries.end(); ++it )
		printf( "Name entry %u: %s (%li)\n", ++i, it->first.c_str(), it->second->time );
}

void TimedList::DumpTimes()
{
	TimeMap::const_iterator it = m_TimeEntries.begin();
	int i = 0;

	for( ; it != m_TimeEntries.end(); ++it )
		printf( "Time entry %u: %s (%li)\n", ++i, it->second->name.c_str(), it->first );
}
