#pragma once
#include "Server.hpp"
#include "Client.hpp"
class	Client;

class	Channel
{
	private:
		std::string					_name;
		std::map<int, Client*>		_clients;
		std::vector<Client*>		_modClients;
		std::vector<Client*>		_halfModClients;
		std::vector<Client*>		_voiceClients;
		std::string					_topic;
		int							_timeCreate;
		int							_topicTime;
		std::string					_topicChanger;
		bool						_allBanned;
		typedef struct Banned
		{
			std::string	name;
			std::string nickMod;
			std::string userMod;
			int			ban_time;
		}	Banned;
		std::vector<Banned>			_banned;
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
		void					setTopic(std::string topic) {_topic = topic;};
		int						size(){return _clients.size();};
		Client*					getClient(int sd) {return _clients.find(sd)->second;};
		std::map<int, Client*>	getClientMap() {return _clients;};
		std::vector<Client*>	getModClients() { return _modClients;}
		std::vector<Banned>		getBanned() { return _banned;}	
		bool					allBanned() { return _allBanned; }
		void					setAllBanned(bool ban) { _allBanned = ban; }
		void					insert(Client *client){_clients.insert(std::make_pair(client->getSd(), client));};
		void					erase(Client *client){
			_clients.erase(client->getSd());
			modErase(client);
			halfModErase(client);
			voiceErase(client);
		};
		int						getTopicTime() {return _topicTime;};
		void					setTopicTime() {struct timeval time; gettimeofday(&time, NULL); _topicTime = time.tv_sec;};
		std::string				getTopicChanger() {return _topicChanger;};
		void					setTopicChanger(std::string nick) {_topicChanger = nick;};
		void					modInsert(Client *client){ _modClients.push_back(client);};
		void					modErase(Client *client)
		{
			std::vector<Client *>::iterator it = _modClients.end();
			for(std::vector<Client *>::iterator i = _modClients.begin(); i != _modClients.end(); i++)
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
		void					halfModInsert(Client *client){ _halfModClients.push_back(client);};
		void					halfModErase(Client *client)
		{
			std::vector<Client *>::iterator it = _halfModClients.end();
			for(std::vector<Client *>::iterator i = _halfModClients.begin(); i != _halfModClients.end(); i++)
			{
				if (client->getSd() == (*(*i)).getSd())
				{
					it = i;
					break;
				}
			}
			if(it != _halfModClients.end())
				_halfModClients.erase(it);
		}
		void					voiceInsert(Client *client){ _voiceClients.push_back(client);};
		void					voiceErase(Client *client)
		{
			std::vector<Client *>::iterator it = _voiceClients.end();
			for(std::vector<Client *>::iterator i = _voiceClients.begin(); i != _voiceClients.end(); i++)
			{
				if (client->getSd() == (*(*i)).getSd())
				{
					it = i;
					break;
				}
			}
			if(it != _voiceClients.end())
				_voiceClients.erase(it);
		}
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
		int						checkClient(Client *client)
		{
			std::map<int, Client*>::iterator it = _clients.find(client->getSd());
			if (it != _clients.end())
				return(1);
			return(0);
		}
		int						bannedFind(std::string ban)
		{
			for (int i = 0; i < _banned.size(); i++)
			{
				if (!_banned[i].name.compare(ban))
					return (i);
			}
			return (-1);
		}
		void					bannedInsert(std::string banName, std::string banNickMod, std::string banUserMod)
		{
			Banned	inserted;
			struct timeval time; 
			gettimeofday(&time, NULL); 
			inserted.ban_time = time.tv_sec;
			inserted.name = banName;
			inserted.nickMod = banNickMod;
			inserted.userMod = banUserMod;
			_banned.push_back(inserted);
		}
		void					bannedErase(int pos)
		{
			std::vector<Banned>::iterator it = _banned.begin();
			if (pos != -1)
			{
				for (int i = 0; i < pos; i++)
					it++;
				_banned.erase(it);
			}
		}
};