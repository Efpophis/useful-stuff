/*

File:             clisocket.h

Purpose:          declares a client socket class that can initiate tcp socket
                  connections.

Author & Date:    Bill Crossley   17 Oct 2008

*/
#include "servsocket.h"

#ifndef __CLISOCKET_H__
#define __CLISOCKET_H__

#include <stdio.h>                /* perror() */
#include <stdlib.h>               /* atoi() */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>               /* read() */
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// deriving from ServerSocket allows us to provide a new open and close, and
// re-use a good bit of the server code (read, write, etc.)
class ClientSocket : public ServerSocket
{
   public:
      ClientSocket();

      // these can do nothing.
      int Open(int) {return -1;}
      int Open(void )     {return -1;}
      int  Accept(bool doFork=false) {if ( doFork ); return -1;}

      // we'll overload these two, and use the rest from the server.
      int Open(char *remoteHost, int remotePort, int block = 1);
      void Close();

      void SetConnTimeout( int timeOut ) { m_connTimeout = timeOut; }

      // SockFD can be simplified, so overload that, too.
      int  SockFD(void) {return m_slaveSocket;}

   private: 
      void EventAfter( int sec );
      void EventCancel( void );
      static void TimeOut( int sig );

      static bool m_timeout;
      int m_connTimeout;

};


#endif
