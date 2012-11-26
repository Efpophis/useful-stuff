/*

File:             servsocket.h

Purpose:          Declares a server class that supports basic operations open,
                  close, read, write.

Author & Date:    Bill Crossley   17 Oct 2008

*/
#ifndef __SERVERSOCKET_H__
#define __SERVERSOCKET_H__

#include <stdio.h>	/* */
#include <stdlib.h>	/* exit() */
#include <string.h>	/* memset(), memcpy() */
#include <sys/utsname.h>	/* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(),
                             listen(), accept() */
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>	/* fork(), write(), close() */
#include <fcntl.h>
#include <errno.h>


#define DEFAULT_PORT (7777)

class ServerSocket
{
   public:
      ServerSocket(int port=DEFAULT_PORT);
      virtual ~ServerSocket();

      int  Open( int port );
      int  Open( void );
      int  Accept( bool doFork=false );
      void Close( void );
      int  Read( unsigned char *buf, int len );
      int  ReadFully( unsigned char* buf, int len );
      int  Write( unsigned char *buf, int len );
      int  Write( char * );
      int  SockFD(void){ return (m_slaveSocket == -1) ? m_serverSocket : m_slaveSocket; }

      int  SetBlocking( int on );

      // returns old size or -1 on error.
      int  SetTxBufSize( int newSize );

      void Attach( int sockFd );

      int  SetFileOption( int option, int on );
      int  GetPort( void ) { return m_port; }
      const char* GetRemoteHostAddr( void ) { return m_connectedHost == NULL ? "0.0.0.0" : (const char*) m_connectedHost; }
      void SetRemoteHost( const char* theHost );

   protected:
      volatile int m_serverSocket,
                   m_port,
                   m_status,
                   m_blocking;
                   

      volatile int m_slaveSocket;
      socklen_t m_clientLength;

      const int m_on;
      const int m_off;

      struct hostent *m_hostPtr;

      char m_hostname[80];

      struct sockaddr_in m_serverName;

      struct linger m_linger;

      struct sockaddr_in m_clientName;

      const int BACK_LOG;

      char *m_connectedHost;

      int GetHostName(char *buffer, int length);
      static void BrokenPipe( int signum );

};



#endif
