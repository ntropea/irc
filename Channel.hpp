#pragma once
#include "Server.hpp"
#include "Client.hpp"
class	Client;

class	Channel
{
	private:
		std::string				_name;
		std::map<int, Client*>	_clients;
		std::vector<Client*>	_modClients;
		std::string				_topic;
		int						_timeCreate;
	public:
		Channel() {};
		Channel(std::string name, Client *creator) {
			_name = name;
			_clients.insert(std::make_pair(creator->getSd(), creator));
			_modClients.push_back(creator);
			_timeCreate = retTime();
		};
		~Channel() {};
		std::string 			getName(){return _name;};
		void					setName(std::string name){ _name = name;};
		std::string 			getTopic(){return _topic;};
		void					setTopic(std::string topic){ _topic = topic;};
		int						size(){return _clients.size();};
		Client*					getClient(int sd) {return _clients.find(sd)->second;};
		void					insert(Client *client){_clients.insert(std::make_pair(client->getSd(), client));};
		void					modInsert(Client *client){ _modClients.push_back(client);};
		void					modErase(Client *client)
		{
			std::vector<Client *>::iterator it = _modClients.end();
			for(std::vector<Client *>::iterator i = _modClients.begin(); i != _modClients.end(); i++ )
			{
				if (client->getSd() == (*(*i)).getSd())
				{
					it = i;
					break;
				}
			}
			if(it != _modClients.end())
				_modClients.erase(it);
		}
		std::map<int, Client*>	getClientMap() {return _clients;};
		int 					retTime() {struct timeval time; gettimeofday(&time, NULL); return(time.tv_sec);};
		int						getTime() {return _timeCreate;};
		int						isMod(Client *client)
		{
			for(int i = 0; i < _modClients.size(); i++)
			{
				if (client->getSd() == _modClients[i]->getSd())
					return (1);
			}
			return (0);
		}

		
};