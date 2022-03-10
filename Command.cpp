#include "Server.hpp"

void	Server::userCmd() {}

void	Server::pingCmd(Client *client, std::vector<std::string> splitted)
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

void	Server::joinCmd(Client *client, std::vector<std::string> splitted) 
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
				if (chan->bannedFind(client->getNick()) == -1 && !chan->allBanned())
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
					msg.append(": 474 " + client->getNick() + " " + splitted[1] + " :Cannot join channel (+b)" + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
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

void	Server::nickCmd(Client *client, std::vector<std::string> splitted)
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
void	Server::quitCmd(Client *client, std::vector<std::string> splitted) //rifare
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
void	Server::whoCmd(Client *client, std::vector<std::string>splitted)
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

void	Server::modeCmd(Client *client, std::vector<std::string>splitted)
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
	else if (splitted.size() > 2)
	{
		chan = findChannel(splitted[1]);
		if (chan == NULL)
		{
			msg.append(": 401 " + client->getNick() + " " + splitted[1] + " :No such nick/channel\n");
			send(client->getSd(), msg.c_str(), msg.length(), 0);
		}
		else
		{
			if (!chan->isMod(client) && splitted[2].compare("b"))
			{
				msg.append(": 482 " + client->getNick() + " " + splitted[1] + " :You're not channel operator\n");
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
			else if(!splitted[2].compare("+b") || !splitted[2].compare("-b") || !splitted[2].compare("b"))
			{
				if (splitted.size() == 3)
				{
					for (int i = 0; i < chan->getBanned().size(); i++)
					{
						msg.append(": 367 " + client->getNick() + " " + splitted[1] + " " + chan->getBanned()[i].name + "!*@* " + chan->getBanned()[i].nickMod + "!~" + chan->getBanned()[i].userMod + " " + std::to_string(chan->getBanned()[i].ban_time) + DEL);
						send(client->getSd(), msg.c_str(), msg.length(), 0);
						msg.clear();
					}
					msg.append(": 368 " + client->getNick() + " " + splitted[1] + " :End of Channel Ban List" + DEL);
					send(client->getSd(), msg.c_str(), msg.length(), 0);
				}
				else
				{
					if (!splitted[2].compare("+b"))
					{
						if (chan->bannedFind(splitted[3]) == -1)
						{
							chan->bannedInsert(splitted[3], client->getNick(), client->getUser());
							msg.append(":" + client->getNick() + "!~" + client->getUser() + " MODE " + splitted[1] + " +b " + splitted[3] + "!*@*" + DEL);
							sendAll(clientInMap(client_map), msg, client);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
						}
					}
					else
					{
						if (splitted[3] == "!" || splitted[3] == "*!" || splitted[3] == "*!")
						{
							chan->setAllBanned(false);
							return ;
						}
						chan->bannedErase(chan->bannedFind(splitted[3]));
						msg.append(":" + client->getNick() + "!~" + client->getUser() + " MODE " + splitted[1] + " -b " + splitted[3] + "!*@*" + DEL);
						sendAll(clientInMap(client_map), msg, client);
						send(client->getSd(), msg.c_str(), msg.length(), 0);
					}
				}
			}
		}
	}
}

void	Server::privmsgCmd(Client *client, std::vector<std::string> splitted, char *buffer) //controllare che chi manda il messaggio sia autorizzato a mandarlo (ban, voice)
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
	else //prima devo controllare che il nickname esista, poi subito dopo controllo se c'è un messaggio (splitted[2]...)
	{
		nicks = ft_split(splitted[1], ","); //(splitto per ',' per ottenere la lista di utenti e canali a cui mandare il messaggio)
		std::vector<Client *> vec = clientInMap(client_map);
		int sent = 0;
		for (int k = 0; k != nicks.size(); k++)
		{
			if (nicks[k][0] == '#') //se il nick è un channel
			{
				Channel *chan = findChannel(nicks[k]);
				if (chan != NULL)
				{
					if (chan->bannedFind(client->getNick()) == -1)
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
					}
					else
					{
						//: 404 frappinz #ciaociao :Cannot send to channel
						msg.append(": 404 " + client->getNick() + " " + chan->getName() + " :Cannot send to channel" + DEL);
						send(client->getSd(), msg.c_str(), msg.length(), 0);
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
void	Server::partCmd(Client *client, std::vector<std::string> splitted)
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
void	Server::listCmd() {}
void	Server::topicCmd(Client *client, std::vector<std::string> splitted, char *buffer)
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
		return ;
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
void	Server::noticeCmd(Client *client, std::vector<std::string> splitted,  char *buffer)
{
	std::string					msg;
	RepliesCreator				reply;
	std::vector<std::string>	nicks;
	std::string					message(buffer);

	message.pop_back();
	message.pop_back();
	if (splitted.size() == 1) // se c'è solo NOTICE
	{
		msg.append("411 " + client->getNick() + " :No recipient given (NOTICE)\n");
		send(client->getSd(), msg.c_str(), msg.length(), 0);
	}
	else if (splitted.size() < 3) // se non c'è il messaggio da mandare
	{
		msg.append("412 " + client->getNick() + " :No text to send\n");
		send(client->getSd(), msg.c_str(), msg.length(), 0);
	}
	else //prima devo controllare che il nickname esista, poi subito dopo controllo se c'è un messaggio (splitted[2]...)
	{
		nicks = ft_split(splitted[1], ","); //(splitto per ',' per ottenere la lista di utenti e canali a cui mandare il messaggio)
		std::vector<Client *> vec = clientInMap(client_map);
		int sent = 0;
		for (int k = 0; k != nicks.size(); k++)
		{
			if (nicks[k][0] == '#') //se il nick è un channel
			{
				Channel *chan = findChannel(nicks[k]);
				if (chan != NULL)
				{
					if (chan->bannedFind(client->getNick()) == -1 || !chan->allBanned())
					{
						if (splitted[2][0] == ':')
						{
							message.erase(0, message.find(':') + 1);
							msg.append(":" + client->getNick() + "!" + client->getUser() + " NOTICE " + nicks[k] + " :" + message + DEL);
							sendAll(vec, msg, client);
						}
						else
						{
							msg.append(":" + client->getNick() + "!" + client->getUser() + " NOTICE " + nicks[k] + " :");
							int i = 2;
							while (i != splitted.size())
							{
								msg.append(splitted[i] + " ");
								i++;
							}
							msg.append(DEL);
							sendAll(vec, msg, client);
							//:frapp!frappinz@IRCItalia-7BBFBF6E.business.telecomitalia.it NOTICE #pupu :ciaoo a ttt
						}
					}
					else
					{
						//: 404 frappinz #ciaociao :Cannot send to channel
						msg.append(": 404 " + client->getNick() + " " + chan->getName() + " :Cannot send to channel" + DEL);
						send(client->getSd(), msg.c_str(), msg.length(), 0);
					}
					sent = 1;
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

	//NOTICE
	//:italia.ircitalia.net 411 frapp|3 :No recipient given (NOTICE)
	//NOTICE #ca | NOTICE ciao
	//italia.ircitalia.net 412 frapp|3 :No text to send
	//NOTICE #ca ciao a tutti | NOTICE #ca :ciao a tutti
	//rapp|3!frappinz@IRCItalia-7BBFBF6E.business.telecomitalia.it NOTICE #ca :ciao a tutti
}
void	Server::kickCmd(Client *client, std::vector<std::string> splitted)
{
	std::string msg;
	if (splitted.size() < 3 )
	{
		msg.append(": 461 " + client->getNick() + " KICK :Not enough parameters" + DEL);
		send(client->getSd(), msg.c_str(), msg.length(), 0);
	}
	else
	{
		std::vector<std::string> nicks = ft_split(splitted[2], ",");
		Channel *chan = findChannel(splitted[1]);
		if (chan != NULL) //se il canale esiste
		{
			if (chan->isMod(client)) //verifico che l'utente sia MOD
			{
				std::vector<Client *> chanClient = clientInMap(chan->getClientMap());
				for(int i = 0; i < nicks.size(); i++)
				{
					for (int y = 0; y != chanClient.size(); y++)
					{
						if(!chanClient[y]->getNick().compare(nicks[i])) //se esiste il client da kickare
						{
							msg.append(":" + client->getNick() + "!~" + client->getUser() + " KICK " + splitted[1] + " " + chanClient[y]->getNick() + " :" + client->getNick() + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
							sendAll(chanClient, msg, client);
							msg.clear();
							chan->erase(chanClient[y]);
							chanClient = clientInMap(chan->getClientMap());
							break ;
						}
						else if (y == chanClient.size() - 1) //se non c'é il client
						{
							msg.append(": 401 " + client->getNick() + " " + nicks[i] + " :No such nick/channel" + DEL);
							send(client->getSd(), msg.c_str(), msg.length(), 0);
							break ;
						}
					}
				}
			}
			else
			{
				msg.append(": 482 " + client->getNick() + " " + splitted[1] + " :You're not channel operator" + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
			}	
		}
		else
		{
			msg.append(": 403 " + client->getNick() + " " + splitted[1] + " :No such nick/channel" + DEL);
			send(client->getSd(), msg.c_str(), msg.length(), 0);
		}
	}
}

void	Server::inviteCmd() {}