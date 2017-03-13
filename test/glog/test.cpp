#include <glog/logging.h>

using namespace google;

int main(int argc, char* argv[]) {
	InitGoogleLogging(argv[0]);
	FLAGS_stderrthreshold=google::DEBUG;
	FLAGS_colorlogtostderr=true;
	FLAGS_minloglevel=google::DEBUG;
	FLAGS_logtoonefile=true;	
	FLAGS_log_dir="logs";
	//SetLogDestination(GLOG_INFO, "aaa");
	//SetLogDestination(GLOG_WARNING, "aaa");
	//SetLogDestination(GLOG_ERROR, "aaa");
	
	LOG(DEBUG)<<"log debug";
	LOG(INFO)<<"log info";
	LOG(WARNING)<<"log warning";
	LOG(ERROR)<<"log error";
	
	DLOG(DEBUG)<<"dlog debug";
	DLOG(INFO)<<"dlog info";
	DLOG(WARNING)<<"dlog warning";
	DLOG(ERROR)<<"dlog error";
}
