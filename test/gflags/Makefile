INCLUDE_DIR=../../include
LIB_DIR=../../lib


all: test

test: test.cpp
	g++ -o $@ $< -I$(INCLUDE_DIR) $(LIB_DIR)/libgflags.a -lpthread