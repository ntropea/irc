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
#include <sys/time.h>
#include <ctime>
#include "Client.hpp"
#include "Channel.hpp"

#define DEL "\n"
#define MAX_CLIENTS 200

class	Client;

class	Channel;

std::vector<std::string>	ft_split(std::string str, std::string token);

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
		std::string								date;
	public:
		Server(){};
		Server(int port, std::string pass);
		std::string	getPass() {return pass;};
		void		client_dc(int sd, int i);
		void 		run();
		void		setDate() { time_t now = time(0); date = std::string(ctime(&now));}
		std::string			getDate() { return date; }

		/* COMANDI */
		void		parse_commands(Client *client, char *buffer, int valread, int i);
		void		checkChannels();
		void		sendAll(std::vector<Client*> vec, std::string msg, Client *client);
		void		pingCmd(Client *client, std::vector<std::string> splitted);
		void		joinCmd(Client *client, std::vector<std::string> splitted);
		void		nickCmd(Client *client, std::vector<std::string> splitted);			
		void		quitCmd(Client *client, std::vector<std::string> splitted);
		void		whoCmd(Client *client, std::vector<std::string>splitted);
		void		modeCmd(Client *client, std::vector<std::string>splitted);
		void		privmsgCmd(Client *client, std::vector<std::string> splitted, char *buffer);
		void		partCmd(Client *client, std::vector<std::string> splitted);
		void		listCmd(Client *client, std::vector<std::string> splitted);
		void		topicCmd(Client *client, std::vector<std::string> splitted, char *buffer);
		void		noticeCmd(Client *client, std::vector<std::string> splitted,  char *buffer);
		void		kickCmd(Client *client, std::vector<std::string> splitted);
		void		inviteCmd();
		/* UTILS*/
		std::vector<Client*> 			clientInMap(std::map<int, Client*> map);
		Channel* 						findChannel(std::string nameChannel);
		std::map<std::string, Channel*>	getChannelMap() { return channel_map; };
		~Server();
};


Server::Server(int port, std::string pass)
{
	this->port = port;
	this->pass = pass;
	memset( &timeout, 0, sizeof(timeout) );
    timeout.tv_sec = 3 * 60;
	opt = 1;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	for (int i = 0; i < MAX_CLIENTS; i++)
		clients_sd[i] = 0;
	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(sock, 6) < 0)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	setDate();
};

void	Server::parse_commands(Client *client, char *buffer, int valread, int i)
{
	std::string sent;
	
	std::vector<std::string> splitted;
	std::string buf(buffer, (size_t)valread);
	if (buf.size() == 1 && buf[0] == '\n')
		return ;
	if (buf.find("\r", 0) != std::string::npos)
		buf.pop_back();
	buf.pop_back();
	splitted = ft_split(buf, " ");
	if (!strncmp(buffer, "QUIT", 4) || !strncmp(buffer, "quit", 4))
		client_dc(sd, i); 
	else if (!strncmp(buffer, "JOIN", 4) || !strncmp(buffer, "join", 4))
		joinCmd(client, splitted);
	else if (!strncmp(buffer, "PING", 4))
		pingCmd(client, splitted);
	else if (!strncmp(buffer, "NICK", 4) || !strncmp(buffer, "nick", 4))
		nickCmd(client, splitted);
	else if (!strncmp(buffer, "PRIVMSG", 7) || !strncmp(buffer, "privmsg", 7))
		privmsgCmd(client, splitted, buffer);
	else if (!strncmp(buffer, "NOTICE", 6)  || !strncmp(buffer, "notice", 6))
		noticeCmd(client, splitted, buffer);
	else if (!strncmp(buffer, "WHO", 3) || !strncmp(buffer, "who", 3))
		whoCmd(client, splitted);
	else if (!strncmp(buffer, "MODE", 4) || !strncmp(buffer, "mode", 4))
		modeCmd(client, splitted);
	else if (!strncmp(buffer, "PART", 4) || !strncmp(buffer, "part", 4))
		partCmd(client, splitted);
	else if (!strncmp(buffer, "TOPIC", 5) || !strncmp(buffer, "topic", 5))
		topicCmd(client, splitted, buffer);
	else if (!strncmp(buffer, "KICK", 4) || !strncmp(buffer, "kick", 4))
		kickCmd(client, splitted);
	else if (!strncmp(buffer, "LIST", 4) || !strncmp(buffer, "list", 4))
		listCmd(client, splitted);
	else
	{
		sent.append("421 " + client->getNick() + " " + splitted[0] + " :Unknown command" + DEL);
		send(client->getSd(), sent.c_str(), sent.length(), 0);
	}
	checkChannels();

}

