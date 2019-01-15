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
	int fd = getNotificationSendFD();
  	if (fd < 0) {
    	return false;
  	}

	fd_set wfds, efds;
	int ret = -1;
	int kSize = sizeof(conn);
	const char* pos = (const char*)const_cast_sockopt(&conn);

  	while (kSize > 0) {
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(fd, &wfds);
		FD_SET(fd, &efds);
		ret = select(fd + 1, NULL, &wfds, &efds, NULL);
		if (ret < 0) {
			return false;
		} else if (ret == 0) {
			continue;
		}
		if (FD_ISSET(fd, &efds)) {
			::close(fd);
			return false;
		}
    	if (FD_ISSET(fd, &wfds)) {
			ret = send(fd, pos, kSize, 0);
			if (ret < 0) {
				if (errno == EAGAIN) {
					continue;
				}
				::close(fd);
				return false;
			}
			kSize -= ret;
			pos += ret;
    	}
  	}
  	return true;
}

void IOThread::notificationHandler(evutil_socket_t fd, short which, void * arg) {
	IOThread* ioThread = (IOThread*)v;
  assert(ioThread);
  (void)which;

  while (true) {
    TNonblockingServer::TConnection* connection = 0;
    const int kSize = sizeof(connection);
    long nBytes = recv(fd, cast_sockopt(&connection), kSize, 0);
    if (nBytes == kSize) {
      if (connection == NULL) {
        // this is the command to stop our thread, exit the handler!
        return;
      }
      connection->transition();
    } else if (nBytes > 0) {
      // throw away these bytes and hope that next time we get a solid read
      GlobalOutput.printf("notifyHandler: Bad read of %d bytes, wanted %d", nBytes, kSize);
      ioThread->breakLoop(true);
      return;
    } else if (nBytes == 0) {
      GlobalOutput.printf("notifyHandler: Notify socket closed!");
      // exit the loop
      break;
    } else { // nBytes < 0
      if (THRIFT_GET_SOCKET_ERROR != THRIFT_EWOULDBLOCK
          && THRIFT_GET_SOCKET_ERROR != THRIFT_EAGAIN) {
        GlobalOutput.perror("TNonblocking: notifyHandler read() failed: ", THRIFT_GET_SOCKET_ERROR);
        ioThread->breakLoop(true);
        return;
      }
      // exit the loop
      break;
    }
  }
}

}