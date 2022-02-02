#include "Command.hpp"

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
