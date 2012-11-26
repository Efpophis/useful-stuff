//
//
// File:             clisocket.cpp
//
// Purpose:          implements a socket class for client applications.
//
// Author & Date:    Bill Crossley   17 Oct 2008
//
//
#include <signal.h>
#include "clisocket.h"

bool ClientSocket::m_timeout = false;

ClientSocket::ClientSocket() : m_connTimeout(2)
{
}

//*****************************************************************************
// Function:         void ClientSocket::Open(char* remoteHost, int remotePort)
//
// Responsibility:   opens a socket connection to the specified port on the
//                   remote host.
//
// Input Params:     char* remoteHost - the remote host to connect to.
//
//                   int remotePort - the port to use to connect.
//
// Output Params:    None.
//
// Return Value:     None.
//
// Cautions:         None.
//*****************************************************************************
int ClientSocket::Open(char *remoteHost, int remotePort, int block /* = 1 */)
{
	// use m_slaveSocket part of the server class, cuz
	// that's the one that is used for read / write.
	m_slaveSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_slaveSocket == -1)
	{
		perror("socket()");
		return -1;
	}

	// set linger for 1 second
	m_linger.l_onoff = m_on;
	m_linger.l_linger = 1;
	m_status = setsockopt(m_slaveSocket, SOL_SOCKET, SO_LINGER,
						  (const char *) &m_linger, sizeof(m_linger));
	if (m_status == -1)
	{
		perror("setsockopt(...,SO_LINGER,...)");
		return -1;
	}

	// turn on keep alive.
	m_status = setsockopt(m_slaveSocket, SOL_SOCKET, SO_KEEPALIVE,
                          (const char *) &m_on, sizeof(m_on));
    if (m_status == -1)
    {
		perror("setsockopt(...,SO_KEEPALIVE,...)");
		return -1;
	}

	m_status = SetFileOption(O_DSYNC, m_on);

	if (m_status == -1)
	{
		perror("SetFileOption(O_DSYNC)");
		return -1;
	}

	// turn of TCP delay
	m_status = setsockopt(m_slaveSocket, getprotobyname("TCP")->p_proto,
                           TCP_NODELAY, (const char*) &m_on, sizeof(m_on));
    if (m_status == -1)
    {
		perror("setsockopt(...,TCP_NODELAY,...)");
		return -1;
	}

	// need to resolve the remote server name or IP address
	m_hostPtr = gethostbyname(remoteHost);

	if (m_hostPtr == NULL)
	{
		unsigned long addr = inet_addr(remoteHost);

		if ((long)addr != -1)
		{
			m_hostPtr = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
		}

		if (m_hostPtr == NULL)
		{
			perror("Error resolving server address");
			return -1;
		}
	}

	m_serverName.sin_family = AF_INET;
	m_serverName.sin_port = htons(remotePort);

	memcpy(&m_serverName.sin_addr, m_hostPtr->h_addr, m_hostPtr->h_length);

	SetBlocking(block);

	EventAfter( m_connTimeout );

	m_status = connect( m_slaveSocket, (struct sockaddr*) &m_serverName,
					    sizeof(m_serverName));
   

	EventCancel();

	if (m_status == -1)
	{
		perror("connect()");
		if (block)
		{
			return -1;
		}
		else
		{
			switch (errno)
			{
				case EALREADY:
				case EINPROGRESS:
				// ignore these errors - they mean the socket will be connected
				// eventually.
					SetRemoteHost((const char*) remoteHost );
				break;

				default:
					close(m_slaveSocket);
					m_slaveSocket = -1;
					return -1;
				break;
			}
		}
	}
	else
	{
		SetRemoteHost((const char*) remoteHost );
		m_port = remotePort;
	}
	return m_slaveSocket;
}


//*****************************************************************************
// Function:         void ClientSocket::Close()
//
// Responsibility:   close the active connection.
//
// Input Params:     None.
//
// Output Params:    None.
//
// Return Value:     None.
//
// Cautions:         None.
//*****************************************************************************
void ClientSocket::Close()
{
	shutdown(m_slaveSocket, 2);
	close(m_slaveSocket);
	m_slaveSocket = -1;
	if( m_connectedHost != NULL )
	{
		delete[] m_connectedHost;
		m_connectedHost = NULL;
	}
}

void ClientSocket::EventAfter( int sec )
{
	// initialize alarm handler
	signal(SIGALRM, TimeOut);

	// reset timeout flag
	m_timeout = false;

	// set alarm for sec seconds.
	alarm(sec); 
}

void ClientSocket::EventCancel( void )
{
	// cancel alarm
	alarm(0);
}

void ClientSocket::TimeOut( int /*sig*/ )
{
	// set timeout flag
	m_timeout = true;
}
