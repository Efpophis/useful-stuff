//
//
// File:          servsocket.cpp
//
// Purpose:       implements a socket class with a server bent.
//
// Author & Date: Bill Crossley
//
//


#include <fcntl.h>
#include <signal.h>
#include <iostream>
#include "servsocket.h"

using namespace std;

//*****************************************************************************
// Function:         ServerSocket::ServerSocket(int port)
//
// Responsibility:   initialize members.
//
// Input Params:     int port - the port on which you want this socket to listen
//                              for connections.
//
// Output Params:    None.
//
// Return Value:     None.
//
// Cautions:         None.
//*****************************************************************************
ServerSocket::ServerSocket(int port) : m_serverSocket(-1),m_port(port),m_status(0),
									m_blocking(0), m_slaveSocket(-1), m_clientLength(0),
                                   m_on(1), m_off(0),m_hostPtr(NULL), BACK_LOG(5),
                                   m_connectedHost(NULL)
{
   memset(m_hostname, 0x00, sizeof(m_hostname));
   memset(&m_serverName, 0x00, sizeof(m_serverName));
   memset(&m_linger, 0x00, sizeof(m_linger));
   memset(&m_clientName, 0x00, sizeof(m_clientName));
   signal(SIGPIPE, BrokenPipe);
}


//*****************************************************************************
// Function:         ServerSocket::~ServerSocket()
//
// Responsibility:   Destructor - close all open sockets.
//
// Input Params:     None.
//
// Output Params:    None.
//
// Return Value:     None.
//
// Cautions:         None.
//****************************************************************************
ServerSocket::~ServerSocket()
{
   Close();

   if( m_connectedHost != NULL )
   {
      delete[] m_connectedHost;
      m_connectedHost = NULL;
   }
}


//*****************************************************************************
// Function:         int ServerSocket::Open(int port)
//
// Responsibility:   Listens for a connection on the port specified.
//
// Input Params:     int port - the TCP port on which to listen for connections.
//
// Output Params:    None.
//
// Return Value:     returns the descriptor of the open and listening socket.
//
// Cautions:         Does not do the accept.
//****************************************************************************
int ServerSocket::Open(int port)
{
   m_port = port;

   return Open();
}

void ServerSocket::SetRemoteHost( const char* theHost )
{
   if( m_connectedHost != NULL )
   {
      delete[] m_connectedHost;
   }
   m_connectedHost = strdup( theHost );
}


