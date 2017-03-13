#include <boost/asio.hpp>
#include <common/common.h>
#include "UserInfo.h"
#include "Exception.h"
#include "SimpleChatServer.h"

using namespace std;
using namespace concurrency;
using namespace common;

User::User(SimpleChatServer *server)
	:server(server), socket(server->io_service_), status(offline) {
	
}

user_ptr UserInfo::newUser(SimpleChatServer *server) {
	return make_shared<User>(server);
}

user_ptr UserInfo::GetUser(const std::string &name) {
	Guard g(mutex);
	//shared_ptr<User> user_ptr;
	auto it = users_room_.find(name);
	if(it == users_room_.end()) {
		throw NoSuchUserException(name);
	}
	return rooms_[it->second][name];
}

void UserInfo::AddUser(user_ptr user) {
	if(user->name.empty() || user->room.empty()) {
		throw EmptyUserNameOrRoom(user->name, user->room);
	}
	Guard g(mutex);
	removeUserNoLock(user->name);

	auto it = rooms_.find(user->room);
	if(it == rooms_.end()) {
		map<string, user_ptr> userMap;
		userMap.insert(make_pair(user->name, user));
		//rooms_[user->room] = userMap;
		rooms_.insert(make_pair(user->room, userMap));
	} else {
		it->second.insert(make_pair(user->name, user));
	}
	users_room_[user->name] = user->room;
}

void UserInfo::removeUserNoLock(const std::string &name) {
	auto it_r = users_room_.find(name);
	if(it_r == users_room_.end())
		return;
	auto userMap = rooms_[it_r->second];
	users_room_.erase(it_r);
	auto it_u = userMap.find(name);
	if(it_u == userMap.end()) {
		return;
	} 
	userMap.erase(it_u);
}

void UserInfo::RemoveUser(const std::string &name) {
	Guard g(mutex);
	removeUserNoLock(name);
}

int UserInfo::Onlines(const std::string &room) {
	Guard g(mutex);
	auto it = rooms_.find(room);
	if(it == rooms_.end()) 
		return 0;
	return int(it->second.size());
}

user_map UserInfo::GetUserMap(const std::string& room) {
	auto it = rooms_.find(room);
	if(it == rooms_.end()) {
		throw NoSuchRoomException(room);
	}
	return it->second;
}