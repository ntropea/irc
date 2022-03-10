// #include "Server.hpp"

// std::vector<std::string> ft_split(std::string str, std::string token)
// {
// 	std::vector<std::string>result;
// 	while (str.size())
// 	{
// 		unsigned long index = str.find(token);
// 		if (index!=std::string::npos)
// 		{
// 			if (index != 0)
// 				result.push_back(str.substr(0, index));
// 			str = str.substr(index + token.size());
// 		}
// 		else
// 		{
// 			result.push_back(str);
// 			str = "";
// 		}
// 	}
// 	return (result);
// }

// int	parse_pass(Client *new_client, std::string s)
// {
// 	std::vector<std::string>	raw_parse;
// 	raw_parse = ft_split(s, " :");
// 	if (raw_parse[1][raw_parse[1].length()-1] == '\n')
// 		raw_parse[1].resize(raw_parse[1].length() - 1); 
// 	if (!new_client->server->getPass().compare(raw_parse[1]))
// 		return(1);
// 	return (0);
// }

// void	parse_nick(Client *new_client, std::string s, std::map<int, Client*> map)
// {
//     std::vector<std::string> raw_nick;
//     raw_nick = ft_split(s, " ");
//     int i = 1;

// 	new_client->setNick(raw_nick[1]);
// 	if(map.size() == 1)
// 		return;
// 	size_t pipe = raw_nick[1].find("|", 0);
// 	if (pipe != std::string::npos) // se troviamo giá una pipe nei due nomi uguali
// 	{
// 		raw_nick[1].resize(pipe);
// 		new_client->setNick(raw_nick[1]);
// 	}
// 	for(std::map<int, Client*>::iterator it = map.begin(); it != map.end(); ++it)
// 	{
// 		if(!it->second->getNick().compare(new_client->getNick()) && new_client->getSd() != it->first) //se troviamo due nomi uguali
// 		{
// 			new_client->setNick(raw_nick[1] + "|" + std::to_string(i));
// 			i++;
// 		}
// 	}
// }

// void	parse_user(Client *new_client, std::string s, std::map<int, Client*> map)
// {
//     std::vector<std::string> raw_user;
//     raw_user = ft_split(s, " ");
//     int i = 1;

// 	new_client->setUser(raw_user[1]);
// 	if(map.size() == 1)
// 		return;
// 	size_t pipe = raw_user[1].find("|", 0);
// 	if (pipe != std::string::npos) // se troviamo giá una pipe nei due nomi uguali
// 	{
// 		raw_user[1].resize(pipe);
// 		new_client->setUser(raw_user[1]);
// 	}
// 	for(std::map<int, Client*>::iterator it = map.begin(); it != map.end(); ++it)
// 	{
// 		if(!it->second->getUser().compare(new_client->getUser()) && new_client->getSd() != it->first) //se troviamo due nomi uguali
// 		{
// 			new_client->setUser(raw_user[1] + "|" + std::to_string(i));
// 			i++;
// 		}
// 	}
// }

// int	parse_info(Client *new_client, char *buffer, int valread, std::map<int, Client*> map)
// {
// 	RepliesCreator				reply;
// 	std::vector<std::string>	raw_parse;
// 	std::string					raw_string(buffer, (size_t)valread);
// 	std::string					sent;

// 	std::cout << raw_string << std::endl;
// 	raw_parse = ft_split(raw_string, "\r\n");
// 	if (!raw_parse[0].compare(0, 5, "PASS :")){
// 		parse_nick(new_client, raw_parse[0], map);
// 		sent.append(reply.makePasswdMisMatch(new_client->getNick()));
// 		send(new_client->getSd(), sent.c_str(), sent.length(), 0);
// 		return (-1);
// 	}
// 	else
// 	{
// 		if (!parse_pass(new_client, raw_parse[0]))
// 		{
// 			if (raw_parse.size() > 1)
// 				parse_nick(new_client, raw_parse[1], map);
// 			sent.append(reply.makePasswdMisMatch(new_client->getNick()));
// 			send(new_client->getSd(), sent.c_str(), sent.length(), 0);
// 			return (-1);
// 		}
// 		if (raw_parse[1].length()){
// 			parse_nick(new_client, raw_parse[1], map);	
// 			parse_user(new_client, raw_parse[2], map);
// 		}
// 	}
// 	new_client->setLogged(true);
// 	sent.append(reply.makeWelcome(new_client->getNick(), new_client->getUser(), "Irc"));
//     sent.append(reply.makeYourHost("Irc", "2.1", new_client->getNick()));
//     sent.append(reply.makeCreated("yesterday", new_client->getNick()));
//     send(new_client->getSd(), sent.c_str(), sent.length(), 0);
// 	return (0);
// }
