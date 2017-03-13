FLAG=
LIBS=
OBJ=Util.o Mutex.o PosixThreadFactory.o
PREFIX=../..
INCLUDE=$(PREFIX)/include
DIR_NAME=concurrency
INSTALL_INCLUDE=$(PREFIX)/include/$(DIR_NAME)
INSTALL_LIB=$(PREFIX)/lib

all: libconcurrency.a install

libconcurrency.a: $(OBJ)
	ar r $@ $(OBJ)

%.o: %.cpp
	g++ $(FLAG) -c $< -o $@ -I$(INCLUDE)
	
install:
	mkdir -p $(INSTALL_INCLUDE)
	cp *.h $(INSTALL_INCLUDE)
	cp libconcurrency.a $(INSTALL_LIB)
	
unstall:
	rm -r $(INSTALL_INCLUDE)
	rm -f $(INSTALL_LIB)/libconcurrency.a
	
clean:
	rm -f *.o libconcurrency.a
