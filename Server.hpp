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
#include "Channel.hpp"
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
		std::map<int, Client*>					client_map;
		std::map<std::string, Channel*> 		channel_map;
	public:
		Server(){};
		Server(int port, std::string pass);
		std::string	getPass() {return pass;};
		void		client_dc(int sd, int i);
		void 		run();
		void		parse_commands(Client *client, char *buffer, int valread, int i);
		std::map<std::string, Channel*>	getChannelMap() { return channel_map; };




		/***************************** COMMANDS *****************************/
		void	userCmd();
		void	pingCmd();
		void	joinCmd(Client *client, std::vector<std::string> splitted)
		{
			RepliesCreator reply;
			std::string msg;
			if (splitted[1][0] != '#')
			{
				if (splitted[1][splitted[1].length() - 1] == '\n')
					splitted[1].resize(splitted[1].length() - 2);
				msg.append(reply.makeErrorNoSuchChannel(client->getNick(), splitted[1]));
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else
			{
				Channel *new_channel = new Channel(splitted[1], client);
				std::cout << "benvenuto.\n";
				client->server->getChannelMap().insert(std::make_pair(splitted[1], new_channel));
				//std::cout << client->server->getChannelMap().find(splitted[1])->first << std::endl;
			}
		}
		void	nickCmd();
		void	quitCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string msg;
			if (splitted.size() > 1)
			{
				for (int i = 1; i < splitted.size(); i++)
					msg.append(splitted[i]);
					msg.append (" ");
			}
			else
				msg.append(client->getNick());
			send(client->getSd(), (msg + "\n").c_str(), (size_t)msg.length() + 1, MSG_OOB);
			client->setLogged(false);
			close(client->getSd());
		}
		void	privmsgCmd();
		void	partCmd();
		void	listCmd();
		void	whoCmd();
		void	topicCmd();
		void	modeCmd();
		void	motdCmd();
		void	noticeCmd();
		void	kickCmd();
		void	inviteCmd();
		~Server();
};

int							parse_info(Client *new_client, char *buffer, int valread);
std::vector<std::string>	ft_split(std::string str, std::string token);