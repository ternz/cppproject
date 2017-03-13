#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <string>
#include <common/common.h>

using common::TException;

class NoSuchUserException: public TException {
public:
	NoSuchUserException() :TException("exception: no such user") {}
	NoSuchUserException(const std::string &user) :TException("") {
		message_ += "exception: no such user:" + user;
	}
};

class LogicalErrorException: public TException {
public:
	LogicalErrorException() :TException("exception: logical error") {}
	LogicalErrorException(const std::string &message) :TException(message) {}
};

class PacketHeaderValueToLargeException: public TException {
public:
	PacketHeaderValueToLargeException(): TException("exception: packet header value to large") {}
	PacketHeaderValueToLargeException(uint32_t value) :TException("") {
		message_ += "exception: packet header value to large [";
		message_ += value;
		message_ +="]";
	}
};

class EmptyUserNameOrRoom: public TException {
public:
	EmptyUserNameOrRoom(): TException("empty user name or room") {}
	EmptyUserNameOrRoom(const std::string &name, const std::string &room): TException("") {
		message_ += "empty user name:[";
		message_ += name;
		message_ += "] or room:[";
		message_ += room;
		message_ += "]";
	}
};

class NoSuchRoomException: public TException {
public:
	NoSuchRoomException(): TException("no such room") {}
	NoSuchRoomException(const std::string& room): TException("") {
		message_ += "no such room [";
		message_ += room;
		message_ += "]";
	}
};

#endif