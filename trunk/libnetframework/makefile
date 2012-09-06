LIB=-lslog
DEBUG=-g #-D__SLOG_FULL__

LIB_NAME=libnetframework.a
SO_NAME=libnetframework.so

LIB_INC=/usr/local/include/libnetframework
LIB_PATH=/usr/local/lib
	
lib:static share
install:
	mkdir ${LIB_INC} -p
	cp *.h ${LIB_INC}
	cp  ${LIB_NAME} ${LIB_PATH}
	cp ${SO_NAME} ${LIB_PATH}
uninstall:
	rm ${LIB_INC} -r
	rm ${LIB_PATH}/${LIB_NAME}
	rm ${LIB_PATH}/${SO_NAME}
	
static:*.cpp *.h
	g++ $^ -c ${DEBUG}
	ar rc ${LIB_NAME} *.o

share:*.cpp *.h
	g++ -fPIC -c  $^ ${LIB} ${DEBUG}
	g++ -shared *.o -o libnetframework.so

test:test_protocol test_demuxer test_socketmanager test_socketmanager_client

test_protocol:test_Protocol.c ProtocolDefault.cpp ProtocolDefault.h ProtocolType.h Protocol.h MemManager.h
	g++ $^  ${LIB} ${DEBUG} -o $@

test_demuxer:test_IODemuxer.c IODemuxerEpoll.cpp IODemuxerEpoll.h IODemuxer.h
	g++ $^  ${LIB} ${DEBUG} -o $@

test_socketmanager:test_SocketManager.cpp SocketManager.cpp SocketManager.h IODemuxerEpoll.cpp IODemuxerEpoll.h IODemuxer.h ProtocolDefault.cpp Socket.cpp EventHandler.h MemManager.h
	g++ $^  ${LIB} ${DEBUG} -o $@

test_socketmanager_client:test_SocketManager_client.c SocketManager.cpp SocketManager.h IODemuxerEpoll.cpp IODemuxerEpoll.h IODemuxer.h ProtocolDefault.cpp Socket.cpp EventHandler.h MemManager.h
	g++ $^  ${LIB} ${DEBUG} -o $@

clean:
	rm *.gch *.o test_protocol test_demuxer test_socketmanager test_socketmanager_client

