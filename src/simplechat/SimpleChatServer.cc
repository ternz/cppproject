#include <stdlib.h>
#include <pthread.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "SimpleChat.pb.h"
#include "SimpleChatServer.h"
#include "UserInfo.h"
#include "Packet.h"
#include "Message.h"
#include "Processor.h"
#include "Exception.h"

using namespace boost::asio;
using namespace google;
using namespace concurrency;
using namespace simplechat;

SimpleChatServer::SimpleChatServer():
	run_(false), 
	threadFactory_(new PosixThreadFactory()), 
	userinfo_(new UserInfo()) {
	//default constrator
	threadFactory_->setDetached(false);
	processor_ = new Processor(this);
}

SimpleChatServer::~SimpleChatServer() {
	if(processor_ != NULL) {
		delete processor_;
	}
}

void SimpleChatServer::Server() {
	run_ = true;

	//install acceptor
	tcp::endpoint ep(tcp::v4(), FLAGS_port);
	acceptor_ = std::make_shared<tcp::acceptor>(io_service_, ep);
	user_ptr new_user = UserInfo::newUser(this);
	acceptor_->async_accept(new_user->socket, boost::bind(acceptHandler, new_user, _1));

	//install signal handler
	boost::asio::signal_set sigset(io_service_, SIGINT, SIGTERM, SIGHUP);
	sigset.async_wait(boost::bind(signalHandler, this, _1, _2));

	//start threads
	for(int i=0; i<FLAGS_threads; i++) {
		auto thread = threadFactory_->newThread(FunctionRunner::create(threadFunc, this));
		thread->start();
		threads_.push_back(thread);
	}
	for(int i=0; i<FLAGS_threads; i++) {
		threads_[i]->join();
	}
	LOG(DEBUG)<<"server stopped";
}

void* SimpleChatServer::threadFunc(void *arg) {
	LOG(DEBUG)<<"thread "<<pthread_self()<<" started";
	SimpleChatServer *server = (SimpleChatServer*)arg;
	//while(server->run_) {
		server->io_service_.run();
	//}
	LOG(DEBUG)<<"thread "<<pthread_self()<<" joined";
}

void SimpleChatServer::signalHandler(SimpleChatServer *server, const boost::system::error_code &err, int signal) {
	if(err != 0) {
		LOG(ERROR)<<"handle signal error: "<<err.message();
		//return;
		exit(1);
	}

	if(signal == SIGHUP) 
		return;
	
	//stop the applicatioin
	LOG(DEBUG)<<"stopping server";
	server->Stop();
}

void SimpleChatServer::acceptHandler(user_ptr user, const boost::system::error_code &err) {
	//TODO: 处理error code
	if(err != 0) {
		LOG(ERROR)<<"handle accept error: "<<err.message();
		return;
	}
	//add user to map
	//user->server->userinfo_->AddUser(user);
	//set asycn_read
	user->status = User::Status::online;
	std::shared_ptr<Packet> packet(new Packet(user));
	async_read(user->socket, packet->Buffer(), transfer_all(), boost::bind(receiveHandler, packet, _1, _2));

	//wait for next conection
	user_ptr new_user = UserInfo::newUser(user->server);
	user->server->acceptor_->async_accept(new_user->socket, boost::bind(acceptHandler, new_user, _1));
}

void SimpleChatServer::receiveHandler(std::shared_ptr<Packet> packet, const error_code& err, std::size_t bytes) {
	//TODO: 处理error code
	switch(packet->Status()) {
		case Packet::reading_header: {
			//可优化为直接读， async_read_some, 若数据不够再async_read
			async_read(packet->GetUser()->socket, packet->Buffer(), transfer_all(), boost::bind(receiveHandler, packet, _1, _2));
			return;
		}
		case Packet::reading_body: {
			//process
			packet->GetUser()->server->processor_->Process(packet);
			return;
		}
		default:
			//正确的代码是不会执行到这里，除非你代码写错了
			LOG(FATAL)<<"error packet status:"<<packet->Status();
			throw LogicalErrorException("you worte wrong code!!");
	}
}

void SimpleChatServer::sendHandler(std::shared_ptr<Packet> packet, const error_code& err, std::size_t bytes) {
	//TODO: 处理error code
	switch(packet->GetUser()->status) {
		case User::online: {
			auto user = packet->GetUser();
			std::shared_ptr<Packet> new_packet(new Packet(user));
			async_read(user->socket, new_packet->Buffer(), transfer_all(), boost::bind(receiveHandler, new_packet, _1, _2));
			return;
		}
		case User::offline:
			return;
	}
}

void SimpleChatServer::sendMessageToRoom(simplechat::MessageSendRequest &msr) {
	sendMessageToRoom(
		msr.user(), 
		msr.room(),
		msr.message(),
		msr.time());
}

void SimpleChatServer::sendMessageToRoom(const std::string &sender, const std::string &room, const std::string &message, uint64_t time) {
	MessageRecv mr;
	mr.set_sender(sender);
	mr.set_room(room);
	mr.set_message(message);
	mr.set_time(time);
	std::string data;
	if(!mr.SerializeToString(&data)) {
		LOG(FATAL)<<"serialize MessageRecv to string failed";
	}
	SharedBuffer buffer = Message::MakeSharedBuffer(MessageType::MESSAGERECV, data);
	try {
		auto userMap = userinfo_->GetUserMap(room);
		for(auto it=userMap.begin(); it != userMap.end(); it++) {
			if(it->first == sender)
				continue;
			Message message(buffer, it->second);
			async_write(it->second->socket, message.Buffer(), transfer_all(), 
				boost::bind(messageSendHandler, message, _1, _2));
		}
	} catch(NoSuchRoomException e) {
		LOG(ERROR)<<"send message to room error: "<<e.what();
		return;
	}
}

void SimpleChatServer::messageSendHandler(Message msg, const error_code& err, std::size_t bytes) {
	//TODO: 处理error code
}