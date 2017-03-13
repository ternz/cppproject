INCLUDE=../../../include
STATIC_LIB=../../../lib/libconcurrency.a ../../../lib/libcommon.a
TARGE=mutex_test thread_test

all: $(TARGE)

mutex_test: mutex_test.cpp
	g++ -o $@ $< -I$(INCLUDE) $(STATIC_LIB) -lpthread

thread_test: thread_test.cpp
	g++ -o $@ $< -I$(INCLUDE) $(STATIC_LIB) -lpthread
	
clean:
	rm -f $(TARGE)
