#include "Server.hpp"

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
};

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
	if (raw_parse[1][raw_parse[1].length()-1] == '\n')
		raw_parse[1].resize(raw_parse[1].length() - 1); 
	if (!new_client->server->getPass().compare(raw_parse[1]))
		return(1);
	return (0);
}

void	parse_nick(Client *new_client, std::string s)
{
	std::vector<std::string>	raw_parse;
	raw_parse = ft_split(s, " ");
	new_client->setNick(raw_parse[1]);
}

void	parse_user(Client *new_client, std::string s){
    std::vector<std::string>raw_user;
    raw_user = ft_split(s, " ");
    new_client->setUser(raw_user[1]);
}

int	parse_info(Client *new_client, char *buffer, int valread)
{
	RepliesCreator				reply;
	std::vector<std::string>	raw_parse;
	std::string					raw_string(buffer, (size_t)valread);
	std::string					sent;

	std::cout << raw_string << std::endl;
	raw_parse = ft_split(raw_string, "\r\n");
	if (!raw_parse[0].compare(0, 5, "PASS :")){
		parse_nick(new_client, raw_parse[0]);
		sent.append(reply.makePasswdMisMatch(new_client->getNick()));
		send(new_client->getSd(), sent.c_str(), sent.length(), 0);
		return (-1);
	}
	else
	{
		if (!parse_pass(new_client, raw_parse[0]))
		{
			if (raw_parse.size() > 1)
				parse_nick(new_client, raw_parse[1]);
			sent.append(reply.makePasswdMisMatch(new_client->getNick()));
			send(new_client->getSd(), sent.c_str(), sent.length(), 0);
			return (-1);
		}
		if (raw_parse[1].length()){
			parse_nick(new_client, raw_parse[1]);	
			parse_user(new_client, raw_parse[2]);
		}
	}
	new_client->setLogged(true);
	sent.append(reply.makeWelcome(new_client->getNick(), new_client->getUser(), "Irc"));
    sent.append(reply.makeYourHost("Irc", "2.1", new_client->getNick()));
    sent.append(reply.makeCreated("yesterday", new_client->getNick()));
    send(new_client->getSd(), sent.c_str(), sent.length(), 0);
	return (0);
}

void	Server::parse_commands(Client *client, char *buffer, int valread, int i)
{
	std::string sent;
	RepliesCreator reply;
	std::vector<std::string> splitted;
	std::string buf(buffer, (size_t)valread);
	buf.pop_back();
	buf.pop_back();
	splitted = ft_split(buf, " ");
	if (!strncmp(buffer, "QUIT", 4))
		client_dc(sd, i); 
	else if (!strncmp(buffer, "JOIN", 4))
		joinCmd(client, splitted);
	else if (!strncmp(buffer, "PING", 4))
		pingCmd(client, splitted);
	else if (!strncmp(buffer, "NICK", 4))
		nickCmd(client, splitted);
	else if (!strncmp(buffer, "PRIVMSG", 7))
		privmsgCmd(client, splitted, buffer);
	else if (!strncmp(buffer, "WHO", 3))
		whoCmd(client, splitted);
	else if (!strncmp(buffer, "MODE", 4))
		modeCmd(client, splitted);
	else if (!strncmp(buffer, "PART", 4))
		partCmd(client, splitted);
	else if (!strncmp(buffer, "TOPIC", 5))
		topicCmd(client, splitted, buffer);
	else
	{
		if (splitted[0][splitted[0].length() - 1] == '\n')
			splitted[0].pop_back();
		sent.append(reply.makeErrorUnknownCommand(splitted[0]));
		send(client->getSd(), sent.c_str(), sent.length(), 0);
	}
}

void	Server::client_dc(int sd, int i)
{
	send(sd, "GOODBYE :)))))))))))\n", 22, 0);
	getsockname(sd , (struct sockaddr*)&addr , (socklen_t*)&addrlen);
	std::cout << "The disconnected host was named " << client_map.find(sd)->second->getUser() << std::endl;
	client_map.find(sd)->second->setLogged(false);
	client_map.erase(sd);
	close(sd);
	clients_sd[i] = 0;
}


void	Server::run()
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
						if (parse_info(client_map.find(sd)->second, buffer, valread) == -1)
							client_dc(sd, i);
					}
					else
                    	parse_commands(client_map.find(sd)->second, buffer, valread, i);
                }
            }
        }
	}
}

Server::~Server(){};