#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <thread>
#include <vector>
#include <boost/bind.hpp>
#include <common/TOutput.h>
#include "SimpleChatClient.h"

using namespace boost::asio;
using namespace std;
using namespace common;

SimpleChatClient::SimpleChatClient(const std::string& ip, short port)
	:status_(disengaged), server_ip_(ip), server_port_(port), socket_(io_service_) {
	
}

void SimpleChatClient::Run() {
	tcp::endpoint ep(ip::address::from_string(server_ip_), server_port_);
	socket_.connect(ep);

	doMessageHeaderRecv();

	std::thread worker([this](){
		GlobalOutput.printf("thread start\n");
		io_service_.run();
	});
	
	char line[MAX_MESSAGE_SIZE+1];
	while(std::cin.getline(line, MAX_MESSAGE_SIZE+1)) {
		doWriteBuffer(parseCommand(line));
	}

	Stop();
	worker.join();
}

void SimpleChatClient::doMessageHeaderRecv() {
	//GlobalOutput.printf("doMessageHeaderRecv()");
	SharedBuffer buff(HEADER_SIZE);
	async_read(socket_, buff.Buffer(), transfer_all(),
		boost::bind(messageHeaderRecvHandler, this, buff, _1, _2));
}

void SimpleChatClient::messageHeaderRecvHandler(SimpleChatClient *client, SharedBuffer buff, const error_code& err, std::size_t bytes) {
	uint32_t size;
	memcpy(&size, buff.Get(), HEADER_SIZE);
	if(size > MAX_FRAME_SIZE) {
		GlobalOutput.printf("ERROR: packet header size too large\n");
		return;
	}
	client->doMessageBodyRecv(size);
}

void SimpleChatClient::doMessageBodyRecv(uint32_t size) {
	//GlobalOutput.printf("doMessageBodyRecv()");
	SharedBuffer buff(size);
	async_read(socket_, buff.Buffer(), transfer_all(), 
		boost::bind(messageBodyRecvHandler, this, buff, _1, _2));
}

void SimpleChatClient::messageBodyRecvHandler(SimpleChatClient *client, SharedBuffer buff, const error_code& err, std::size_t bytes) {
	uint32_t type;
	memcpy(&type, buff.Get(), TYPE_SIZE);
	std::string data(buff.Get()+TYPE_SIZE, buff.Size()-TYPE_SIZE);
	switch(MessageType(type)) {
		case LOGIN_RSP: {
			LoginResponse lr;
			if(!lr.ParseFromString(data)) {
				GlobalOutput.printf("parse login response error\n");
				break;
			}
			if(lr.code() != SUCCESS) {
				GlobalOutput.printf("login response error: %s\n", lr.message().c_str());
			} else {
				GlobalOutput.printf("Login in ok\n");
			}
			break;
		}
		case LOGOUT_RSP: {
			break;
		}
		case MESSAGESEND_RSP: {
			break;
		}
		case MESSAGERECV: {
			MessageRecv mr;
			if(!mr.ParseFromString(data)) {
				GlobalOutput.printf("parse MessageRecv error\n");
				break;
			}
			time_t tval = time_t(mr.time());
			struct tm *tmval = gmtime(&tval);
			printf("[%02d:%02d:%02d %s] %s\n", tmval->tm_hour, tmval->tm_min, tmval->tm_sec,
					mr.sender().c_str(), mr.message().c_str());
			break;
		}
		default:
			GlobalOutput.printf("invalid message type:%d\n", type);
	}
	client->doMessageHeaderRecv();
}

void SimpleChatClient::doWriteBuffer(SharedBuffer buffer) {
	if(!buffer) return;
	async_write(socket_, buffer.Buffer(), transfer_all(), 
		boost::bind(sendHandler, this, buffer, _1, _2));
}

SharedBuffer SimpleChatClient::packetLoginRequest(const std::string& user, const std::string& room) {
	LoginRequest lr;
	lr.set_user(user);
	lr.set_room(room);
	string data;
	if(!lr.SerializeToString(&data)) {
		GlobalOutput.printf("LoginRequest SerializeToString failed\n");
		exit(1);
	}
	uint32_t framesize = data.size()+TYPE_SIZE;
	uint32_t type = uint32_t(LOGIN_REQ);
	SharedBuffer buff(framesize+HEADER_SIZE);
	memcpy(buff.Get(), &framesize, HEADER_SIZE);
	memcpy(buff.Get()+HEADER_SIZE, &type, TYPE_SIZE);
	memcpy(buff.Get()+HEADER_SIZE+TYPE_SIZE, data.data(), data.size());
	return buff;
}

