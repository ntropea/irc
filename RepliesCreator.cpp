#include "RepliesCreator.hpp"

RepliesCreator::RepliesCreator()
{
	//std::cout << "RepliesCreator created" << std::endl;
}

RepliesCreator::~RepliesCreator()
{
	//std::cout << "RepliesCreator deleted" << std::endl;
}

/**PUBBLIC-FUNCTIONS**/

std::string RepliesCreator::makeWelcome(std::string CNickname, std::string CUsername, std::string SName)
{
	std::string text;

	text =  "001 " + CNickname + " :Welcome to the" + SName +  \
			"Network, " + CUsername + DEL;
	return (text);
}

std::string RepliesCreator::makeYourHost(std::string Servname, std::string SVersion, std::string CNickname)
{
	std::string text;

	text =  "002 " + CNickname + " :Your host is " + Servname + \
			", running version " + SVersion + DEL;
	return (text);
}

std::string RepliesCreator::makeCreated(std::string SDate, std::string CNick)
{
	std::string text;

	text = "003 " + CNick + " :This server was created " + SDate + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorNoSuchNick(std::string Name)
{
	std::string text;

	text = "401 " + Name+  " :No such nick/channel" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorNoSuchChannel(std::string ChName)
{
	std::string text;

	text = "403 " + ChName + " :No such channel" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorCannotSentToChan(std::string ChName)
{
	std::string text;

	text = "404 " + ChName + " :Cannot send to channel" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorTooManyChannels(std::string Name)
{
	std::string text;

	text = "405 " + Name + " :You have joined too many channels" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorNoOrigin(std::string Nick)
{
	std::string text;

	text = "409 " + Nick + " :No origin specified" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorUnknownCommand(std::string command)
{
	std::string text;

	text = "421 " + command + " :Unknown command" + DEL;
	return text;
}


std::string RepliesCreator::makeErrorNoNickNameGiven(std::string CNick)
{
	std::string text;

	text = "431 " + CNick + " :No nickname given" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorErroneusNickName(std::string CNick, std::string nick)
{
	std::string text;

	text = "432 " + CNick + " " + nick + " :Erroneus nickname" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorNickNameInUse(std::string CNick, std::string nick)
{
	std::string text;

	text = "433 " + CNick + " " + nick + " :Nickname is already in use" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorUserNotInChannel(std::string nick, std::string channel)
{
	std::string text;

	text = "441 " + nick + " " + channel + " :They aren't on that channel" + DEL;
	return text;
}

std::string RepliesCreator::makeErrorNotOnChannel(std::string ChName)
{
	std::string text;

	text = "442 " + ChName + " :You're not on that channel" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorUserOnChannel(std::string nick, std::string channel)
{
	std::string text;

	text = "443 " + nick + " " + channel + " :is already on channel" + DEL;
	return text;
}

std::string RepliesCreator::makeErrorNeedMoreParams(std::string CNick, std::string command)
{
	std::string text;

	text = "461 " + CNick + " " + command + " :Not enough parameters" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorAlreadyRegistered(std::string CNick)
{
	std::string text;

	text = "462 " + CNick + " :You may not reregister" + DEL;
	return (text);
}

std::string RepliesCreator::makePasswdMisMatch(std::string CNick)
{
	std::string text;

	text = "464 " + CNick + " :Password incorrect" + DEL;
	return (text);
}

std::string RepliesCreator::makeErrorNoPrivileges(std::string CNick)
{
	std::string text;

	text = "481 " + CNick + " :Permission Denied- You-re not an IRC operator" + DEL;
	return text;
}

std::string RepliesCreator::makeErrorUModeUnknownFlag(std::string CNick)
{
	std::string text;

	text = "501 " + CNick + " :Unknown mode char" + DEL;
	return text;
}
