#ifndef MULTICAST_H
#define MULTICAST_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte

class Multicast
{
	public:
		Multicast();
		~Multicast();
		
		bool open( const std::string& groupAddr, int port );
		bool close();
		
		int read( unsigned char* buffer, int len );
		int write( const std::string& groupAddr, int port, 
                 unsigned char* buffer, int len );
		
	private:
		int m_socket;
		
		sockaddr_in  m_bindAddr;
		
		ip_mreq	     m_imreq;		
		
};


#endif
