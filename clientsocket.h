#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H


#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


using namespace std;


class clientsocket
{
public:
	clientsocket( string addr, int portno);
	~clientsocket();
	int send( const char *data);
	int send_32bit ( uint32_t data);
private:

	int sockfd;
	struct sockaddr_in serv_addr;

};

#endif