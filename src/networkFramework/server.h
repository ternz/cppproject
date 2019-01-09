#ifndef NF_SERVER_ 
#define NF_SERVER_

#include <vector>
#include <memory>
#include <event2/event.h>
#include "concurrency/Mutex.h"

namespace ZNetworkFramework {

/// Overload condition actions.
enum OverloadAction {
  OVERLOAD_NO_ACTION,       ///< Don't handle overload */
  OVERLOAD_CLOSE_ON_ACCEPT, ///< Drop new connections immediately */
  OVERLOAD_DRAIN_TASK_QUEUE ///< Drop some tasks from head of task queue */
};

class IOThread;

class Server {
public:

	Server(int port);
	~Server();

	void server();
	void asyncServer();
	//void setPort(int port) {port_ = port;}
	int getPort() {return port_;}
	int getIOThreadNum() {return ioThreadNum_;}
	void setIOThreadNum(int num) {
		if(!serving_) {
			ioThreadNum_ = num;
			assert(num > 0);
		}
	}

private:
	friend class IOThread;
	
	void init(int port);
	void createListenSocket();
	void handleAccept(int listenfd);
	Connection* createConnection(int fd, const sockaddr* addr, socklen_t addrLen);
	void returnConnection(Connection* conn);

	bool serverOverloaded();
	bool drainPendingTask();

private:
	static const int LISTEN_BACKLOG = 1024;
	static const int DEFAULT_THREAD_NUM = 1;

	bool serving_;
	int port_;
	int listenfd_;
	//struct event_base* evbase_;
	int numIOThreads_;
	std::vector<std::shared_ptr<IOThread> > ioThreads_;
	int nextThread_;

	std::vector<Connection*> activeConnections_;
	std::stack<Connection*> freeConnections_;

	OverloadAction overloadAction_;
  	bool overloaded_;
  	/// Count of connections dropped since overload started
 	uint32_t nConnectionsDropped_;
  	/// Count of connections dropped on overload since server started
  	uint64_t nTotalConnectionsDropped_;

	Mutex connMutex_;
};

}

#endif