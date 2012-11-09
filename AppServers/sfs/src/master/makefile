INCS=-I/usr/local/include/libnetframework -I../
LIBS=-lslog -lnetframework -lpthread -lssl
FLAGS=-g

all:sfs_master

sfs_master:*.h *.cpp ../SFSProtocolFamily.h ../SFSProtocolFamily.cpp
	g++ ${INCS} $^ ${LIBS} ${FLAGS} -o $@
clean:
	rm sfs_master