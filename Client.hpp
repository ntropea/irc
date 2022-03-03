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
		~Client();
		std::string getNick(){return(this->_nickname);};
		std::string getUser(){return(this->_username);};
		bool		getLog(){return(this->_logged);};
		int			getSd(){return(this->_sd);};
		void		setNick(std::string str){this->_nickname = str;};
		void		setUser(std::string str){this->_username = str;};
		void		setSd(int n){this->_sd = n;};
		void		setLogged(bool var){this->_logged = var;};
};