//*****************************************************************************
// Function:         int ServerSocket::Open(void)
//
// Responsibility:   Creates a socket, and binds it to the port in m_port, and
//                   puts the socket into a listening state.
//
// Input Params:     None.
//
// Output Params:    None.
//
// Return Value:     returns the descriptor of the listening socket.
//
// Cautions:         None.
//****************************************************************************
int ServerSocket::Open(void)
{
   if ((m_serverSocket != -1) || (m_slaveSocket != -1))
   {
      perror("socket already open()");
      return -1;
   }

   // create the socket.
   m_serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if (m_serverSocket == -1)
   {
      perror("socket()");
      return -1;
   }

   // turn off bind address checking, and allow
   // port numbers to be reused - otherwise
   // the TIME_WAIT phenomenon will prevent
   // binding to these address.port combinations
   // for (2 * MSL) seconds.
   m_status = setsockopt(m_serverSocket, getprotobyname("TCP")->p_proto,
                           TCP_NODELAY, (const char*) &m_on, sizeof(m_on));
   if (m_status == -1)
   {
      perror("setsockopt(...,TCP_NODELAY,...)");
      return -1;
   }

   m_status = setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR,
                          (const char *) &m_on, sizeof(m_on));
   if (m_status == -1)
   {
      perror("setsockopt(...,SO_REUSEADDR,...)");
      return -1;
   }

   m_status = setsockopt(m_serverSocket, SOL_SOCKET, SO_KEEPALIVE,
                          (const char *) &m_on, sizeof(m_on));
   if (m_status == -1)
   {
      perror("setsockopt(...,SO_KEEPALIVE,...)");
      return -1;
   }

   // set linger for 1 second
   m_linger.l_onoff = m_on;
   m_linger.l_linger = 1;
   m_status = setsockopt(m_serverSocket, SOL_SOCKET, SO_LINGER,
                          (const char *) &m_linger, sizeof(m_linger));
   if (m_status == -1)
   {
      perror("setsockopt(...,SO_LINGER,...)");
      return -1;
   }

   m_status = SetFileOption(O_DSYNC, m_on);

   if (m_status == -1)
   {
      perror("SetFileOption(O_DSYNC)");
      return -1;
   }

   // find out who I am
   m_status = GetHostName(m_hostname, sizeof(m_hostname));
   if (m_status == -1)
   {
      perror("GetHostName()");
      return -1;
   }

   m_hostPtr = gethostbyname(m_hostname);

   if (m_hostPtr == NULL)
   {
      perror("gethostbyname()");
      return -1;
   }

   memset(&m_serverName, 0x00, sizeof(m_serverName));
   memcpy(&m_serverName.sin_addr, m_hostPtr->h_addr, m_hostPtr->h_length);


   // allow server be contactable on any of
   // its IP addresses
   m_serverName.sin_addr.s_addr=htonl(INADDR_ANY);

   m_serverName.sin_family = AF_INET;

   // network-order
   m_serverName.sin_port = htons(m_port);

   m_status = bind(m_serverSocket, (struct sockaddr *) &m_serverName,
                    sizeof(m_serverName));
   if (m_status == -1)
   {    
      perror("bind()");
      return -1;
   }
   else
   {
     // read the port back into our m_port member.
     // this allows us to do a dynamic binding (by passing zero as the
     // port number) and then know what port we got, which could be useful.
     socklen_t namelen = sizeof( m_serverName );

     m_status = getsockname( m_serverSocket, 
			     (struct sockaddr *) &m_serverName,
			     &namelen );
     if ( m_status != -1 )
     {
       m_port = ntohs(m_serverName.sin_port);
     }
   }

   m_status = listen(m_serverSocket, BACK_LOG);

   if (m_status == -1)
   {
      perror("listen()");

      close( m_serverSocket );
      m_serverSocket = -1;
   } 
   return m_serverSocket;
}

int ServerSocket::SetBlocking( int on )
{
	int result = SetFileOption( O_NONBLOCK, !on );
	
	if ( result != -1 )
	{
		m_blocking = on;
	}
	return result;
}

//*****************************************************************************
// Function:         int ServerSocket::Accept(bool doFork)
//
// Responsibility:   accepts a connection on our socket.
//
// Input Params:     bool doFork - whether or not the program should fork() once
//                                 a connection is accepted.
//
// Output Params:    None.
//
// Return Value:     returns the descriptor for the new connection if either
//                     a) doFork == false, or
//                     b) doFork == true
//                   and this is the child process.  Returns the descriptor
//                   for the old socket if doFork is true and this is the
//                   parent process.  Returns -1 on error.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::Accept(bool doFork)
{
   int retSocket;
   int pid;

   m_clientLength = sizeof(m_clientName);

   m_slaveSocket = accept(m_serverSocket, (struct sockaddr *) &m_clientName,
                          &m_clientLength);

   if (m_slaveSocket == -1)
   {
	   // hush the error if we're non-blocking and errno indicates
	   // that the fact that we'd block was the only problem.
	   if ( !((m_blocking == m_off) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) )
	   {
		   perror("ServerSocket::accept()");		   
	   }
	   return m_slaveSocket;
   }   

   if (doFork)
   {
      pid = fork();
      switch (pid)
      {
         case -1:
            perror("fork()");
            return pid;
         break;

         // child pid - close server socket and return slave socket.
         case 0:
            close(m_serverSocket);
            m_serverSocket = -1;
            retSocket = m_slaveSocket;

            if (getpeername(m_slaveSocket, (struct sockaddr *) &m_clientName,
		                      &m_clientLength) == -1)
            {
               perror("getpeername()");
               return -1;
            }
            else
            {
               cerr << "Connection request from "
				    << inet_ntoa(m_clientName.sin_addr)
				    << " on port " << m_port << endl;
            }
         break;

         default: // parent pid - close slave socket and return server socket.
            close(m_slaveSocket);
            m_slaveSocket = -1;
            retSocket = m_serverSocket;
         break;
      }

   }
   else
   {
      //close(m_serverSocket);
      //m_serverSocket = -1;

      retSocket = m_slaveSocket;

      if (getpeername(m_slaveSocket, (struct sockaddr *) &m_clientName,
                      &m_clientLength) == -1)
      {
         perror("getpeername()");
         return -1;
      }
      else
      {

         cerr << "Connection request from "
              << inet_ntoa(m_clientName.sin_addr)
              << " on port " << m_port << endl;

         SetRemoteHost(inet_ntoa( m_clientName.sin_addr ));
      }
   }

   return retSocket;
}


