#ifndef Z_PROTOCOL_
#define Z_PROTOCOL_
#include <memory>

namespace ZNetworkFramework {

class Protocol {
public:
	//解析协议
	int wantSize();
	bool parseData(const char* buffer, const int size);

	//获取解析后的数据结构
	int getDataId();
	template <class T> std::shared_ptr<T> getDataEntity();
};


}

#endif