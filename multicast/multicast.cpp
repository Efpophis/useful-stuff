#include "multicast.h"

Multicast::Multicast()
{
	memset(&m_bindAddr, 0, sizeof(m_bindAddr));
	memset(&m_imreq, 0, sizeof(m_imreq));
}

Multicast::~Multicast()
{
	close();
}


bool Multicast::open( const std::string& groupAddr, int rxPort )
{
	m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_socket < 0)
	{
		perror("socket()");
		exit(1);
	}
	
	m_bindAddr.sin_family 	   = PF_INET;
	m_bindAddr.sin_port   	   = htons(rxPort);
	m_bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int status = bind(m_socket, (struct sockaddr*)&m_bindAddr,
			  sizeof(m_bindAddr));
			  
	if ( status < 0 )
	{
		perror("bind()");
		exit(1);
	}
	
	m_imreq.imr_multiaddr.s_addr = inet_addr( groupAddr.c_str() );
	m_imreq.imr_interface.s_addr = INADDR_ANY;
	
	status = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			    (const void*)&m_imreq, sizeof(m_imreq));
             
	if ( status < 0 )
	{
		perror("setsockopt()");
		exit(1);
	}	
			
	
	struct in_addr iaddr;
	
	iaddr.s_addr = INADDR_ANY;
	
	setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
			sizeof(iaddr));
	
	unsigned char ttl = 3;
	setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl,
			sizeof(ttl));
			
	unsigned char loop = 1; // loopback is on
	setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
			sizeof(loop));
				
	return true;		
}

bool Multicast::close()
{
	shutdown(m_socket,2);

	::close(m_socket);
	
	return true;
}

int Multicast::read( unsigned char* buffer, int len )
{
	sockaddr_in peer;
	socklen_t socklen = sizeof(peer);
	peer.sin_family = PF_INET;
	peer.sin_port   = m_bindAddr.sin_port;
	peer.sin_addr.s_addr = m_bindAddr.sin_addr.s_addr;
	
	int status = recvfrom( m_socket, buffer, len, 0,
			                 (struct sockaddr*)&peer, &socklen );
			      
	return status;
}


int Multicast::write( const std::string& groupAddr, int port, 
                      unsigned char* buffer, int len )
{
	sockaddr_in destAddr;
  	
	destAddr.sin_family = PF_INET;
	destAddr.sin_addr.s_addr = inet_addr( groupAddr.c_str() );
	destAddr.sin_port = htons( port );

   
	int status = sendto(m_socket, buffer, len, 0,
			              (struct sockaddr*)&destAddr, sizeof(destAddr));
	
	return status;
}
