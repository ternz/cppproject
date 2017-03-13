#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <glog/logging.h>
#include "Processor.h"
#include "SimpleChat.pb.h"
#include "Packet.h"
#include "SimpleChatServer.h"
#include "UserInfo.h"
#include "Exception.h"

using namespace boost::asio;
using namespace simplechat;

void Processor::Process(std::shared_ptr<Packet>& packet) {
	assert(packet->GetUser()->server == server_);
	switch(packet->GetType()) {
		case LOGIN_REQ:
			processLogin(packet);
			break;
		case LOGOUT_REQ:
			processLogout(packet);
			break;
		case MESSAGESEND_REQ:
			processSendMessage(packet);
			break;
		default:
			doCommonResponse(packet->GetUser(), 
				INVALID_MESSAGE_TYPE, "invalid message type");
			break;
	}
}

void Processor::processLogin(std::shared_ptr<Packet>& packet) {
	LoginRequest login_req;
	if(!login_req.ParseFromString(packet->GetMessageString())) {
		LOG(ERROR)<<"login request parse from string failed";
		doCommonResponse(packet->GetUser(), PARSE_MESSAGE_ERROR, "parse login request message error");
		return;
	}
	if(!packet->GetUser()->name.empty()) {
		LOG(ERROR)<<"user ["<<packet->GetUser()->name<<"] has login already";
		doCommonResponse(packet->GetUser(), USER_ALREADY_LOGIN, "user already login");
		return;
	}
	packet->GetUser()->name = login_req.user();
	packet->GetUser()->room = login_req.room();
	try {
		server_->userinfo_->AddUser(packet->GetUser());
	} catch(EmptyUserNameOrRoom ex) {
		LOG(ERROR)<<"add user exception: "<<ex.what();
		doCommonResponse(packet->GetUser(), INVALID_USER_OR_ROOM, ex.what());
		return;
	}
	LoginResponse login_rsp;
	login_rsp.set_code(SUCCESS);
	login_rsp.set_message("success");
	login_rsp.set_onlines(server_->userinfo_->Onlines(login_req.room()));
	std::string data;
	if(!login_rsp.SerializeToString(&data)) {
		LOG(FATAL)<<"login response serialize to string failed";
	}
	std::shared_ptr<Packet> out_packet(
		new Packet(packet->GetUser(), uint32_t(LOGIN_RSP), data));
	//return out_packet;
	async_write(packet->GetUser()->socket, out_packet->Buffer(), transfer_all(), 
		boost::bind(SimpleChatServer::sendHandler, out_packet, _1, _2));
}

void Processor::processLogout(std::shared_ptr<Packet> &packet) {
	LogoutRequest logout_req;
	if(!logout_req.ParseFromString(packet->GetMessageString())) {
		LOG(ERROR)<<"logout request parse from string failed";
		doCommonResponse(packet->GetUser(), PARSE_MESSAGE_ERROR, "parse logout request message error");
		return;
	}
	if(packet->GetUser()->name != logout_req.user()) {
		LOG(ERROR)<<"invalid logout request user, recevice ["
			<<logout_req.user()<<"] but request ["<<packet->GetUser()->name<<"]";
		doCommonResponse(packet->GetUser(), INVALID_USER_OR_ROOM, "invalid user");
		return;
	}
	packet->GetUser()->status = User::Status::offline;
	server_->userinfo_->RemoveUser(packet->GetUser()->name);
	LogoutResponse logout_rsp;
	logout_rsp.set_code(SUCCESS);
	logout_rsp.set_message("success");
	std::string data;
	if(!logout_rsp.SerializeToString(&data)) {
		LOG(FATAL)<<"logout response serialize to string failed";
	}
	std::shared_ptr<Packet> out_packet(
		new Packet(packet->GetUser(), uint32_t(LOGOUT_RSP), data));

	async_write(packet->GetUser()->socket, out_packet->Buffer(), transfer_all(), 
		boost::bind(SimpleChatServer::sendHandler, out_packet, _1, _2));
}

void Processor::processSendMessage(std::shared_ptr<Packet> &packet) {
	MessageSendRequest ms_req;
	if(!ms_req.ParseFromString(packet->GetMessageString())) {
		LOG(ERROR)<<"message send request parse from string failed";
		doCommonResponse(packet->GetUser(), PARSE_MESSAGE_ERROR, "parse message send request message error");
		return;
	}

	//verify the user and room
	if(ms_req.user() != packet->GetUser()->name ||
		ms_req.room() != packet->GetUser()->room) {
		LOG(ERROR)<<"user["<<packet->GetUser()->name<<"-"<<ms_req.user()<<"] or room ["
			<<packet->GetUser()->room<<"-"<<ms_req.room()<<"] not match";
		doCommonResponse(packet->GetUser(), INVALID_USER_OR_ROOM, "user or room not match");
		return;
	}

	server_->sendMessageToRoom(ms_req);

	MessageSendResponse ms_rsp;
	ms_rsp.set_code(SUCCESS);
	ms_rsp.set_message("success");
	std::string data;
	if(!ms_rsp.SerializeToString(&data)) {
		LOG(FATAL)<<"message send response serialize to string failed";
	}
	std::shared_ptr<Packet> out_packet(
		new Packet(packet->GetUser(), uint32_t(MESSAGESEND_RSP), data));
		
	async_write(packet->GetUser()->socket, out_packet->Buffer(), transfer_all(), 
		boost::bind(SimpleChatServer::sendHandler, out_packet, _1, _2));
}

void Processor::doCommonResponse(std::shared_ptr<User> user, ErrorCode errcode, const std::string &msg) {
	CommonResponse com_rsp;
	com_rsp.set_code(errcode);
	com_rsp.set_message(msg);
	std::string data;
	if(!com_rsp.SerializeToString(&data)) {
		LOG(FATAL)<<"common rsponse serialize to string failed";
	}
	std::shared_ptr<Packet> out_packet(
		new Packet(user, uint32_t(COMMON_RSP), data));
	async_write(user->socket, out_packet->Buffer(), transfer_all(),
		boost::bind(SimpleChatServer::sendHandler, out_packet, _1, _2));
}