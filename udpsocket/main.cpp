#include "udpsocket.h"

#include <iostream>

using namespace std;

int main( int argc, char* argv[])
{

	if ( argc != 3 )
	{
		cout << "usage:  " << argv[0] << " [localport] [remoteport]" <<
		endl;
		return 1;
	}
	
	int localport  = atoi( argv[1] );
	int remoteport = atoi( argv[2] );
	
	UdpSocket mySock;
	
	mySock.initialize( localport );
   
	string msg = "herro!";
   
	char msgrx[256];
   
	memset( msgrx, 0, sizeof(msgrx) );
	
	cout << "send returned " << mySock.send( "localhost", remoteport, (unsigned char*)msg.c_str(), msg.length() ) << endl;

	
	cout << "receive returned " << mySock.receive( (unsigned char*)msgrx, sizeof(msgrx) ) << endl;	
	cout << "received ";
	
	cout << msgrx << endl;
	
	cout << endl;
	
	return 0;
}
