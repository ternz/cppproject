#ifndef __USERINFO_H__
#define __USERINFO_H__

#include <map>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <concurrency/concurrency.h>

//#include "SimpleChatServer.h"

class SimpleChatServer;

class User {
public:
	enum Status {offline, online};
	explicit User(SimpleChatServer *server);
    std::string name;
    std::string room;
    Status status;
	boost::asio::ip::tcp::socket socket;
	SimpleChatServer *server;
};

typedef std::shared_ptr<User> user_ptr;
typedef std::map<std::string, user_ptr> user_map;

class UserInfo {
public:
	//typedef std::shared_ptr<ip::tcp::socket> socket_ptr;

	static user_ptr newUser(SimpleChatServer *server);
    user_ptr GetUser(const std::string &name);
	//void AddNewConection(user_ptr &user);
    void AddUser(user_ptr user);
    void RemoveUser(const std::string &name);
	int Onlines(const std::string &room);

	user_map GetUserMap(const std::string& room);
private:
	void removeUserNoLock(const std::string &name);

    std::map<std::string, std::map<std::string, user_ptr > > rooms_;
    std::map<std::string, std::string> users_room_;

    concurrency::Mutex mutex;
};

#endif