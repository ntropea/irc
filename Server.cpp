#include "Server.hpp"

Server::Server(int port, std::string pass)
{
	this->port = port;
	this->pass = pass;
	memset( &timeout, 0, sizeof(timeout) );
    timeout.tv_sec = 3 * 60;
	opt = 1;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	memset(&read_set, 0, sizeof(read_set));
	memset(&write_set, 0, sizeof(write_set));
	for (int i = 0; i < MAX_CLIENTS; i++)
		clients_sd[i] = 0;
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

void	Server::run()
{
	while (1)
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_SET(sock, &read_set);
		FD_SET(sock, &write_set);
		max_sd = sock;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			sd = clients_sd[i];
			if (sd > 0)
			{
				FD_SET(sd, &read_set);
				FD_SET(sd, &write_set);
			}
			if (sd > max_sd)
				max_sd = sd;
		}
		activity = select(max_sd + 1, &read_set, &write_set, NULL, &timeout);
	    std::cout << "activity is " << activity << std::endl;
        if ((activity < 0) && (errno!=EINTR)){
            std::cout << "activity is " << activity << std::endl;
            printf("select error\n");}
	}
}

Server::~Server(){};