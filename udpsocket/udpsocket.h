#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <string>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// a UDP socket class that sports a cache of resolved hosts
// giving the class the ability to send messages to arbitrary
// targets and enhancing performance where dns lookups are required

class UdpSocket
{
	public:
		UdpSocket();
		~UdpSocket();
		
		bool initialize(int port=0);
		
		int send(const std::string& host, int port, 
			      unsigned char* msg, uint32_t length);
			  
		
		int receive(unsigned char* msg, uint32_t length);
		
		const int port() const;
		const int sockFd() const;
	
	private:
	
		int 		   m_sockFd;
		struct sockaddr_in m_localAddr;
		
		std::map<std::string, struct sockaddr_in> m_hostCache;
};

#endif
