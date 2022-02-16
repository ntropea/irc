#pragma once
#include "Server.hpp"
#include "Client.hpp"
class	Client;

class	Channel
{
	private:
		std::string				_name;
		std::map<int, Client*>	_clients;
		std::string				_topic;
		std::map<int, Client*>	_mods;
	public:
		Channel() {};
		Channel(std::string name, Client *creator) {
			_name = name;
			_clients.insert(std::make_pair(creator->getSd(), creator));
			_mods.insert(std::make_pair(creator->getSd(), creator));
		};
		~Channel() {};
		std::string getName(){return _name;};
		void		setName(std::string name){ _name = name;};
		std::string getTopic(){return _topic;};
		void		setTopic(std::string topic){ _topic = topic;};
		int			size(){return _clients.size();};
		Client*		getClient(int sd) {return _clients.find(sd)->second;};
};