#include "clientsocket.h"


clientsocket::clientsocket( string addr, int portno)
{
	int err;

	this->sockfd = socket( AF_INET, SOCK_STREAM, 0);
	if ( this->sockfd == -1)
	{
		// failed to create new socket
		std::cout << "Error: socket() failed, " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	memset( &this->serv_addr, 0, sizeof(this->serv_addr));
	this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_port = htons(portno);
	inet_pton(AF_INET, addr.c_str(), &this->serv_addr.sin_addr);

	err = connect( sockfd, (struct sockaddr *)&this->serv_addr, sizeof(serv_addr));
	if ( err == -1)
	{
		std::cout << "Error: connect() failed, " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}



clientsocket::~clientsocket()
{
	close(this->sockfd);
}


int clientsocket::send( const char *data)
{
	int n, pos = 0;
	unsigned int len = strlen(data);
	
	while( (n = write( this->sockfd, &data[pos], len)) > 0)
	{
		pos += n; len -= n;
	}

	if ( n < 0)
	{
		// an error occurred
		return -1;
	}

	return 0;
}


int clientsocket::send_32bit( uint32_t data)
{
	write( this->sockfd, &data, sizeof(data));
}