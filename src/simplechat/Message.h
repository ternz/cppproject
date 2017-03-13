#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <memory>
#include <boost/asio.hpp>
#include "UserInfo.h"
#include "Packet.h"

class User;

struct SharedBuffer {
	SharedBuffer(){};
    SharedBuffer(std::shared_ptr<char> data, size_t size): data(data), size(size) {}
	std::shared_ptr<char> data;
	size_t size;
};

class Message {
public:
	Message(){}
	Message(uint32_t type, const std::string &msg, std::shared_ptr<User> user) 
		:user_(user) {
		Fill(type, msg);
	}
	explicit Message(SharedBuffer buffer): buffer_(buffer) {}
	Message(SharedBuffer buffer, std::shared_ptr<User> user): buffer_(buffer), user_(user) {}

	void Fill(uint32_t type, const std::string &msg) {
		buffer_ = MakeSharedBuffer(type, msg);
	}
	
	size_t Size() {return buffer_.size;}

	void SetUser(std::shared_ptr<User> &user) {user_ = user;}
	std::shared_ptr<User> GetUser() {return user_;}

	boost::asio::mutable_buffers_1 Buffer() {
		return boost::asio::buffer(buffer_.data.get(), buffer_.size);
	}
	void SetBuffer(SharedBuffer buffer) {buffer_ = buffer;}

	static SharedBuffer MakeSharedBuffer(uint32_t type, const std::string &msg);
private:
	SharedBuffer buffer_;
	std::shared_ptr<User> user_;
};

#endif