//*****************************************************************************
// Function:         int ServerSocket::ReadFully(unsigned char* buf, int len)
//
// Responsibility:   Read len bytes from the socket.  Return only upon reading
//                   specified number of bytes or an error occurs.
//
// Input Params:     unsigned char* buf - pointer to a buffer in which to
//                                        place the bytes we read.  Must
//                                        be len bytes long.
//
//                   int len - the length of the buffer pointed to by buf.
//
// Output Params:    Data read is written to buf.
//
// Return Value:     returns number of bytes actually read or -1 on error.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::ReadFully(unsigned char* buf, int len)
{
   int bytesRead = 0;
   int total     = 0;

   while (total < len)
   {
      bytesRead = Read(buf, len - total);

      if ( bytesRead == 0 )
      {
          return 0;
      }
      if (bytesRead < 0)
      {
         return -1;
      }
      else
      {
         total += bytesRead;
         buf   += bytesRead;
      }
   }
   return total;
}


//*****************************************************************************
// Function:         int ServerSocket::Read(unsigned char* buf, int len)
//
// Responsibility:   read len bytes from the socket if available.
//
// Input Params:     unsigned char* buf - pointer to a buffer in which to
//                                        place the bytes we read.  Must be
//                                        len bytes long.
//
//                   int len - the length of the buffer pointed to by buf.
//
// Output Params:    Data read is written to buf.
//
// Return Value:     returns number of bytes actually read or -1 on error.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::Read(unsigned char* buf, int len)
{
   int bytesRead = -1;

   if (m_slaveSocket != -1)
   {
      bytesRead = read(m_slaveSocket, buf, len);

      if (bytesRead < 0)
      {	
         bytesRead = -1;
      }
      else if (bytesRead == 0)
      {
         bytesRead = -1;
         // this means connection closed by foreign host - what do do here? 
      }
   }
   return bytesRead;
}


//*****************************************************************************
// Function:         int ServerSocket::Write(char* str)
//
// Responsibility:   write out a nul-terminated string.
//
// Input Params:     char* str - null terminated string to send.
//
// Output Params:    None.
//
// Return Value:     returns number of bytes written or -1 on error.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::Write(char *str)
{
   return Write((unsigned char*)str, (int)strlen(str));
}


//*****************************************************************************
// Function:         int ServerSocket::Write(unsigned char* buf, int len)
//
// Responsibility:   send len bytes from buf out the socket.
//
// Input Params:     unsigned char* buf - pointer to a message to send out.
//
//                   int len - the number of bytes to send.
//
// Output Params:    None.
//
// Return Value:     returns number of bytes written or -1 on error.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::Write(unsigned char *buf, int len)
{
   int result = 0;
   int bytessent = 0;
   int oldSize;

   if (m_slaveSocket != -1)
   {
      oldSize = SetTxBufSize(len);

      while (bytessent < len)
      {
         result = write(m_slaveSocket, buf, len - bytessent);

         if (result == -1)
         {
            perror("write()");
            return result;
         }
         else
         {
            // update number of bytes sent
            bytessent += result;

            // advance buffer pointer to next unsent byte
            buf += result;
         }
      }

      if (oldSize != -1)
      {
         SetTxBufSize(oldSize);
      }
   }
   return bytessent;
}


