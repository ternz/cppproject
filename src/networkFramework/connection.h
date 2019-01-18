#ifndef Z_CONNECTION_
#define Z_CONNECTION_

#include <event2/event.h>

namespace ZNetworkFramework {

class IOThread;

/// Three states for sockets: recv frame size, recv data, and send mode
enum SocketState { SOCKET_RECV, SOCKET_SEND };

/**
 * Five states for the nonblocking server:
 *  1) initialize
 *  2) read 4 byte frame size
 *  3) read frame of data
 *  4) send back data (if any)
 *  5) force immediate connection close
 */
enum ConnState {
  Conn_INIT,
  Conn_READ_REQUEST,
  Conn_WAIT_TASK,
  Conn_SEND_RESULT,
  Conn_CLOSE_CONNECTION
};

class Connection {
public:
	Connection();
	~Connection();

	int getIOThreadNum();
	bool notifyIOThread();

	void init();

	void transition();

	void close();

private:
  	void setRead() { setFlags(EV_READ | EV_PERSIST); }
  	void setWrite() { setFlags(EV_WRITE | EV_PERSIST); }
 	void setIdle() { setFlags(0); }
  	void setFlags(short eventFlags);

	static void eventHandler(evutil_socket_t fd, short which, void* arg);
  	void workSocket();

private:
	int fd_;
	Server* server_;
	IOThread* ioThread_;

  	struct event event_;
  	short eventFlags_;

	SocketState sockState_;
	ConnState connState_;

	int readWant_;
  	int readBufferPos_;
  	char* readBuffer_;
  	int readBufferSize_;
 	char* writeBuffer_;
  	int writeBufferSize_;
  	int writeBufferPos_;
  	int largestWriteBufferSize_;
  	int callsForResize_;
	
};

}

#endif