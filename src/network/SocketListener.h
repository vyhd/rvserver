/* SocketListener: Listens for incoming connections and passes them to an
 * external handler (with the GetConnection() call) for processing. */

#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

class ChatServer;

class SocketListener
{
public:
	SocketListener();
	~SocketListener();

	/* Attempt to start listening on iPort, passing clients to pServer. */
	bool Connect( int iPort );
	void Disconnect();

	/* Returns a socket fd, or -1 if none is available (no new connections). */
	int GetConnection();

private:
	/* Server socket IDs */
	int m_iServerSocket;

	/* port we're listening on */
	int m_iPort;

};

#endif // SOCKETLISTENER_H