//*****************************************************************************
// Function:         void ServerSocket::Close(void)
//
// Responsibility:   close any currently open sockets.
//
// Input Params:     None.
//
// Output Params:    None.
//
// Return Value:     None.
//
// Cautions:         None.
//*****************************************************************************
void ServerSocket::Close(void)
{
   if (m_serverSocket != -1)
   {
      close(m_serverSocket);
      m_serverSocket = -1;
   }
   if (m_slaveSocket != -1)
   {
      close(m_slaveSocket);
      m_slaveSocket = -1;
   }
}


//*****************************************************************************
// Function:         int ServerSocket::GetHostName(char* buffer, int length)
//
// Responsibility:   replacement of unix system call gethostname() to aid
//                   portability.
//
// Input Params:     char* buffer - pointer to a place to put the host name.
//
//                   int length - max number of characters to put into buffer.
//
// Output Params:    buffer is loaded with the host name.
//
// Return Value:     returns 0 on success or -1 on failure.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::GetHostName(char *buffer, int length)
{
    struct utsname sysname;

    memset(&sysname, 0x00, sizeof(sysname));

    m_status = uname(&sysname);

    if (m_status != -1)
    {
        strncpy(buffer, sysname.nodename, length);
    }

    return (m_status);
}


//*****************************************************************************
// Function:         int ServerSocket::SetFileOption(int option, int on)
//
// Responsibility:   changes the file option.
//
// Input Params:     int option - option value.
//
//                   int on - option setting.
//
// Output Params:    none.
//
// Return Value:     returns 0 on success or -1 on failure.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::SetFileOption(int option, int on)
{
   int result = -1;
   int flags;
   int socket;

   if (m_slaveSocket == -1)
   {
      socket = m_serverSocket;
   }
   else
   {
      socket = m_slaveSocket;
   }

   if (socket != -1)
   {
      flags = fcntl(socket, F_GETFL, 0);

      if (flags == -1)
      {
          flags = 0;
      }

      if (on)
      {
         flags |= option;
      }
      else
      {
         flags &= ~option;
      }

      result = fcntl(socket, F_SETFL, flags);
   }
   return result;
}


//*****************************************************************************
// Function:         int ServerSocket::SetTxBufSize(int newSize)
//
// Responsibility:   Changes the transmit buffer size to match the size of
//                   data to be transmitted.
//
// Input Params:     int newSize - size of data to be transmitted.
//
// Output Params:    none.
//
// Return Value:     returns 0 on success or -1 on failure.
//
// Cautions:         None.
//*****************************************************************************
int ServerSocket::SetTxBufSize(int newSize)
{
   int oldSize = -1;
   int result;
   socklen_t optLen = sizeof(oldSize);

   result = getsockopt(m_slaveSocket, SOL_SOCKET, SO_SNDBUF, &oldSize,
                        &optLen);

   if (result != -1)
   {
      result = setsockopt(m_slaveSocket, SOL_SOCKET, SO_SNDBUF, &newSize,
                           sizeof(newSize));

      if (result != -1)
      {
         result = oldSize;
      }
      else
      {
         perror("setsockopt(...SO_SNDBUF...)");
      }
   }
   else
   {
     perror("getsockopt(...SO_SNDBUF...)");
   }

   return result;
}

//*****************************************************************************
// Function:         int ServerSocket::BrokenPipe(int signum)
//
// Responsibility:
//
// Input Params:
//
// Output Params:
//
// Return Value:
//
// Cautions:
//*****************************************************************************
void ServerSocket::BrokenPipe(int /*signum*/)
{
   // just handle it and let the system call return an error.
   // that should work.  
   signal(SIGPIPE, BrokenPipe);
}

void ServerSocket::Attach( int sockFd )
{
    m_slaveSocket = sockFd;
}
