INC=-I/usr/local/include/libnetframework

FLAGS=-D__SLOG_FULL__ -g #-static
#LIBS=-lslog -lnetframework
LIBS=-lslog /usr/local/lib/libnetframework.a -lpthread

all:test_server
	
test_server:MTServerAppFramework_main.cpp MTServerAppFramework.cpp MTServerAppFramework.h
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@
clean:
	rm test_server
