INC=-I/usr/local/include/libnetframework
#LIBS=-lslog -lnetframework
LIBS=-lslog /usr/local/lib/libnetframework.a
FLAGS=#-static

all:test_socketmanager test_socketmanager_client
test_socketmanager:test_SocketManager.c
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@
	
test_socketmanager_client:test_SocketManager_client.c
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

clean:
	rm test_socketmanager test_socketmanager_client
