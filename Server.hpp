#pragma once

#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <sstream>

#define MAX_CLIENTS 30

class	Server {
	private:
		int					port;
		std::string			pass;
		int					sock;
		struct sockaddr_in	addr;
		int					opt;
		fd_set				read_set;
		fd_set				write_set;
		int					clients_sd[MAX_CLIENTS];
		int					max_sd;
		int					sd;
		int					activity;
		struct timeval 		timeout;
	public:
		Server(){};
		Server(int port, std::string pass);
		//Server(Server &cpy);
		//Server &operator=(Server &cpy);
		void run();
		~Server();
};