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
		void	pingCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string msg;
			RepliesCreator reply;

			if (splitted.size() < 2)
				msg.append(reply.makeErrorNoOrigin(client->getNick()));
			else if (splitted.size() == 2)
			{
				if (splitted[1][splitted[1].length() - 1] == '\n')
					splitted[1].resize(splitted[1].length() - 2);
				msg.append("PONG " + splitted[1] + "\n");
			}
			else
			{
				if (splitted[2][splitted[2].length() - 1] == '\n')
					splitted[2].resize(splitted[2].length() - 2);
				msg.append("PONG " + splitted[2] + " " + splitted[1] + "\n");
			}
			send(client->getSd(), msg.c_str(), msg.length(), 0);
		};
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

		void	sendAll(std::vector<Client*> vec, std::string channel, Client* client)
		{
			std::string msg;

			for(int k = 0; k != vec.size(); k++)
			{
				msg.append(":" + vec[k]->getNick() + "!~" + vec[k]->getUser() + " JOIN :" + channel + DEL);
				send(vec[k]->getSd(), msg.c_str(), msg.length(), 0);
				msg.clear();
			}

			//:benjolo2!~benjo2@Azzurra-3476AEA0.business.telecomitalia.it JOIN :#woww
		}
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
					chan->insert(client);
					std::vector<Client*> vec = clientInMap(chan->getClientMap());
					sendAll(vec, splitted[1], client);
					for(int k = 0; k != vec.size(); k++)
					{
						if (vec[k]->getMod())
						{
							sent.append(":127.0.0.1 353 " + vec[k]->getNick() + " = " + splitted[1] + " :@" + vec[k]->getNick() + "\n");
							send(client->getSd(), sent.c_str(), sent.length(), 0);
							sent.clear();
						}
					}
					for(int k = 0; k != vec.size(); k++)
					{
						if (!vec[k]->getMod())
						{
							sent.append(":127.0.0.1 353 " + vec[k]->getNick() + " = " + splitted[1] + " :" + vec[k]->getNick() + "\n");
							send(client->getSd(), sent.c_str(), sent.length(), 0);
							sent.clear();
						}
					}
					sent.append(":127.0.0.1 366 " + client->getNick() + " " + splitted[1] + " :End of /NAMES list.\n331\n");
					send(client->getSd(), sent.c_str(), sent.length(), 0);
					std::cout << "benvenuto.\n";
				}
				else
				{
					Channel *new_channel = new Channel(splitted[1], client);
					client->setMod(true);
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
			//:benjolo2!~benjo2@Azzurra-3476AEA0.business.telecomitalia.it JOIN :#woww
		}
		void	nickCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string		msg;
			RepliesCreator	reply;

			if (splitted.size() == 1)
			{
				if (splitted[0][splitted[0].length() - 1] == '\n')
					splitted[0].resize(splitted[0].length() - 2);
				msg.append(reply.makeErrorNeedMoreParams(client->getNick(), splitted[0]));
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else
			{
				if (splitted[1][splitted[1].length() - 1] == '\n')
					splitted[1].resize(splitted[1].length() - 2);
				if (splitted[1].compare(client->getNick()))
				{
					msg.append(":" + client->getNick() + " ");
					client->setNick(splitted[1]);
					msg.append("NICK :" + splitted[1] + "\n");
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
			}
		}
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
		void	whoCmd(Client *client, std::vector<std::string>splitted)
		{
			std::string msg;
			RepliesCreator reply;
			if (splitted[1][splitted[1].length() - 1] == '\n')
				splitted[1].resize(splitted[1].length() - 2);
			Channel* chan = findChannel(splitted[1]);

			if(!chan)
				std::cout << "|" << splitted[1] << "|" << "\nao non funge" << std::endl;
			else
			{
				std::vector<Client*> vec = clientInMap(chan->getClientMap());
				for(int k = 0; k != vec.size(); k++)
				{
					msg = reply.makeWhoReply(vec[k]->getNick(), splitted[1]);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
					msg.clear();
				}
				msg = reply.makeEndofWhoreply(client->getNick(), splitted[1]);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
		}

		void	modeCmd(Client *client, std::vector<std::string>splitted)
		{
			std::string msg = "324 " + client->getNick() + "329" + DEL;
			send(client->getSd(), (msg + "\n").c_str(), (size_t)msg.length() + 1, MSG_OOB);
		}
		/*
		void	privmsgCmd(Client *client, std::vector<std::string> splitted) //da finire
		{
			std::string					msg;
			RepliesCreator				reply; //forse dovremmo levare replies creator e scrivere le cose direttamente??
			std::vector<std::string>	nicks; //se ci sono le virgole in splitted[1], ovvero una lista di utenti, si dovrebbe splittare per ',' e mandare il messaggio a tutti gli utenti della lista

			if (splitted.size() == 1) // se c'è solo PRIVMSG
			{
				if (splitted[0][splitted[0].length() - 1] == '\n')
					splitted[0].resize(splitted[0].length() - 2); //dovremmo provare a farlo direttamente senza if
				msg.append("411 " + client->getNick() + " :No recipient given (PRIVMSG)\n");
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else //prima devo controllare che il nickname (o tutti quelli in lista? da checkare) esista, poi subito dopo controllo se c'è un messaggio (splitted[2]...)
			{
				nicks = ft_split(splitted[1], ","); //(splitto per ',' per ottenere la lista di utenti e canali a cui mandare il messaggio (devo controllare se accetta gli spazi ma mi pare di no))
				if (splitted[1][splitted[1].length() - 1] == '\n')
					splitted[1].resize(splitted[0].length() - 2);
				for (std::map<int, Client*>::iterator it; it = client_map.begin(); it++)
				{
					if (!it->second->getNick().compare(splitted[1]))
					{
						break ;
					}
					//se il messaggio comincia con ':', bisogna concatenare tutt gli splitted successivi al 2 (e potrebbe essere un problema perché noi splittiamo per gli spazi, e non so se vengono trimmati)
					//altrimenti, viene considerato solo splitted[2]
				}
			}
			
		}
		*/
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