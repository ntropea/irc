#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "RepliesCreator.hpp"

void	userCmd();
void	pingCmd();
void	joinCmd();
void	nickCmd();
void	quitCmd(Client *client, std::vector<std::string> splitted);
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
