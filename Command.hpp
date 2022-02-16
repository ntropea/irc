#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "RepliesCreator.hpp"

void	userCmd();
void	pingCmd();
void	joinCmd(Client *client, std::vector<std::string> splitted)
{
	RepliesCreator reply;
	std::string msg;
	if (splitted[1][0] != '#')
	{
		if (splitted[1][splitted[1].length() - 1] == '\n')
			splitted[1].pop_back();
		msg.append(reply.makeErrorNoSuchChannel(client->getNick(), splitted[1]));
		send(client->getSd(), msg.c_str(), msg.length(), 0);
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
