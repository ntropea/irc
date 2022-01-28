#include "Server.hpp"

Server::Server(int port, std::string pass)
{
	this->port = port;
	this->pass = pass;
	if ((this->sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	memset(&addr, 0, sizeof(addr));
	this->addr.sin_family = AF_INET;
	this->addr.sin_addr.s_addr = INADDR_ANY;
	this->addr.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	if (listen(sock, 6) == -1)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
};

Server::~Server(){};