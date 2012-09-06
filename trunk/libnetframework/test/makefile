LIB=-lslog
DEBUG=-g #-D__SLOG_FULL__
INC_DIR=-I../

test_SocketManager:test_SocketManager.cpp ../*.cpp ../*.h
	g++ $^ ${INC_DIR} ${LIB} ${DEBUG} -o $@
	
test_SocketManager_client:test_SocketManager_client.cpp ../*.cpp ../*.h
	g++ $^ ${INC_DIR} ${LIB} ${DEBUG} -o $@

clean:
	rm test_SocketManager