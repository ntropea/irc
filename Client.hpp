#pragma once
#include <iostream>
class Server;

class Client	{
	private:
		std::string _username;
		std::string _nickname;
		int			_sd;
		bool		_logged;
	public:
		Server *server;
		Client() {_logged = false;};
		~Client() {};
		std::string getNick(){return(this->_nickname);};
		std::string getUser(){return(this->_username);};
		bool		getLog(){return(this->_logged);};
		int			getSd(){return(this->_sd);};
		void		setNick(std::string str){this->_nickname = str;};
		void		setUser(std::string str){this->_username = str;};
		void		setSd(int n){this->_sd = n;};
		void		setLogged(bool var){this->_logged = var;};
		void		setRandomClient()
		{
			int i = rand() % 9999 + 1;
			_nickname.append("nick#" + std::to_string(i));
			_username.append("user#" + std::to_string(i));
			//send(_sd, "You can change your NICKNAME by using \"NICK\" followed by your preference\n", 74, 0);
		}
};

