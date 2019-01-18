
#include "connection.h"

namespace ZNetworkFramework {

int Connection::getIOThreadNum() {
	return ioThread_->getNumber();
}

bool Connection::notifyIOThread() {
	return ioThread_->notify(this);
}

void Connection::setFlags(short eventFlags) {
	if (eventFlags_ == eventFlags) {
		return;
	}

	if (eventFlags_ != 0) {
		if (event_del(&event_) == -1) {
			GlobalOutput("Connection::setFlags event_del");
			return;
		}
	}

  	eventFlags_ = eventFlags;
	if (!eventFlags_) {
		return;
	}

  	event_set(&event_, fd_, eventFlags_, Connection::eventHandler, this);
  	event_base_set(ioThread_->getEventBase(), &event_);

	if (event_add(&event_, 0) == -1) {
		GlobalOutput("Connection::setFlags(): could not event_add");
	}
}

void Connection::close() {
	if (event_del(&event_) == -1) {
		GlobalOutput.perror("TConnection::close() event_del", errno);
	}
	::close(fd_);
	server_->returnConnection(this);
}

void Connection::transition() {
	
}

}