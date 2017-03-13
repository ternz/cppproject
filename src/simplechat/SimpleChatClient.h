#ifndef __SIMPLECHATCLIENT_H__
#define __SIMPLECHATCLIENT_H__

#include <boost/asio.hpp>
#include <memory>
using boost::asio::ip::tcp;
using boost::system::error_code;

#include "SimpleChat.pb.h"
using namespace simplechat;

#define HEADER_SIZE 4
#define TYPE_SIZE 4
#define MAX_PACKET_SIZE 65536
#define MAX_FRAME_SIZE MAX_PACKET_SIZE-HEADER_SIZE

#define MAX_MESSAGE_SIZE 255

struct SharedBuffer {
public:
	explicit SharedBuffer(size_t size): size_(size),
		data_(std::shared_ptr<char>(new char[size], [](char *p){delete[] p;})) {
		if(size != 0) {
			memset(data_.get(), 0, size);
		}
	}
	char* Get() {return data_.get();}
	size_t Size() {return size_;}
	boost::asio::mutable_buffers_1 Buffer() {
		return boost::asio::buffer(Get(), Size());
	}
	operator bool() const {
		if(size_) return true;
		return false;
	}
private:
	std::shared_ptr<char> data_;
	size_t size_;
};

class SimpleChatClient {
public:
	enum Status {disengaged, joined};
private:
	Status status_;

	SharedBuffer parseCommand(char* input);
	void doMessageHeaderRecv();
	void doMessageBodyRecv(uint32_t size);
	void doWriteBuffer(SharedBuffer buffer);

	static void messageHeaderRecvHandler(SimpleChatClient *client, SharedBuffer buff, const error_code& err, std::size_t bytes);
	static void messageBodyRecvHandler(SimpleChatClient *client, SharedBuffer buff, const error_code& err, std::size_t bytes);
	static void sendHandler(SimpleChatClient *client, SharedBuffer buff, const error_code& err, std::size_t bytes){}

	static SharedBuffer packetLoginRequest(const std::string& user, const std::string& room);
	static SharedBuffer packetLogoutRequest(const std::string& user);
	static SharedBuffer packetMessageSendRequest(const std::string& user, const std::string& room, const std::string& message, uint64_t time);

	// void unpacketLoginResponse(const char* ptr, int len, LoginResponse* lr) {
	// 	unpacketLoginResponse(string(ptr, len), lr);
	// }
	// void unpacketLogoutResponse(const char* ptr, int len, LogoutResponse* lr) {
	// 	unpacketLogoutResponse(string(ptr, len), lr);
	// }
	// void unpacketMessageSendResponse(const char* ptr, int len, MessageSendResponse* msr) {
	// 	unpacketMessageSendResponse(string(ptr, len), msr);
	// }
	// void unpacketMessageRecv(const char* ptr, int len, MessageRecv* mr) {
	// 	unpacketMessageRecv(string(ptr, len), mr);
	// }

	// void unpacketLoginResponse(const std::string& data, LoginResponse* lr);
	// void unpacketLogoutResponse(const std::string& data, LogoutResponse* lr);
	// void unpacketMessageSendResponse(const std::string& data, MessageSendResponse* msr);
	// void unpacketMessageRecv(const std::string& data, MessageRecv* mr);

	//static const char* timeStr(time_t t);
	
public:
	SimpleChatClient(const std::string& ip, short port);

	Status Status() {return status_;}
	void Run();
	void Stop(){socket_.close();io_service_.stop();}

private:
	boost::asio::io_service io_service_;
	tcp::socket socket_;
	std::string server_ip_;
	short server_port_;

	std::string username_;
	std::string room_;
};

#endif