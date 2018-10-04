#include "assertreport.h"

#include <Winsock2.h>
#include <Ws2tcpip.h>

#include <stdlib.h>	// for rand()
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "logx.h"


static int serversock=0;
static struct sockaddr_in server_addr;


static int assertreport_create_socket( void )
{
	int bufsz=16*1024;
	struct linger lngr;
	lngr.l_onoff  = 1;
	lngr.l_linger = 2;

	serversock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( serversock<0 )
	{
		const char* s = strerror(WSAGetLastError());
		LOGE( "socket() failed: %s", s );
		return 0;
	}
	return 1;
}


int assertreport_send( const char* msg, int len )
{
	int addrlen = sizeof(server_addr);
	int rv = sendto( serversock, msg, len, 0, (struct sockaddr*) &server_addr, addrlen );
	if ( rv < 0 ) perror( "sendto" );
	return rv;
}


int assertreport_init( const char* host, int portnr )
{
	int created = assertreport_create_socket();
	if ( !created )
	{
		LOGE( "Failed to create socket." );
		return 0;
	}

	memset( &server_addr, 0, sizeof(server_addr) );

	struct hostent *he = gethostbyname( host );
	if ( !he ) { perror("gethostbyname"); return 0; }
	struct in_addr ip_addr = *(struct in_addr *)(he->h_addr_list[0]);
	char *ipnr = inet_ntoa( ip_addr );

	server_addr.sin_family	= AF_INET;
	server_addr.sin_port	= htons( portnr );
	int retval = inet_pton( AF_INET, ipnr, &server_addr.sin_addr );
	if ( !retval ) { perror( "inet_aton() failed" ); return 0; }

	return 1;
}


int assertreport_exit( void )
{
	int rv = closesocket( serversock );
	if ( rv ) perror( "close( sock )" );
	serversock = 0;
	return rv == 0;
}

#if defined(MAIN)
int main( int argc, char* argv[] )
{
	if ( argc != 2 ) { fprintf(stderr, "Usage: %s hostname", argv[0]); }
	assertreport_init( argv[1], 2323 );
	assertreport_send( "HELLO", 6 );
	assertreport_exit();
}
#endif

