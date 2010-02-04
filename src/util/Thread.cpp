#include <errno.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "Thread.h"

/* compatibility hack */
#define gettid() syscall(SYS_gettid)

/* removed until we actually implement this one */
#define LOG(x,y) {}

void Thread::Start( void*(*CreateFn)(void *), void *data )
{
	int ret = pthread_create( &m_Thread, NULL, CreateFn, data );
	if( ret )
	{
		LOG( "pthread_create: %s", strerror(errno) );
	}
}

void Thread::Stop()
{
	void *val;
	int ret = pthread_join( m_Thread, &val );
	if( ret )
	{
		LOG( "pthread_join: %s", strerror(errno) );
	}
}

void Thread::Pause()
{
	int ret = ptrace( PTRACE_ATTACH, int(m_iID), NULL, NULL );
	if( ret == -1 )
	{
		LOG( "ptrace failed: %s\n", strerror(errno) );
	}

	m_bPaused = true;
}

void Thread::Continue()
{
	int ret = ptrace( PTRACE_DETACH, int(m_iID), NULL, NULL );
	if( ret == -1 )
	{
		LOG( "ptrace failed: %s\n", strerror(errno) );
	}

	m_bPaused = false;
}

Mutex::Mutex()
{
	pthread_mutex_init( &m_Lock, NULL );
}

Mutex::~Mutex()
{
	pthread_mutex_destroy( &m_Lock ); 
}

int Mutex::Lock()
{
	return pthread_mutex_lock( &m_Lock ); 
}

int Mutex::Unlock()
{
	return pthread_mutex_unlock( &m_Lock ); 
}

bool Mutex::TryLock()
{
	return pthread_mutex_trylock(&m_Lock) == 0;
}

Spinlock::Spinlock()
{
	pthread_spin_init( &m_Lock, PTHREAD_PROCESS_SHARED );
}

Spinlock::~Spinlock()
{
	pthread_spin_destroy( &m_Lock );
}

int Spinlock::Lock()
{
	return pthread_spin_lock( &m_Lock );
}

int Spinlock::Unlock()
{
	return pthread_spin_unlock( &m_Lock );
}

bool Spinlock::TryLock()
{
	return pthread_spin_trylock(&m_Lock) == 0;
}
