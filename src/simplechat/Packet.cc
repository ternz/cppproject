#include <string>
#include <cstring>
#include <boost/asio.hpp>

#include "Exception.h"
#include "Packet.h"

using namespace boost::asio;
using namespace std;

Packet::Packet() {
	init();
}

Packet::Packet(std::shared_ptr<User> user) {
	init();
	user_ = user;
}

Packet::Packet(std::shared_ptr<User> user, uint32_t type, const std::string &data) {
	user_ = user;
	status_ = writing;
	type_ = 0; 
	header_.value = TYPE_SIZE + data.size();
	data_ = new char[HEADER_SIZE + TYPE_SIZE + data.size()];
	memcpy(data_, header_.data, HEADER_SIZE);
	memcpy(data_+HEADER_SIZE, &type, TYPE_SIZE);
	memcpy(data_+HEADER_SIZE+TYPE_SIZE, data.c_str(), data.size());
}

Packet::~Packet() {
	if(data_ != NULL)
		delete [] data_;
}

// size_t Packet::RequireSize() {
// 	switch(status_) {
// 		case none:
// 		case complete:
// 			return 0;
// 		case reading_header:
// 			return HEADER_SIZE;
// 		case reading_body:
// 		//todo
// 		case writing:
// 		//todo
// 	}
// }

boost::asio::mutable_buffers_1 Packet::Buffer() {
	switch(status_) {
		case none: 
			status_ = reading_header;
			return buffer(header_.data, HEADER_SIZE);
		case reading_header:
			status_ = reading_body;
			PrepareForReadingBody();
			return buffer(data_+HEADER_SIZE, header_.value);
		case reading_body:
			return buffer(data_+HEADER_SIZE, header_.value);
		case writing:
			return buffer(data_, header_.value + HEADER_SIZE);
		default:
			throw LogicalErrorException("wrong way invaking function Packet::Buffer");
	}
}

void Packet::PrepareForReadingBody() {
	if(header_.value > MAX_FRAME_SIZE) 
		throw PacketHeaderValueToLargeException(header_.value);
	data_ = new char[HEADER_SIZE + header_.value];
	//pos_ = data_ + HEADER_SIZE;
}

void Packet::FillDataForWriting(char *data, std::size_t size) {
	//
}

uint32_t Packet::GetType() {
	memcpy(&type_, data_+HEADER_SIZE, TYPE_SIZE);
	return type_;
}

char* Packet::GetMessage() {
	return data_ + HEADER_SIZE + TYPE_SIZE;
}

size_t Packet::GetMessageSize() {
	return header_.value - TYPE_SIZE;
}

string Packet::GetMessageString() {
	return string(GetMessage(), GetMessageSize());
}