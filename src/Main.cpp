#include <cstring>
#include <cerrno>

#include "ChatServer.h"
#include "logger/Logger.h"
#include "model/User.h"
#include "util/FileUtil.h"
#include "util/Config.h"

extern "C" {
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
}

using namespace std;

// global pointers, for signal callbacks
ChatServer *g_pServer = NULL;
Config *g_pConfig = NULL;

int g_LockFile = -1;

const char* const LOCK_FILE_PATH = "/tmp/rvserver-lock";

/* On caught signal, add a message and flush logs before exiting. */
static void clean_exit( int signum )
{
	// concurrency guard: only run this handler once
	static bool g_bInHandler = false;

	if( g_bInHandler )
		return;

	g_bInHandler = true;

	/* Tell the server to clean up its connections. This will
	 * take a while: all users need to be removed from the server
	 * and their preferences must be saved. */

	if( g_pServer )
	{
		LOG->Stdout( "Stopping server, please be patient..." );

		delete g_pServer;
		g_pServer = NULL;

		LOG->Stdout( "Server stopped, thanks for waiting :)" );
	}

	// flush and close all log files
	if( LOG )
	{
		delete LOG;
		LOG = NULL;
	}

	// delete the lock file, if it exists and we own it
	if( g_LockFile > 0 )
	{
		close( g_LockFile );
		unlink( LOCK_FILE_PATH );
		g_LockFile = -1;
	}

	exit(signum);
}

static void HandleSignal( int signum )
{
	switch( signum )
	{
	case SIGHUP:	/* reload config and restart */
		{
			if( !g_pServer )
				return;

			if( LOG )
				LOG->System( "Caught SIGHUP! Reloading config..." );

			g_pServer->Stop();
			g_pConfig->Load( FileUtil::GetConfigFilePath() );
			g_pServer->Start();

			break;
		}

	case SIGINT:
	case SIGTERM:
	case SIGSEGV:
	case SIGABRT:
		clean_exit( signum );
		break;

	default:
		LOG->Stdout( "Unhandled signal: %i\n", signum );
	}
}

static void daemonize()
{
	/* Check for the lock file. If it exists, exit. */
	if( access(LOCK_FILE_PATH, F_OK) == 0 )
	{
		LOG->Stdout( "Lock file exists! Exiting..." );
		clean_exit( EXIT_SUCCESS );
	}

	/* Attempt to fork. */
	pid_t pid = fork();

	if( pid > 0 )
	{
		clean_exit( EXIT_SUCCESS );
	}
	else if( pid < 0 )
	{
		LOG->Stdout( "Failed to fork: %s\n", strerror(errno) );
		clean_exit( EXIT_FAILURE );
	}

	/* If fork returns 0, we're a daemon. Yay. */	

	/* Allow everyone to access our files. */
	umask( 0 );

	/* Try to get a unique session ID. */
	pid_t sid = setsid();

	if( sid < 0 )
	{
		LOG->Stdout( "Failed to get a session ID! %s\n", strerror(errno) );
		clean_exit( EXIT_FAILURE );
	}

	/* Create our lock file and fork the daemon process. */
	g_LockFile = open( LOCK_FILE_PATH, O_CREAT | O_EXCL | O_FSYNC, 0755 );

	if( g_LockFile < 0 )
	{
		LOG->Stdout( "Could not create lock file at %s: %s", LOCK_FILE_PATH, strerror(errno) );
		clean_exit( EXIT_FAILURE );
	}

	/* Write the PID to the lock file (text, for the control script) */
	char sPID[12];
	sprintf( sPID, "%i\n", getpid() );
	write( g_LockFile, sPID, sizeof(sPID) );

	/* Stop writing to stdout. We've daemonized and it'll get annoying. */
	fclose( stdout );
	fclose( stdin );
	fclose( stderr );
}

static void SetUpSignalHandlers()
{
	// prevent the server from stopping on unexpected disconnect
	sigignore( SIGPIPE );

	// we use SIGHUP as a sentinel to reload configuration.
	signal( SIGHUP, HandleSignal );

	// we intercept these signals with our cleaner version
	signal( SIGINT, HandleSignal );
	signal( SIGTERM, HandleSignal );
	signal( SIGSEGV, HandleSignal );
	signal( SIGABRT, HandleSignal );
}

int main( int argc, char **argv )
{
	daemonize();
	SetUpSignalHandlers();

	/* Everything past here runs only in the daemon. */

	g_pConfig = new Config;

	// load configuration
	if( !g_pConfig->Load(FileUtil::GetConfigFilePath()) )
	{
		LOG->Stdout( "Failed to load config from \"%s\"!", FileUtil::GetConfigFilePath()  );
		delete g_pConfig;
		return 1;
	}

	const char* const LOG_PATH = g_pConfig->Get( "LogPath" );

	LOG = new Logger( g_pConfig );
	LOG->Open( LOG_PATH );

	g_pServer = new ChatServer;
	g_pServer->SetConfig( g_pConfig );

	g_pServer->Start();

	// run the server until the end of time
	g_pServer->MainLoop();

	// we should never get here, but if we do, clean up.
	LOG->Stdout( "I'a! Cthulhu fhtagn!" );

	delete g_pServer;
	g_pServer = NULL;

	delete g_pConfig;
	g_pConfig = NULL;

	return 0;
}

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
