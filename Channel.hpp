#pragma once
#include "Server.hpp"

class	Channel
{
	private:
		std::string				_name;
		std::map<int, Client*>	_clients;
		std::string				_topic;
		std::map<int, Client*>	_mods;
		bool					_invite;
	public:
		Channel() {

		};
		Channel(std::string name, Client *creator, bool invite) {
			_invite = invite;
			_name = name;
			_clients.insert(creator);
			_mods.insert(creator);
		};
		~Channel() {};
		std::string getName(){return _name;};
		void		setName(std::string name){ _name = name};
		std::string getTopic(){return _topic;};
		void		setTopic(std::string topic){ _topic = topic};
		int			size(){return _clients.size();};
		Client*		getClient(int sd) {return _clients.find(sd);};
};