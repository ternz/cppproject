#include <stdlib.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "SimpleChatServer.h"

DEFINE_int32(port, 9090, "port to listen on");
DEFINE_uint32(threads, 1, "number of threads to execute");

int main(int argc, char *argv[]) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = true;
	//FLAGS_stderrthreshold = google::DEBUG;
	//FLAGS_colorlogtostderr = true;
	//FLAGS_minloglevel = google::DEBUG;

	SimpleChatServer server;
	server.Server();
}