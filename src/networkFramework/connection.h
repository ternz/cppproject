#ifndef Z_CONNECTION_
#define Z_CONNECTION_

#include <event2/event.h>

namespace ZNetworkFramework {

class IOThread;

/// Three states for sockets: recv frame size, recv data, and send mode
enum SocketState { SOCKET_RECV_FRAMING, SOCKET_RECV, SOCKET_SEND };

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
  Conn_READ_FRAME_SIZE,
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

  	void workSocket();

private:
	IOThread* ioThread_;
};

}

#endif