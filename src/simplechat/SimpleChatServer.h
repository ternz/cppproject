#ifndef __SIMPLECHATSERVER_H__
#define __SIMPLECHATSERVER_H__

#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

#include <concurrency/concurrency.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "SimpleChat.pb.h"


DECLARE_int32(port);
DECLARE_uint32(threads);

using boost::asio::ip::tcp;
using boost::system::error_code;

class UserInfo;
class User;
class Packet;
class Message;
class Processor;

class SimpleChatServer {
public:
	typedef std::shared_ptr<tcp::socket> socket_ptr;

    SimpleChatServer();
	~SimpleChatServer();

    void Server();

	void Stop() {
		if(run_ == false) return;
		run_ = false;
		acceptor_->close();
		io_service_.stop();
	}

	friend class User;
	friend class Processor;
private:
	static void signalHandler(SimpleChatServer *server, const error_code &err, int signal);
    static void acceptHandler(std::shared_ptr<User> user, const error_code &err);
	static void receiveHandler(std::shared_ptr<Packet> packet, const error_code& err, std::size_t bytes);
	static void sendHandler(std::shared_ptr<Packet> packet, const error_code& err, std::size_t bytes);
	static void* threadFunc(void *arg);

	//void responseLogin(const std::string &name);
	//void responseLogin(std::shared_ptr<User> user);
	//void responseSendMessage(std::shared_ptr<User> user, std::shared_ptr<Packet> packet);
	void sendMessageToRoom(simplechat::MessageSendRequest &msr);
	void sendMessageToRoom(const std::string &sender, const std::string &room, const std::string &message, uint64_t time);
	static void messageSendHandler(Message msg, const error_code& err, std::size_t bytes);
	//void responseLogout(const std::string &name);
	//void responseLogout(std::shared_ptr<User> user);
	//void noticeSomeoneEnter();
	//void noticeSomeoneLeave();
	//void responseListRoom();
private:
	bool run_;
    boost::asio::io_service io_service_;
	std::shared_ptr<tcp::acceptor> acceptor_;
    std::shared_ptr<UserInfo> userinfo_;

	boost::shared_ptr<concurrency::PosixThreadFactory> threadFactory_;
	std::vector<boost::shared_ptr<concurrency::Thread> > threads_;

	Processor *processor_;
};

#endif