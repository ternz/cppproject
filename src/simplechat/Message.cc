#include <cstring>
#include <string>
#include "Message.h"

SharedBuffer Message::MakeSharedBuffer(uint32_t type, const std::string &msg) {
	size_t data_size = HEADER_SIZE + TYPE_SIZE + msg.size();
	std::shared_ptr<char> data(new char[data_size], [](char* p){delete[] p;});
	char *ptr = data.get();
	uint32_t framesize = data_size - HEADER_SIZE;
	memcpy(ptr, &framesize, HEADER_SIZE);
	memcpy(ptr+HEADER_SIZE, &type, TYPE_SIZE);
	memcpy(ptr+HEADER_SIZE+TYPE_SIZE, msg.c_str(), msg.size());

	return SharedBuffer(data, data_size);
}

