#include "common/common.h"
#include "util.h"

namespace ZNetworkFramework {

int set_fd_flag(int flag) {
	int o_flags;
	o_flags = fcntl(fd, F_GETFL);
	if(o_flags < 0) return o_flags;
	return fcntl(fd, F_SETFL, o_flags | flag);
}

int set_nonblocking(int fd) {
	return set_fd_flag(O_NONBLOCK);
}

int set_close_on_exec(int fd) {
	return set_fd_flag(FD_CLOEXEC);
}

}