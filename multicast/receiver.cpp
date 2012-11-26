#include <iostream>
#include "multicast.h"

using namespace std;

#define MAXBUFSIZE 65536 // Max UDP Packet size is 64 Kbyte

int main()
{
	Multicast mc;
	char msg[MAXBUFSIZE];
	
	mc.open( "226.0.0.1", 4096 );
	
	while ( strcmp( msg, "quit" ) )
	{
		memset( msg, 0, MAXBUFSIZE );
		mc.read( (unsigned char*)msg, MAXBUFSIZE );
	
		cout << msg << endl;
	}
	
	mc.close();
	
	return 0;
	
}
