INCS=-I/usr/local/include/libnetframework -I../
LIBS=-lslog -lnetframework -lpthread -lssl
FLAGS=-g

all:sfs_chunk

sfs_chunk:*.h *.cpp ../SFSProtocolFamily.h ../SFSProtocolFamily.cpp
	g++ ${INCS} $^ ${LIBS} ${FLAGS} -o $@
clean:
	rm sfs_chunk