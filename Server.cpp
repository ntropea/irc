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

std::vector<std::string> ft_split(std::string str, std::string token)
{
	std::vector<std::string>result;
	while (str.size()){
		unsigned long index = str.find(token);
		if (index!=std::string::npos){
			result.push_back(str.substr(0, index));
			str = str.substr(index + token.size());
			if(str.size()==0)
				result.push_back(str);}
		else{
			result.push_back(str);
			str = "";
		}
	}
	return (result);
}

int	parse_pass(Client *new_client, std::string s)
{
	std::vector<std::string>	raw_parse;
	raw_parse = ft_split(s, " ");
	if (raw_parse[1].compare(new_client->server->getPass()))
		return (1);
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

	raw_parse = ft_split(raw_string, "\r\n");
	if (raw_parse[0].find("PASS :") == std::string::npos){
		std::cout << "Ciao" << std::endl;
		parse_nick(new_client, raw_parse[0]);
		sent.append(reply.makePasswdMisMatch(new_client->getNick()));
		send(new_client->getSd(), sent.c_str(), sent.length(), 0);
		return (-1);
	}
	else
	{
		if (!parse_pass(new_client, raw_parse[0]))
		{
			parse_nick(new_client, raw_parse[1]);
			sent.append(reply.makePasswdMisMatch(new_client->getNick()));
			send(new_client->getSd(), sent.c_str(), sent.length(), 0);
			return (-1);
		}
		parse_nick(new_client, raw_parse[1]);
		parse_user(new_client, raw_parse[2]);
	}
	sent.append(reply.makeWelcome(new_client->getNick(), new_client->getUser(), "Irc"));
    sent.append(reply.makeYourHost("Irc", "2.1", new_client->getNick()));
    sent.append(reply.makeCreated("yesterday", new_client->getNick()));
    send(new_client->getSd(), sent.c_str(), sent.length(), 0);
	return (0);
}

void	Server::run()
{
	std::string	w = "Welcome to my IRC server! uwu";
	int valread = 0;
	char buffer[1025];
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
	    //std::cout << "activity is " << activity << std::endl;
        if ((activity < 0) && (errno != EINTR))
            perror("Error");
		if (FD_ISSET(sock, &read_set))
		{
			if ((new_sd = accept(sock, (struct sockaddr *) &addr, (socklen_t *) &addrlen)) < 0)
			{
				perror("Error");
				exit(1);
			}
			if ((send(new_sd, w.c_str(), w.length(), 0)) < 0)
				perror("Error");
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (clients_sd[i] == 0)
				{
					Client *new_client = new Client;
					clients_sd[i] = new_sd;
					new_client->setSd(new_sd);
					new_client->server = this;
					map.insert(std::make_pair(clients_sd[i], new_client));
					break ;
				}
			}
		}
		for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = clients_sd[i];
            if (FD_ISSET(sd, &read_set))
            {
                if ((valread = read(sd, buffer, 1024)) == 0)
                {
                    getsockname(sd , (struct sockaddr*)&addr , (socklen_t*)&addrlen);  
                    std::cout << "The disconnected host was named " << map.find(sd)->second->getUser() << std::endl;
                    printf("Host disconnected , sd is %d, ip %s , port %d \n" , sd, 
                          inet_ntoa(addr.sin_addr) , ntohs(addr.sin_port));
					delete	map.find(sd)->second;
					map.erase(sd);
                    close(sd);
                    clients_sd[i] = 0;
                }
                else
                {
                    buffer[valread] = '\0';
                    if (map.find(sd)->second->getLog() == false)
					{
						if (parse_info(map.find(sd)->second, buffer, valread) == -1)
						{
							delete	map.find(sd)->second;
							map.erase(sd);
							close(sd);
							clients_sd[i] = 0;
						}
					}
                    else if (map.find(sd)->second->getLog() == true)
					{
                        std::cout << "this is parse_commands shit" << std::endl;
                        //parse_commands(mappa.find(sd)->second, buffer, valread);
					}
                    //send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
	}
}

Server::~Server(){};