SharedBuffer SimpleChatClient::packetLogoutRequest(const std::string& user) {
	LogoutRequest lr;
	lr.set_user(user);
	string data;
	if(!lr.SerializeToString(&data)) {
		GlobalOutput.printf("LogoutRequest SerializeToString failed\n");
		exit(1);
	}
	uint32_t framesize = data.size()+TYPE_SIZE;
	uint32_t type = uint32_t(LOGOUT_REQ);
	SharedBuffer buff(framesize+HEADER_SIZE);
	memcpy(buff.Get(), &framesize, HEADER_SIZE);
	memcpy(buff.Get()+HEADER_SIZE, &type, TYPE_SIZE);
	memcpy(buff.Get()+HEADER_SIZE+TYPE_SIZE, data.data(), data.size());
	return buff;
}

SharedBuffer SimpleChatClient::packetMessageSendRequest(const std::string& user, const std::string& room, const std::string& message, uint64_t time) {
	MessageSendRequest msr;
	msr.set_user(user);
	msr.set_room(room);
	msr.set_message(message);
	msr.set_time(time);
	std::string data;
	if(!msr.SerializeToString(&data)) {
		GlobalOutput.printf("MessageSendRequest SerailizeToString failed\n");
		exit(1);
	}
	uint32_t framesize = data.size()+TYPE_SIZE;
	uint32_t type = uint32_t(MESSAGESEND_REQ);
	SharedBuffer buff(framesize+HEADER_SIZE);
	memcpy(buff.Get(), &framesize, HEADER_SIZE);
	memcpy(buff.Get()+HEADER_SIZE, &type, TYPE_SIZE);
	memcpy(buff.Get()+HEADER_SIZE+TYPE_SIZE, data.data(), data.size());
	return buff;
}

enum InputState {command, message};
string get_token(char **input) {
	char *start = *input;
	while(*start != '\n' && *start !='\r' && *start != '\0' &&
		(*start == ' ' || *start == '\t')) start++;
	char *end = start;
	while(*end != '\n' && *end != '\r' && *end != '\0' &&
		*end != ' ' && *end != '\t') end++;
	*input = end;
	return string(start, end-start);
}
SharedBuffer SimpleChatClient::parseCommand(char* input) {
	int count=0;
	InputState state;
	char* p = input;
	if(*p != '!') {
		state = message;
	}
	else {
		state = command;
		count++;
		p++;
	}
	if(state == message) {
		int len = strlen(input);
		if(len == 0) 
			return SharedBuffer(0);
		return packetMessageSendRequest(
			username_, room_, std::string(input, len), time(NULL));
	} 
	vector<string> tokens;
	while(*p != '\n' && *p !='\r' && *p != '\0') {
		tokens.push_back(get_token(&p));
	}
	if(tokens.size() == 0) {
		GlobalOutput.printf("no command input\n");
		return SharedBuffer(0);
	} 
	if(tokens[0] == "login") {
		if(!username_.empty() && !room_.empty()) {
			GlobalOutput.printf("already login, don\'t login again\n");
			return SharedBuffer(0);
		}
		if(tokens.size() != 3) {
			GlobalOutput.printf("login command argument error\n");
			return SharedBuffer(0);
		}
		username_ = tokens[1];
		room_ = tokens[2];
		return packetLoginRequest(username_, room_);
	}
	if(tokens[0] == "logout") {
		if(tokens.size() != 1) {
			GlobalOutput.printf("logout command argument error\n");
			return SharedBuffer(0);
		}
		string name = username_;
		username_ = "";
		room_ = "";
		return packetLogoutRequest(name);
	}
	GlobalOutput.printf("invalid command:%s\n", tokens[0].c_str());
	return SharedBuffer(0);
}	

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("usage: %s, <ip> <port>\n", argv[0]);
		return 0;
	}
	SimpleChatClient client(argv[1], short(atoi(argv[2])));
	client.Run();
	return 0;
}