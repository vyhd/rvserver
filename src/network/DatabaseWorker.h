/* DatabaseWorker: handles network requests in a separate thread, using 
 * a queue and User pointers to manipulate . */

#include <string>
#include <queue>

#include "network/Socket.h"
#include "util/Thread.h"

class User;
struct Request;

class DatabaseWorker
{
	// We don't allow any direct access to these methods.
	// They must go through the Connector only.
	friend class DatabaseConnector;
protected:
	/* host must be an absolute URL. auth/config are relative to host. */
	DatabaseWorker( const std::string &host, const std::string &auth, const std::string &config );
	~DatabaseWorker();

	// kill the worker thread
	void Stop();

	bool IsConnected() const { return m_Socket.IsOpen(); }

	/* makes a new Request* and pushes it onto the queue. */
	void Login( User *user, const std::string &passwd );
	void SavePrefs( const User *user );

private:
	// shortcuts for handling the connection
	bool Connect();
	void Disconnect();

	// internal handlers for requests
	void DoLogin( Request *req );
	void DoSavePrefs( Request *req );

	/* gets configuration for this user. Called during Login. */
	void DoLoadPrefs( User *user );

	// workaround for member function and normal function thread
	static void *Start( void *p ) { ((DatabaseWorker*)p)->HandleRequests(); return NULL; }
	void HandleRequests();

	/* sends a request with URL-encoded parameters, returns response */
	std::string SendPOST( const std::string &url, const std::string &params );

	std::string m_sServer, m_sAuthPage, m_sConfigPage;
	std::queue<Request*> m_Requests;

	/* Only allow one system to access the queue at a time. */
	Spinlock m_QueueLock;

	/* true while the thread is still running. */
	bool m_bRunning;

	Socket m_Socket;
	Thread m_Thread;
};
