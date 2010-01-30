/* Wrapper class for a socket and associated operations. This way, we can
 * abstract network operations and keep low-level stuff out of other files.
 */
#ifndef SOCKET_H
#define SOCKET_H

#include <string>

class Socket
{
public:
	Socket() : m_iSocket(-1) { }
	Socket( int socket ) : m_iSocket(socket) { }

	const char* GetIP() const;

	// name difference is needed: same arg types, can't overload
	bool OpenHost( const std::string &host, int port );
	bool Open( const std::string &ip, int port );
	void Close();

	bool IsOpen() const	{ return m_iSocket > 0; }

	int Read( char *buffer, unsigned len, bool bDontWait = true );
	int Write( const char *buffer, unsigned len, bool bDontWait = true );
	int Write( const std::string &str, bool bDontWait = true );

private:
	int m_iSocket;
};

#endif // SOCKET_H
