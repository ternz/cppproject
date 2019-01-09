#include <event2/event.h>
#include "iothread.h"

namespace ZNetworkFramework {

void IOThread::registerListenEvent(int listenfd) {
	if(listenfd_ >= 0) {
		GlobalOutput.printf("IO thread listen socket had registered.");
		return;
	}
	assert(evbase_ != NULL && listenfd >= 0);
	listenfd_ = listenfd;
	event_set(&listenEvent_, 
			listenfd_, 
			EV_READ | EV_PERSIST, 
			IOThread::listenHandler, 
			server_);
	event_base_set(evbase_, &listenEvent_);
	if(event_add(&listenEvent_, 0) < 0) {
		throw TException("event_add() failed on server listen event");
	}
	GlobalOutput.printf("IO thread #%d registered for listen.", id_);
}

void IOThread::registerNotificationEvent() {
	if(notificationPipeFDs_[0] >= 0 && notificationPipeFDs_[1] >= 0) {
		GlobalOutput.printf("IO thread notification event had registered.");
		return;
	}
	createNotificationPipe();
	event_set(&notificationEvent_, 
			getNotificationRecvFD(),
			EV_READ | EV_PERSIST, 
			IOThread::notificationHandler,
			this);
	event_base_set(evbase_, notificationEvent_);
	if(event_add(&notificationEvent_, 0) < 0) {
		throw TException("event_add() failed on iothread notification event");
	}
}

void IOThread::createNotificationPipe() {
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, notificationPipeFDs_) == -1) {
    	GlobalOutput.perror("IOThread::createNotificationPipe ", errno);
    	throw TException("can't create notification pipe");
  	}
	if (set_nonblocking(notificationPipeFDs_[0]) < 0
		|| set_nonblocking(notificationPipeFDs_[1]) < 0) {
		close(notificationPipeFDs_[0]);
		close(notificationPipeFDs_[1]);
		throw TException("IOThread::createNotificationPipe() set O_NONBLOCK");
	}
	for (int i = 0; i < 2; ++i) {
		if(set_close_on_exec(notificationPipeFDs_[i]) == -1) {
			close(notificationPipeFDs_[0]);
			close(notificationPipeFDs_[1]);
			throw TException(
				"IOThread::createNotificationPipe() set FD_CLOEXEC");
		}
  	}
}

bool IOThread::notify(Connection* conn) {

}

void IOThread::notificationHandler(evutil_socket_t fd, short which, void * arg) {

}

}