#ifndef __PACKET_H__
#define __PACKET_H__

#include <memory>
#include <boost/asio.hpp>
#include <common/common.h>
//#include "UserInfo.h"

#define HEADER_SIZE 4
#define TYPE_SIZE 4
#define MAX_PACKET_SIZE 65536
#define MAX_FRAME_SIZE MAX_PACKET_SIZE-HEADER_SIZE

class User;

class Packet {
public:
	enum Status {none, reading_header, reading_body, complete, writing};

// 必须放在成员函数Status()前面，否则编译错误
private:
	Status status_;

public:
	// const int HEADER_SIZE = 4;
	// const int TYPE_SIZE = 4;
	// const int MAX_PACKET_SIZE = 65536;
	// const int MAX_FRAME_SIZE = MAX_PACKET_SIZE - HEADER_SIZE;

	Packet();
	explicit Packet(std::shared_ptr<User> user);
	Packet(std::shared_ptr<User> user, uint32_t type, const std::string &data);
	~Packet();

	Status Status() {return status_;}
	
	bool IsComplete() { return (Status() == complete);}
	explicit operator bool() {return IsComplete();}

	//size_t RequireSize();
	boost::asio::mutable_buffers_1 Buffer();
	void PrepareForReadingBody();
	//void AddToDataSize(int size) {data_size_ += size;}
	//void Forward(int step) {pos_ += step;}

	void FillDataForWriting(char *data, std::size_t size);

	uint32_t GetType();
	char* GetMessage();
	size_t GetMessageSize();
	std::string GetMessageString();

	void SetUser(std::shared_ptr<User> user) {user_ = user;}
	std::shared_ptr<User> GetUser() {return user_;}

private:

	void init() {
		status_ = none;
		type_ = 0; 
		data_ = NULL;
		memset(header_.data, 0, HEADER_SIZE);
	}

	union {
		uint32_t value; 
		char data[HEADER_SIZE];
	} header_;
	uint32_t type_;
	char *data_;
	//int data_size_;
	//char *pos_;
	std::shared_ptr<User> user_;
};

#endif