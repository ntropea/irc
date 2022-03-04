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
# include <sys/time.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "RepliesCreator.hpp"
#define IP "127.0.0.1"
#define RPL_NAMREPLY			"353"
#define RPL_ENDOFNAME			"366"

#define MAX_CLIENTS 30

class	Client;

class	Channel;

std::vector<std::string>	ft_split(std::string str, std::string token);
int							parse_info(Client *new_client, char *buffer, int valread);

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
				vec.push_back(i->second);
			return vec;
		};
		void	sendAll(std::vector<Client*> vec, std::string msg, Client *client)
		{

			for(int k = 0; k != vec.size(); k++)
			{
				if(client->getSd() != vec[k]->getSd())
					send(vec[k]->getSd(), msg.c_str(), msg.length(), 0);
			}
		}

		/***************************** COMMANDS *****************************/
		void	userCmd();
		void	pingCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string msg;
			RepliesCreator reply;

			if (splitted.size() < 2)
				msg.append(reply.makeErrorNoOrigin(client->getNick()));
			else if (splitted.size() == 2)
				msg.append("PONG " + splitted[1] + "\n");
			else
				msg.append("PONG " + splitted[2] + " " + splitted[1] + "\n");
			send(client->getSd(), msg.c_str(), msg.length(), 0);
		};
		void	joinCmd(Client *client, std::vector<std::string> splitted)
		{
			RepliesCreator reply;
			std::string msg;
			std::vector<std::string> names;
			names = ft_split(splitted[1], ",");
			for (int i = 0; i < names.size(); i++)
			{
				if (names[i][0] != '#')
				{
					msg.append(reply.makeErrorNoSuchChannel(client->getNick(), names[i]));
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
				else
				{
					Channel* chan = findChannel(names[i]);
					if(chan != NULL)
					{
						msg.append(":" + client->getNick() + "!" + client->getUser() + "@127.0.0.1 " + splitted[0] + " " + names[i] + "\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						msg.clear();				
						chan->insert(client);
						if (!chan->getTopic().empty())
						{
							msg.append(": 332 " + client->getNick() + " " + chan->getName() + " :" + chan->getTopic() + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
							msg.clear();
							msg.append(": 333 " + client->getNick() + " " + chan->getName() + " " + chan->getTopicChanger() + " " + std::to_string(chan->getTopicTime()) + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
						}
						std::vector<Client*> vec = clientInMap(chan->getClientMap());
						msg.append(":" + client->getNick() + "!~" + client->getUser() + " JOIN :" + names[i] + DEL);
						sendAll(vec, msg, client);
						msg.clear();
						for(int k = 0; k != vec.size(); k++)
						{
							if (chan->isMod(vec[k]))
							{
								msg.append(":127.0.0.1 353 " + vec[k]->getNick() + " = " + names[i] + " :@" + vec[k]->getNick() + "\n");
								send(client->getSd(), msg.c_str(), msg.length(), 0);
								msg.clear();
							}
						}
						for(int k = 0; k != vec.size(); k++)
						{
							if (!chan->isMod(vec[k]))
							{
								msg.append(":127.0.0.1 353 " + vec[k]->getNick() + " = " + names[i] + " :" + vec[k]->getNick() + "\n");
								send(client->getSd(), msg.c_str(), msg.length(), 0);
								msg.clear();
							}
						}
						msg.append(":127.0.0.1 366 " + client->getNick() + " " + names[i] + " :End of /NAMES list.\n331\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
					}
					else
					{
						Channel *new_channel = new Channel(names[i], client);
						msg.append(":" + client->getNick() + "!" + client->getUser() + "@127.0.0.1 " + splitted[0] + " " + names[i] + "\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						msg.clear();
						msg.append(":127.0.0.1 353 " + client->getNick() + " = " + names[i] + " :@" + client->getNick() + "\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						msg.clear();
						msg.append(":127.0.0.1 366 " + client->getNick() + " " + names[i] + " :End of /NAMES list.\n331\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						channel_map.insert(std::make_pair(names[i], new_channel));
					}
				}
			}
		}

		void	nickCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string		msg;
			RepliesCreator	reply;

			if (splitted.size() == 1)
			{
				msg.append(reply.makeErrorNeedMoreParams(client->getNick(), splitted[0]));
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else
			{
				if (splitted[1].compare(client->getNick()))
				{
					msg.append(":" + client->getNick() + " ");
					client->setNick(splitted[1]);
					msg.append("NICK :" + splitted[1] + "\n");
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
			}
		}
		void	quitCmd(Client *client, std::vector<std::string> splitted) //rifare
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
			Channel* chan = findChannel(splitted[1]);
			if(!chan)
			{
				msg.append(reply.makeErrorNoSuchChannel(client->getNick(), splitted[1]));
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
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
			std::string msg;
			Channel*	chan;
			if (splitted.size() == 1)
			{
				msg.append(": 461 " + client->getNick() + "MODE :Not enough parameters\n");
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else if(splitted.size() == 2)
			{
				chan = findChannel(splitted[1]);
				if (chan == NULL)
				{
					msg.append(": 401 " + client->getNick() + " " + splitted[1] + " :No such nick/channel\n");
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
				else
				{
					std::string msg = "324 " + client->getNick() + " " + splitted[1] + " +" + DEL;
					send(client->getSd(), (msg + "\n").c_str(), (size_t)msg.length() + 1, MSG_OOB);
					msg.clear();
					msg = "329 " + client->getNick() + " " + splitted[1] + " ";
					msg.append(std::to_string(chan->getTime()) + DEL);
					send(client->getSd(), (msg + "\n").c_str(), (size_t)msg.length() + 1, MSG_OOB);
				}
			}
			else if (splitted.size() > 3)
			{
				chan = findChannel(splitted[1]);
				if (chan == NULL)
				{
					msg.append(": 401 " + client->getNick() + " " + splitted[1] + " :No such nick/channel\n");
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
				else
				{
					if (!chan->isMod(client))
					{
						msg.append(": 482 " + client->getNick() + " " + splitted[1] + " :You're not channel operator\n"); //:italia.ircitalia.net 482 benjo|3 #bau :You're not channel operator
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						return;
					}
					if (!splitted[2].compare("+o")) //op
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " +o " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->modInsert(vec[i]);
									break;
								}
							}
							
						}
					}
					else if (!splitted[2].compare("-o")) //deop
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " -o " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->modErase(vec[i]);
									break;
								}
							}
						}
					}
					else if (!splitted[2].compare("+h")) //halfop
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " +h " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->halfModInsert(vec[i]);
									break;
								}
							}
						}
					}
					else if (!splitted[2].compare("-h")) //dehalfop
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " -h " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->halfModErase(vec[i]);
									break;
								}
							}
						}
					}
					else if (!splitted[2].compare("+v")) //voice
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " +v " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->voiceInsert(vec[i]);
									break;
								}
							}
						}
					}
					else if (!splitted[2].compare("-v")) //devoice
					{
						if (splitted[3].size() != 0)
						{
							std::vector<Client *> vec = clientInMap(client_map);
							for(int i = 0; i != vec.size(); i++)
							{
								if(!vec[i]->getNick().compare(splitted[3]))
								{
									msg.append(":" + client->getNick() + " MODE " + splitted[1] + " -v " + vec[i]->getNick() + DEL);
									sendAll(vec, msg, client);
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									chan->voiceErase(vec[i]);
									break;
								}
							}
						}
					}
				}
			}
		}

		void	privmsgCmd(Client *client, std::vector<std::string> splitted, char *buffer)
		{
			std::string					msg;
			RepliesCreator				reply;
			std::vector<std::string>	nicks;
			std::string					message(buffer);

			message.pop_back();
			message.pop_back();
			if (splitted.size() == 1) // se c'è solo PRIVMSG
			{
				msg.append("411 " + client->getNick() + " :No recipient given (PRIVMSG)\n");
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else if (splitted.size() < 3) // se non c'è il messaggio da mandare
			{
				msg.append("412 " + client->getNick() + " :No text to send\n");
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else //prima devo controllare che il nickname (o tutti quelli in lista? da checkare) esista, poi subito dopo controllo se c'è un messaggio (splitted[2]...)
			{
				nicks = ft_split(splitted[1], ","); //(splitto per ',' per ottenere la lista di utenti e canali a cui mandare il messaggio)
				std::vector<Client *> vec = clientInMap(client_map);
				int sent = 0;
				for (int k = 0; k != nicks.size(); k++)
				{
					if (nicks[k][0] == '#') //se il nick è un channel
					{
						if (findChannel(nicks[k]) != NULL)
						{
							if (splitted[2][0] == ':')
							{
								message.erase(0, message.find(':') + 1);
								msg.append(":" + client->getNick() + "!~" + client->getUser() + " PRIVMSG " + nicks[k] + " :" + message + DEL);
								sendAll(vec, msg, client);
							}
							else
							{
								msg.append(":" + client->getNick() + "!~" + client->getUser() + " PRIVMSG " + nicks[k] + " :" + splitted[2] + DEL);
								sendAll(vec, msg, client);
							}
							sent = 1;
						}
					}
					else
					{
						for (int i = 0; i != vec.size(); i++)
						{
							if (!vec[i]->getNick().compare(nicks[k]))
							{
								if (splitted[2][0] == ':')
								{
									message.erase(0, message.find(':') + 1);
									msg.append(":" + client->getNick() + "!~" + client->getUser() + " PRIVMSG " + nicks[k] + " :" + message);
									send(vec[i]->getSd(), msg.c_str(), msg.length(), 0);
								}
								else
								{
									msg.append(":" + client->getNick() + "!~" + client->getUser() + " PRIVMSG " + nicks[k] + " :" + splitted[2]);
									send(vec[i]->getSd(), msg.c_str(), msg.length(), 0);
								}
								sent = 1;
								break ;
							}
						}
					}
					if (sent == 0)
					{
						msg.append("401 " + client->getNick() + " " + nicks[k] + " :No such nick/channel\n");
						send(client->getSd(), msg.c_str(), msg.length(), 0);
					}
					else
						sent = 0;
				}
			}
		}
		void	partCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string	msg;
			Channel		*chan;
			std::vector<std::string> names;
			names = ft_split(splitted[1], ",");
			if (splitted.size() == 1)
			{
				msg.append(": 461 " + client->getNick() + " PART :Not enough parameters" + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			else if (splitted.size() == 2)
			{
				for (int i = 0; i < names.size(); i++)
				{
					chan = findChannel(names[i]);
					if (chan == NULL)
					{
						msg.append(": 403 " + client->getNick() + " " + names[i] + " :No such channel" + DEL);
						send(client->getSd(), msg.c_str(), msg.length(), 0);
					}
					else
					{
						if (!chan->checkClient(client))
						{
							msg.append(" :442 " + client->getNick() + " " + chan->getName() + " :You're not on that channel" + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
						}
						else
						{
							msg.append(":" + client->getNick() + "!~" + client->getUser() + " PART " + chan->getName() + DEL);
							sendAll(clientInMap(chan->getClientMap()), msg, client);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
							chan->erase(client);
						}
					}
				}
			}
		}
		void	listCmd();
		void	topicCmd(Client *client, std::vector<std::string> splitted, char *buffer)
		{
			std::string	msg;
			Channel*	chan;
			std::string	buf(buffer);
			buf.pop_back();
			buf.pop_back();
			if (splitted.size() == 1)
			{
				msg.append(": 461 " + client->getNick() + " TOPIC :Not enough parameters" + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
				return ;
			}
			chan = findChannel(splitted[1]);
			if (chan == NULL)
			{
				msg.append(": 403 " + client->getNick() + " " + splitted[1] + " :No such channel" + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}
			if (splitted.size() == 2)
			{
				if (chan->getTopic().empty())
				{
					msg.append(": 331 " + client->getNick() + " " + chan->getName() + " :No topic is set." + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
				else
				{
					msg.append(": 332 " + client->getNick() + " " + chan->getName() + " :" + chan->getTopic() + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
					msg.clear();
					msg.append(": 333 " + client->getNick() + " " + chan->getName() + " " + chan->getTopicChanger() + " " + std::to_string(chan->getTopicTime()) + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
			}
			else if (splitted.size() > 2)
			{
				if (splitted[2][0] == ':')
				{
					buf.erase(0, buf.find(':') + 1);
					msg.append(":" + client->getNick() + "!~" + client->getUser() + " TOPIC " + chan->getName() + " :" + buf + DEL);
					sendAll(clientInMap(chan->getClientMap()), msg, client);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
					chan->setTopic(buf);
					chan->setTopicTime();
					chan->setTopicChanger(client->getNick());
				}
				else
				{
					msg.append(":" + client->getNick() + "!~" + client->getUser() + " TOPIC " + chan->getName() + " :" + splitted[2] + DEL);
					sendAll(clientInMap(chan->getClientMap()), msg, client);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
					chan->setTopic(splitted[2]);
					chan->setTopicTime();
					chan->setTopicChanger(client->getNick());
				}
			}
		}
		void	motdCmd();
		void	noticeCmd();
		void	kickCmd(Client *client, std::vector<std::string> splitted)
		{
			std::string msg;
			if (splitted.size() < 3 )
				send(client->getSd(), "KICK requires more parameters\n", 31, 0);
			else
			{
				Channel *chan = findChannel(splitted[2]);
				if (chan != NULL) //se il canale esiste
				{
					std::cout << "ho trovato il canale\n";
					int i = 0;
					while (i < chan->getModClients().size())
					{
						if (chan->getModClients()[i] == client) //verifico che l'utente sia MOD
						{
							std::map<int, Client *>::iterator y;
							for (y = chan->getClientMap().begin(); y != chan->getClientMap().end(); y++)
							{
								if (y->second->getNick() == splitted[3]) //se esiste il client da kickare
								{
									//:ffafa!~fafa@Azzurra-3476AEA0.business.telecomitalia.it KICK #cacca frapp :ffafa
									msg.append(":" + client->getNick() + "!~" + client->getUser() + "KICK " + splitted[2] + " :" + client->getNick());
									send(client->getSd(), msg.c_str(), msg.length(), 0);
									sendAll(clientInMap(chan->getClientMap()), msg, client);
									chan->getClientMap().erase(y->first);
									break ;
								}
							}
							if (y == chan->getClientMap().end()) //se non c'é il client
							{
								msg.append(splitted[3] + ": No such nick/channel" + DEL);
								send(client->getSd(), msg.c_str(), msg.length(), 0);
								break ;
							}
						}
						else if (i == chan->getModClients().size()) //se non é OP 
						{
							msg.append(splitted[2] + ": You're not channel operator" + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
							break ;
						}
						i++;	
					}
				}
				else
				{
					msg.append(splitted[3] + ": No such nick/channel" + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
			}
		}
		void	inviteCmd();
		~Server();

};