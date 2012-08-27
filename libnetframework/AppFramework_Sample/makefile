INC=-I/usr/local/include/libnetframework

FLAGS=-D__SLOG_FULL__ #-static
#LIBS=-lslog -lnetframework
LIBS=-lslog /usr/local/lib/libnetframework.a

all:test_server test_client
test_server:ServerAppFramework_main.c ServerAppFramework.cpp ServerAppFramework.h
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

test_client:ClientAppFramework_main.c ClientAppFramework.cpp ClientAppFramework.h
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

clean:
	rm test_server test_client
