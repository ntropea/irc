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
#include "Client.hpp"
#include "RepliesCreator.hpp"
//#include "Channel.hpp"

#define MAX_CLIENTS 30

class	Client;

class	Channel;

class	Server {
	private:
		int										port;
		std::string								pass;
		int										sock;
		struct sockaddr_in						addr;
		int										addrlen;
		int										opt;
		fd_set									read_set;
		fd_set									write_set;
		int										clients_sd[MAX_CLIENTS];
		int										max_sd;
		int										sd;
		int										activity;
		struct timeval 							timeout;
		int										new_sd;
		std::map<int, Client*>					map;
		std::map<int, Client*>::iterator		it;
		std::map<std::string, Channel*> 		channel_map;

	public:
		Server(){};
		Server(int port, std::string pass);
		std::string	getPass() {return pass;};
		void		client_dc(int sd, int i);
		void		join(Client *client, std::vector<std::string> splitted);
		void 		run();
		void		parse_commands(Client *client, char *buffer, int valread, int i);
		~Server();
};

int							parse_info(Client *new_client, char *buffer, int valread);
std::vector<std::string>	ft_split(std::string str, std::string token);