void	Server::client_dc(int sd, int i)
{
	getsockname(sd , (struct sockaddr*)&addr , (socklen_t*)&addrlen);
	std::cout << "The disconnected host was named " << client_map.find(sd)->second->getUser() << std::endl;
	client_map.find(sd)->second->setLogged(false);
	client_map.erase(sd);
	close(sd);
	clients_sd[i] = 0;
}

std::vector<std::string> ft_split(std::string str, std::string token)
{
	std::vector<std::string>result;
	while (str.size())
	{
		unsigned long index = str.find(token);
		if (index!=std::string::npos)
		{
			if (index != 0)
				result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
		}
		else
		{
			result.push_back(str);
			str = "";
		}
	}
	return (result);
}

int	parse_pass(Client *new_client, std::string s)
{
	std::vector<std::string>	raw_parse;
	raw_parse = ft_split(s, " :");
	if (raw_parse[1][raw_parse[1].length() - 1] == '\n')
		raw_parse[1].resize(raw_parse[1].length() - 1);
	std::cout << "|" << raw_parse[1] << "|\n"; 
	if (!new_client->server->getPass().compare(raw_parse[1]))
		return(1);
	return (0);
}

void	parse_nick(Client *new_client, std::string s, std::map<int, Client*> map)
{
    std::vector<std::string> raw_nick;
    raw_nick = ft_split(s, " ");
    int i = 1;

	new_client->setNick(raw_nick[1]);
	if(map.size() == 1)
		return;
	size_t pipe = raw_nick[1].find("|", 0);
	if (pipe != std::string::npos) // se troviamo giá una pipe nei due nomi uguali
	{
		raw_nick[1].resize(pipe);
		new_client->setNick(raw_nick[1]);
	}
	for(std::map<int, Client*>::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(!it->second->getNick().compare(new_client->getNick()) && new_client->getSd() != it->first) //se troviamo due nomi uguali
		{
			new_client->setNick(raw_nick[1] + "|" + std::to_string(i));
			i++;
		}
	}
}

void	parse_user(Client *new_client, std::string s, std::map<int, Client*> map)
{
    std::vector<std::string> raw_user;
    raw_user = ft_split(s, " ");
    int i = 1;

	new_client->setUser(raw_user[1]);
	if(map.size() == 1)
		return;
	size_t pipe = raw_user[1].find("|", 0);
	if (pipe != std::string::npos) // se troviamo giá una pipe nei due nomi uguali
	{
		raw_user[1].resize(pipe);
		new_client->setUser(raw_user[1]);
	}
	for(std::map<int, Client*>::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(!it->second->getUser().compare(new_client->getUser()) && new_client->getSd() != it->first) //se troviamo due nomi uguali
		{
			new_client->setUser(raw_user[1] + "|" + std::to_string(i));
			i++;
		}
	}
}

int		parse_info(Client *new_client, char *buffer, int valread, std::map<int, Client*> map)
{

	std::vector<std::string>	raw_parse;
	std::string					raw_string(buffer, (size_t)valread);
	std::string					sent;

	//std::cout << raw_string << std::endl;
	raw_parse = ft_split(raw_string, "\r\n");
	if (!raw_parse[0].compare(0, 5, "PASS :")){
		parse_nick(new_client, raw_parse[0], map);
		sent.append("464 " + new_client->getNick() + " :Password incorrect" + DEL);
		send(new_client->getSd(), sent.c_str(), sent.length(), 0);
		return (-1);
	}
	else
	{
		if (!parse_pass(new_client, raw_parse[0]))
		{
			if (raw_parse.size() > 1)
				parse_nick(new_client, raw_parse[1], map);
			sent.append("464 " + new_client->getNick() + " :Password incorrect" + DEL);
			send(new_client->getSd(), sent.c_str(), sent.length(), 0);
			return (-1);
		}
		if (raw_parse[1].length()){
			parse_nick(new_client, raw_parse[1], map);	
			parse_user(new_client, raw_parse[2], map);
		}
	}
	new_client->setLogged(true);
	sent.append("001 " + new_client->getNick() + " :Welcome to the IRC Network, " + new_client->getUser() + DEL);
    sent.append("002 " + new_client->getNick() + " :Your host is IRC, running version 2.1" + DEL);
	sent.append("003 " + new_client->getNick() + " :This server was created " + new_client->server->getDate() + DEL);
    send(new_client->getSd(), sent.c_str(), sent.length(), 0);
	return (0);
}


