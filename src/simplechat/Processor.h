#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <string>
#include <memory>
#include "SimpleChat.pb.h"

class Packet;
class SimpleChatServer;
class User;

class Processor {
public:
	explicit Processor(SimpleChatServer *server): server_(server) {}
	void Process(std::shared_ptr<Packet> &packet);

private:
	void processLogin(std::shared_ptr<Packet> &packet);
	void processLogout(std::shared_ptr<Packet> &packet);
	void processSendMessage(std::shared_ptr<Packet> &packet);
	void doCommonResponse(std::shared_ptr<User> user, simplechat::ErrorCode errcode, const std::string &msg);

	SimpleChatServer *server_;
};

#endif