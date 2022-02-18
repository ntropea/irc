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
#define IP "127.0.0.1"
#define RPL_NAMREPLY			"353"
#define RPL_ENDOFNAME			"366"
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
		Channel* findChannel(std::string nameChannel)
		{
			for(std::map<std::string, Channel*>::iterator i = channel_map.begin(); i != channel_map.end(); i++)
			{
				if (!i->first.compare(nameChannel))
					return(i->second);
			}
			return(NULL);
		};
		std::vector<Client*> clientInMap(std::map<int, Client*> map)
		{
			std::vector<Client*> vec;
			for(std::map<int, Client*>::iterator i = map.begin(); i != map.end(); i++)
			{
				vec.push_back(i->second);
			}
			return vec;
		};
		void	joinCmd(Client *client, std::vector<std::string> splitted)
		{
			RepliesCreator reply;
			std::string msg;
			if (splitted[1][splitted[1].length() - 1] == '\n')
				splitted[1].resize(splitted[1].length() - 2);
			if (splitted[1][0] != '#')
			{
				msg.append(reply.makeErrorNoSuchChannel(client->getNick(), splitted[1]));
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else
			{
				Channel* chan = findChannel(splitted[1]);
				if(chan != NULL)
				{
					std::string sent;
					sent.append(":" + client->getNick() + "!" + client->getUser() + "@127.0.0.1 " + splitted[0] + " " + splitted[1] + "\n");
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					sent.clear();					
					chan->getClientMap().insert(std::make_pair(client->getSd(), client));
					std::vector<Client*> vec = clientInMap(chan->getModMap());
					for(int k = 0; k != vec.size(); k++)
					{
						sent.append(":127.0.0.1 353 " + vec[k]->getNick() + " = " + splitted[1] + " :@" + vec[k]->getNick() + "\n");
						send(client->getSd(), sent.c_str(), sent.length(), 0);
						sent.clear();
					}
					std::vector<Client*> vec2 = clientInMap(chan->getClientMap());
					for(int k = 0; k != vec2.size(); k++)
					{
						sent.append(":127.0.0.1 353 " + vec2[k]->getNick() + " = " + splitted[1] + " :" + vec2[k]->getNick() + "\n");
						send(client->getSd(), sent.c_str(), sent.length(), 0);
						sent.clear();
					}
					sent.append(":127.0.0.1 366 " + client->getNick() + " " + splitted[1] + " :End of /NAMES list.\n331\n");
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					std::cout << "benvenuto.\n";
				}
				else
				{
					Channel *new_channel = new Channel(splitted[1], client);
					std::string sent;
					sent.append(":" + client->getNick() + "!" + client->getUser() + "@127.0.0.1 " + splitted[0] + " " + splitted[1] + "\n");
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					sent.clear();
					sent.append(":127.0.0.1 353 " + client->getNick() + " = " + splitted[1] + " :@" + client->getNick() + "\n");
					std::cout << "benvenuto.\n";
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					sent.clear();
					sent.append(":127.0.0.1 366 " + client->getNick() + " " + splitted[1] + " :End of /NAMES list.\n331\n");
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					channel_map.insert(std::make_pair(splitted[1], new_channel));
				}
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