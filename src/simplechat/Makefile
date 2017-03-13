FLAG=-std=c++11
INCLUDE=../../include
LIB_DIR=../../lib
#STATIC_LIB=$(LIB_DIR)/libconcurrency.a $(LIB_DIR)/libcommon.a libglog.a
STATIC_LIB=libconcurrency.a libcommon.a libglog.a libgflags.a libboost_system.a
STATIC_LIB_PATH=$(addprefix $(LIB_DIR)/, $(STATIC_LIB))

OBJ= main.o SimpleChat.pb.o UserInfo.o SimpleChatServer.o Packet.o Message.o Processor.o 

all: SimpleChatServer SimpleChatClient

SimpleChatClient: SimpleChatClient.cc SimpleChat.pb.o
	g++ $(FLAG) -o $@ $< SimpleChat.pb.o -I$(INCLUDE) $(STATIC_LIB_PATH) -L$(LIB_DIR) -lprotobuf

SimpleChatServer: $(OBJ)
	g++ $(FLAG) -o $@ $(OBJ) -I$(INCLUDE) $(STATIC_LIB_PATH) -L$(LIB_DIR) -lpthread -lprotobuf

%.o: %.cc
	g++ $(FLAG) -c $< -o $@ -I$(INCLUDE)
	
clean:
	rm -f $(OBJ) SimpleChatServer SimpleChatClient