void	Server::run() //aggiungere signal per ctrl-c e ctrl-d
{
	std::string	w = "Welcome to my IRC server! uwu\n";
	int valread = 0;
	char buffer[1025];
	while (1)
	{
		FD_ZERO(&read_set);						//impostiamo tutto a 0
		FD_SET(sock, &read_set);				//mettiamo fd del server dentro array di fd
		max_sd = sock;
		for (int i = 0; i < MAX_CLIENTS; i++) 	//tramite il nostro array di client, risistemiamo gli fd in readset
		{
			sd = clients_sd[i];
			if (sd > 0)
				FD_SET(sd, &read_set);
			if (sd > max_sd)
				max_sd = sd;
		}
		activity = select(max_sd + 1, &read_set, NULL, NULL, NULL); //qua rimane in loop
        if ((activity < 0) && (errno != EINTR))
            perror("Error select");
		//If something happened on the master socket , 
        //then its an incoming connection
		if (FD_ISSET(sock, &read_set))
		{
			if ((new_sd = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) < 0)
			{
				perror("accept");
				exit(1);
			}
			printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_sd , inet_ntoa(addr.sin_addr) , ntohs(addr.sin_port));
			if (setsockopt(new_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			{
			 	perror("setsockopt failed");
				exit(EXIT_FAILURE);
			}
			if ((send(new_sd, w.c_str(), w.length(), 0)) < 0)
				perror("send");
			for (int i = 0; i < MAX_CLIENTS; i++) //aggiungiamo il socket all'array di fd
			{
				if (clients_sd[i] == 0)
				{
					Client *new_client = new Client;
					clients_sd[i] = new_sd;
					new_client->setSd(new_sd);
					new_client->server = this;
					client_map.insert(std::make_pair(clients_sd[i], new_client));
					break ;
				}
			}
		}
		for (int i = 0; i < MAX_CLIENTS; i++)// se non é una nuova connessione é qualche altra azione
        {
            sd = clients_sd[i];
            if (FD_ISSET(sd, &read_set)) //controllo se qualcuno si é disconnesso e controllo nuovi messaggi
            {
                if ((valread = read(sd, buffer, 1024)) == 0) //Se ha fatto cntrl c da client
					client_dc(sd, i);
                else
                {
                    buffer[valread] = '\0';
                    if (client_map.find(sd)->second->getLog() == false) //se é la prima connessione e non ha loggato
					{
						if (parse_info(client_map.find(sd)->second, buffer, valread, client_map) == -1)
							client_dc(sd, i);
					}
					else
                    	parse_commands(client_map.find(sd)->second, buffer, valread, i);
                }
            }
        }
	}
}

Channel* Server::findChannel(std::string nameChannel)
{ 
	for(std::map<std::string, Channel*>::iterator i = channel_map.begin(); i != channel_map.end(); i++)
	{
		if (!i->first.compare(nameChannel))
			return(i->second);
	}
	return(NULL);
};
void	Server::checkChannels()
{
	std::map<std::string, Channel *>::iterator i = channel_map.begin();
	while (i != channel_map.end())
	{
		if (i->second->getClientMap().empty())
			channel_map.erase(i++);
		else
			++i;
	}
}
std::vector<Client*> Server::clientInMap(std::map<int, Client*> map)
{
	std::vector<Client*> vec;
	for(std::map<int, Client*>::iterator i = map.begin(); i != map.end(); i++)
		vec.push_back(i->second);
	return vec;
};
void	Server::sendAll(std::vector<Client*> vec, std::string msg, Client *client)
{

	for(int k = 0; k != vec.size(); k++)
	{
		if(client->getSd() != vec[k]->getSd())
			send(vec[k]->getSd(), msg.c_str(), msg.length(), 0);
	}
}

/***************************** COMMANDS *****************************/

void	Server::pingCmd(Client *client, std::vector<std::string> splitted)
{
	std::string msg;
	

	if (splitted.size() < 2)
		msg.append("409 " + client->getNick() + " :No origin specified" + DEL);
	else if (splitted.size() == 2)
		msg.append("PONG " + splitted[1] + "\n");
	else
		msg.append("PONG " + splitted[2] + " " + splitted[1] + "\n");
	send(client->getSd(), msg.c_str(), msg.length(), 0);
};

void	Server::joinCmd(Client *client, std::vector<std::string> splitted) 
{
	
	std::string msg;
	std::vector<std::string> names;
	names = ft_split(splitted[1], ",");
	for (int i = 0; i < names.size(); i++)
	{
		if (names[i][0] != '#')
		{
			msg.append("403 "  + client->getNick() + " " + names[i] + " :No such channel" + DEL);
			send(client->getSd(), msg.c_str(), msg.length(), 0);
		}
		else
		{
			Channel* chan = findChannel(names[i]);
			if(chan != NULL)
			{
				if (chan->bannedFind(client->getNick()) == -1)
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

	if (splitted.size() == 1)
	{
		msg.append( "461 " + client->getNick() + " " + splitted[0] + " :Not enough parameters" + DEL);
		send(client->getSd(), msg.c_str(), msg.length(), 0);
	}
	else if (splitted[1].compare(client->getNick()))
	{
		for(std::map<int, Client*>::iterator it = client_map.begin(); it != client_map.end(); ++it)
		{
			if (!it->second->getNick().compare(splitted[1]))
			{
				//: 433 fra frapp|2 :Nickname is already in use.
				msg.append(": 433 " + client->getNick() + " " + splitted[1] + " :Nickname is already in use." + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
				return;
			}
		}
		//:bauuu!frappinz NICK :fra
		msg.append(":" + client->getNick() + "!" + client->getUser() + " NICK :" + splitted[1] + DEL);
		client->setNick(splitted[1]);
		//:bau!frappinz NICK :bauuu
		send(client->getSd(), msg.c_str(), msg.length(), 0);
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
	
	Channel* chan = findChannel(splitted[1]);
	if(!chan)
	{
		msg.append("403 "  + client->getNick() + " " + splitted[1] + " :No such channel" + DEL);
		send(client->getSd(), msg.c_str(), msg.length(), 0);
	}
	else
	{
		std::vector<Client*> vec = clientInMap(chan->getClientMap());
		for(int k = 0; k != vec.size(); k++)
		{
			msg = "352 " + vec[k]->getNick() + " " + splitted[1] + DEL;
			send(client->getSd(), msg.c_str(), msg.length(), 0);
			msg.clear();
		}
		msg = "315 " + client->getNick() + " " + splitted[1] + ":End of WHO list" + DEL;
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
void	Server::listCmd(Client *client, std::vector<std::string> splitted) 
{
	std::string msg;
	std::vector<std::string> names;

	msg.append(": 321 " + client->getNick() + " Channel :Users  Name" + DEL);
	send(client->getSd(), msg.c_str(), msg.length(), 0);
	msg.clear();
	if (splitted.size() == 1)
	{
		for (std::map<std::string, Channel*>::iterator it = channel_map.begin(); it != channel_map.end(); ++it)
		{
			msg.append(": 322 " + client->getNick() + " " + it->second->getName() + " " + std::to_string(it->second->getClientMap().size())  + " : " + it->second->getTopic() + DEL);
			send(client->getSd(), msg.c_str(), msg.length(), 0);
			msg.clear();
		}
	}
	else
	{
		names = ft_split(splitted[1], ",");
		for (int i = 0; i < names.size(); i++)
		{
			std::map<std::string, Channel*>::iterator it = channel_map.find(names[i]);
			if (names[i][0] != '#')
			{
				msg.append(": 521 " + client->getNick() + " :Bad list syntax, type /quote list ? or /raw list ?" + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
				msg.clear();
			}
			else if (it != channel_map.end())
			{
				msg.append(": 322 " + client->getNick() + " " + it->second->getName() + " " + std::to_string(it->second->getClientMap().size()) + " : " + it->second->getTopic() + DEL);
				send(client->getSd(), msg.c_str(), msg.length(), 0);
				msg.clear();
			}
		}
	}
	msg.append(": 323 " + client->getNick() + " :End of /LIST" + DEL);
	send(client->getSd(), msg.c_str(), msg.length(), 0);
}
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

Server::~Server(){};
