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

	//序列化data, 写入buffer, 成功返回数据长度；buffer过小返回0，改写size指示需要的大小值；出错返回-1
	template <class T> int serialize(int dataId, T* data, char* buffer, int* size);
};


}

#endif