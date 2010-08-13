#include "TimedList.h"

// testing:
//#define STANDALONE

#if !defined(STANDALONE)
#include "logger/Logger.h"
#endif

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

void TimedList::Add( const ListEntry &entry )
{
	// Do we already have this entry?
	ListEntry *pEntry = m_NameEntries[entry.name];

	if( !pEntry )
	{
		m_Entries.push_back( entry );
		pEntry = &m_Entries.back();
	}

	m_NameEntries[entry.name] = pEntry;
	m_TimeEntries[entry.time] = pEntry;
}

void TimedList::Remove( const std::string &name )
{
	uint64_t start = getusecs();

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
	m_NameEntries.erase( pEntry->name );
	m_TimeEntries.erase( pEntry->time );

	list<ListEntry>::iterator it = m_Entries.begin();

	// 
	for( it = m_Entries.begin(); it != m_Entries.end(); ++it )
		if( &*it == pEntry )
			break;

	if( it == m_Entries.end() )
	{
#if !defined(STANDALONE)
		LOG->System( "What? Coherency problem between main entry list and lookup lists..." );
#endif
		return;	// nothing we can do
	}

	m_Entries.erase( it );

	uint64_t end = getusecs();

#if defined(STANDALONE)
	printf( "Removing \"%s\" took %llu usecs.\n", name.c_str(), end - start );
#else
	LOG->Debug( "Removing \"%s\" took %llu usecs.", name.c_str(), end - start );
#endif
}

bool TimedList::HasName( const string &name ) const
{
	return m_NameEntries.find(name) != m_NameEntries.end();
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
#if defined(STANDALONE)
		printf( "Removing entry for \"%s\" (now = %u, then = %u)\n", pEntry->name.c_str(), now, pEntry->time );
#else
		LOG->Debug( "Removing entry for \"%s\" (now = %u, then = %u)\n", pEntry->name.c_str(), now, pEntry->time );
#endif
		Remove( pEntry->name );
	}
}

void TimedList::DumpNames()
{
	NameMap::const_iterator it = m_NameEntries.begin();
	int i = 0;

	for( ; it != m_NameEntries.end(); ++it )
		printf( "Name entry %u: %s (%u)\n", ++i, it->first.c_str(), it->second->time );
}

void TimedList::DumpTimes()
{
	TimeMap::const_iterator it = m_TimeEntries.begin();
	int i = 0;

	for( ; it != m_TimeEntries.end(); ++it )
		printf( "Time entry %u: %s (%u)\n", ++i, it->second->name.c_str(), it->first );
}

#if defined(STANDALONE)
int main()
{
	TimedList list;

	ListEntry entry( "Fire_Adept", 1 );

	list.Add( entry );
	list.Add( ListEntry("monkeymom2",	2) );
	list.Add( ListEntry("Cosmos",		3) );
	list.Add( ListEntry("hiker",		4) );
	list.Add( ListEntry("henner",		5) );
	list.Add( ListEntry("Muse",		6) );
	list.Add( ListEntry("Sarah",		7) );
	list.Add( ListEntry("kikori_kid",	8) );
	list.Add( ListEntry("Chroz",		9) );

	list.DumpNames();
	printf( "-----\n" );
	list.DumpTimes();

	printf( "Updating, which should remove everything...\n" );

	list.Update();

	list.DumpNames();
	printf( "-----\n" );
	list.DumpTimes();

	return 0;
}
#endif
