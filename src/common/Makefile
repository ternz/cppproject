FLAG=
LIBS=
OBJ=TOutput.o
PREFIX=../..
DIR_NAME=common
INSTALL_INCLUDE=$(PREFIX)/include/$(DIR_NAME)
INSTALL_LIB=$(PREFIX)/lib

all: libcommon.a install

libcommon.a: $(OBJ)
	ar r $@ $(OBJ)

%.o: %.cpp
	g++ $(FLAG) -c $< -o $@
	
install:
	mkdir -p $(INSTALL_INCLUDE)
	cp *.h $(INSTALL_INCLUDE)
	cp libcommon.a $(INSTALL_LIB)
	
unstall:
	rm -r $(INSTALL_INCLUDE)
	rm -f $(INSTALL_LIB)/libcommon.a
	
clean:
	rm -f *.o libcommon.a