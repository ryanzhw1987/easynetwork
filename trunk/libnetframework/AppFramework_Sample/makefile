INC=-I/usr/local/include/libnetframework

FLAGS=-D__SLOG_FULL__ -g #-static
#LIBS=-lslog -lnetframework
LIBS=-lslog /usr/local/lib/libnetframework.a -pthread


all:test_server test_client
test_server:StringProtocolFamily.h ServerAppFrameworkMain.cpp ServerAppFramework.cpp ServerAppFramework.h
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

test_client:StringProtocolFamily.h ClientAppFrameworkMain.cpp ClientAppFramework.cpp ClientAppFramework.h
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

clean:
	rm test_server test_client
