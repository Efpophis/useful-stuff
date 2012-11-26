#include "multicast.h"
#include <iostream>

#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte

using namespace std;

int main(int argc, char* argv[])
{
	Multicast mc;
	string msg = "hello";
   
	if ( argc == 2 )
		msg = argv[1];
	
	// use first avail port for sending
	mc.open( "226.0.0.1", 0 );
	
	// send it to our guy listening on 4096
	mc.write( "226.0.0.1", 4096, (unsigned char*)msg.c_str(), msg.length() );
		
	mc.close();
	
	return 0;
	
}
