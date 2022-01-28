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
	    std::cout << "activity is " << activity << std::endl;
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
					//parsing
					break ;
				}
			}
		}
		for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = clients_sd[i];
            if (FD_ISSET(sd, &read_set))
            {
                if ((valread = read(sd, buffer, 1024)) == 0) //if return is 0, somebody disconnected
                {
                    getsockname(sd , (struct sockaddr*)&addr , (socklen_t*)&addrlen);  
                    std::cout << "The disconnected host was named " << map.find(sd)->second->getUser() << std::endl;
                    printf("Host disconnected , sd is %d, ip %s , port %d \n" , sd, 
                          inet_ntoa(addr.sin_addr) , ntohs(addr.sin_port));     
                    //Close the socket and mark as 0 in list for reuse 
                    close(sd);  
                    clients_sd[i] = 0;
                }
                else
                {
                    buffer[valread] = '\0';
                    if (map.find(sd)->second->getLog() == false)
					{
                        std::cout << "this is parse info shit" << std::endl;
                        //parse_info(map.find(sd)->second, buffer, valread);
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