#include "common/common.h"
#include "server.h"
#include "util.h"

namespace networkFramework {

Server::Server(int port) {
	init(port);
}

Server::~Server() {
	if(eventBase_ != NULL) {
		
	}
}

void Server::server() {
	//!!!!!!!
}

void Server::createListenSocket() {
	int sockfd;

	struct addrinfo hints, *res, *res0;
	int error;

	char port[sizeof("65536") + 1];
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	sprintf(port, "%d", port_);

	// Wildcard address
	error = getaddrinfo(NULL, port, &hints, &res0);
	if (error) {
		throw TException("TNonblockingServer::serve() getaddrinfo "
						+ string(THRIFT_GAI_STRERROR(error)));
	}

	// Pick the ipv6 address first since ipv4 addresses can be mapped
	// into ipv6 space.
	for (res = res0; res; res = res->ai_next) {
		if (res->ai_family == AF_INET6 || res->ai_next == NULL)
		break;
	}

	// Create the server socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1) {
		freeaddrinfo(res0);
		throw TException("TNonblockingServer::serve() socket() -1");
	}

#ifdef IPV6_V6ONLY
	if (res->ai_family == AF_INET6) {
		int zero = 0;
		if (-1 == setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, const_cast_sockopt(&zero), sizeof(zero))) {
			GlobalOutput("TServerSocket::listen() IPV6_V6ONLY");
		}
	}
#endif // #ifdef IPV6_V6ONLY

	int one = 1;

	// Set THRIFT_NO_SOCKET_CACHING to avoid 2MSL delay on server restart
	//setsockopt(sockfd, SOL_SOCKET, THRIFT_NO_SOCKET_CACHING, const_cast_sockopt(&one), sizeof(one));
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&one), sizeof(one));

	if (::bind(sockfd, res->ai_addr, static_cast<int>(res->ai_addrlen)) == -1) {
		close(sockfd);
		freeaddrinfo(res0);
		throw TTransportException(TTransportException::NOT_OPEN,
								"Server::serve() bind", errno);
	}
	// Done with the addr info
	freeaddrinfo(res0);

	// Set socket to nonblocking mode
	if (set_nonblocking(sockfd) < 0) {
		close(sockfd);
		throw TException("Server::createListenSocket() set O_NONBLOCK");
	}

	// Turn linger off to avoid hung sockets
	struct linger ling = {0, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_LINGER, reinterpret_cast<const void*>(&ling), sizeof(ling));

	if (listen(sockfd, LISTEN_BACKLOG) == -1) {
		close(sockfd);
		throw TException("Server::serve() listen");
	}

	// Cool, this socket is good to go, set it as the listenfd_
	listenfd_ = sockfd;

	/*if (!port_) {
		sockaddr_in addr;
		socklen_t size = sizeof(addr);
		if (!getsockname(listenfd_, reinterpret_cast<sockaddr*>(&addr), &size)) {
			listenPort_ = ntohs(addr.sin_port);
		} else {
			GlobalOutput.perror("TNonblocking: failed to get listen port: ", THRIFT_GET_SOCKET_ERROR);
		}
	}*/
}


void Server::handleAccept(int listenfd) {
	assert(listenfd == listenfd_);

	socklen_t addrLen;
  	sockaddr_storage addrStorage;
  	sockaddr* addrp = (sockaddr*)&addrStorage;
  	addrLen = sizeof(addrStorage);

	int fd;
	while(fd = ::accept(fd, addrp, addrLen) != -1) {
		if (overloadAction_ != OVERLOAD_NO_ACTION && serverOverloaded()) {
			Guard g(connMutex_);
			nConnectionsDropped_++;
			nTotalConnectionsDropped_++;
			if (overloadAction_ == OVERLOAD_CLOSE_ON_ACCEPT) {
				::close(clientSocket);
				return;
			}
		} else if (overloadAction_ == OVERLOAD_DRAIN_TASK_QUEUE) {
			if (!drainPendingTask()) {
				// Nothing left to discard, so we drop connection instead.
				::close(clientSocket);
				return;
			}
		}

		if(set_nonblocking(fd) < 0) {
			GlobalOutput.perror("Server handleAccept set O_NONBLOCK error(%d)", errno);
			::close(fd);
			return;
		}

		Connection* conn = createConnection(fd, addrp, addrLen);
		if(conn == NULL) {
			GlobalOutput.printf("Server createConnection failed");
			::close(fd);
			return;
		}

		if(conn->getIOThreadNum() == 0) {
			conn->transition();
		} else {
			if(!conn->notifyIOThread()) {
				GlobalOutput.perror("[ERROR] notifyIOThread failed on fresh connection, closing", errno);
       	 		returnConnection(conn);
			}
		}

		addrLen = sizeof(addrStorage);
	}

	if(errno != EAGAIN || errno != EWOULDBLOCK) {
		GlobalOutput.perror("[ERROR] server::handleAccept ", errno);
	}
}

}