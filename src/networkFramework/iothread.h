#ifndef Z_IO_THREAD_
#define Z_IO_THREAD_

#include <event2/event.h>

namespace ZNetworkFramework {

class Server;
class Connection;

class IOThread {
public:
	IOThread();
	~IOThread();

	int getNumber() {return number_;}

	void registerListenEvent(int listenfd);
	void registerNotificationEvent();

	int getNotificationSendFD() {return notificationEvent_[1];}
	int getNotificationRecvFD() {return notificationEvent_[0];}

	bool notify(Connection* conn);

private:
	static void listenHandler(evutil_socket_t fd, short which, void * arg);
	static void notificationHandler(evutil_socket_t fd, short which, void * arg);

	void createNotificationPipe();

private:
	int number_;
	int listenfd_;
	Server* server_;
	event_base* evbase_;
    struct event listenEvent_;
    struct event notificationEvent_;
    int notificationPipeFDs_[2];
};

}

#endif