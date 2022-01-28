#pragma once

#include <iostream>

#define DEL "\n"
#define DELSIZE 1

class RepliesCreator
{
	public:
		RepliesCreator();
		~RepliesCreator();

		std::string makeWelcome(std::string CNickname, std::string CUsername, std::string SName);
		std::string makeYourHost(std::string Servname, std::string SVersion, std::string CNickname);
		std::string makeCreated(std::string SDate, std::string CNick);

		std::string makeErrorNeedMoreParams(std::string CNick, std::string command);
		std::string makeErrorAlreadyRegistered(std::string CNick);
		std::string makeErrorNoNickNameGiven(std::string CNick);
		std::string makeErrorErroneusNickName(std::string CNick, std::string nick);
		std::string makeErrorNickNameInUse(std::string CNick, std::string nick);
		std::string makePasswdMisMatch(std::string CNick);
	private:
};