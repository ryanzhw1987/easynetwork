INCS=-I/usr/local/include/libnetframework -I../
LIBS=-lslog -lnetframework -lpthread -lssl
FLAGS=-g

all:lib sfs

lib:SFSFile.h SFSFile.cpp ../SFSProtocolFamily.h ../SFSProtocolFamily.cpp
	g++ -c ${INCS} $^ ${FLAGS}
	ar rc libsfsfile.a *.o

sfs:SFSClient.cpp SFSFile.h
	g++ ${INCS} $^ ${LIBS} ./libsfsfile.a ${FLAGS} -o $@

clean:
	rm *.o *.h.gch sfs
