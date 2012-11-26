#include <netdb.h>
#include <arpa/inet.h>
#include "udpsocket.h"


UdpSocket::UdpSocket()
:
 m_sockFd(-1)
{
	memset(&m_localAddr, 0x00, sizeof(m_localAddr));
}

UdpSocket::~UdpSocket()
{
	if (m_sockFd != -1)
		close(m_sockFd);
		
}


const int UdpSocket::port() const
{
	return (ntohs(m_localAddr.sin_port));
}


const int UdpSocket::sockFd() const
{
	return (m_sockFd);
}

// sets up the socket to receive any message on specified port
bool UdpSocket::initialize(int port)
{	
	bool result = false;
	
	m_localAddr.sin_family      = PF_INET;
	m_localAddr.sin_addr.s_addr = INADDR_ANY;
	m_localAddr.sin_port        = htons(port);
	
	int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (s < 0)
	{
		// oops
	}
	else if (bind( s, (struct sockaddr*)&m_localAddr, sizeof(m_localAddr)) < 0)
	{
		// oops
	}
	else
	{
		m_sockFd = s;
		result   = true;
	}
	
	return result;
}

int UdpSocket::receive(unsigned char* msg, uint32_t length)
{
	int result = -1;
   
	if (m_sockFd != -1)
	{
		struct sockaddr_in hostAddr;
		socklen_t hostAddrSize = sizeof(hostAddr);
		
		// this call blocks until something arrives
		result = recvfrom(m_sockFd, msg, length, 0, (struct sockaddr*)&hostAddr, &hostAddrSize);
	}

	return result; 
}

int UdpSocket::send(const std::string& host, int port, 
		              unsigned char* msg, uint32_t length)
{
	std::string theHost = host;
	
	if (m_hostCache.find(host) == m_hostCache.end())
	{
		// look it up and put it in
		struct sockaddr_in resolvedHost;
		struct hostent*    hostPtr = gethostbyname(host.c_str());
		
		if (hostPtr == NULL)
		{
			unsigned long addr = inet_addr(host.c_str());
			
			if ((long)addr != -1)
			{
				hostPtr = gethostbyaddr((char*)addr, sizeof(addr), AF_INET);
			}
			
			if (hostPtr == NULL)
			{
				return -1;
			}			
		}
		memcpy(&resolvedHost.sin_addr, hostPtr->h_addr, hostPtr->h_length);
		
		m_hostCache[host] = resolvedHost;
	}

	struct sockaddr_in target_host_addr = m_hostCache[host];
	
	target_host_addr.sin_family      = PF_INET;
	target_host_addr.sin_port        = htons(port);

		
	return sendto(m_sockFd, msg, length, 0, (struct sockaddr*)&target_host_addr, sizeof(struct sockaddr));
}
