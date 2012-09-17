INC=-I/usr/local/include/libnetframework -I./
FLAGS=-D__SLOG_FULL__ -g #-static
LIBS=-lslog /usr/local/lib/libnetframework.a -lpthread

SERVER_DIR=./DownServer
CLIENT_DIR=./DownClient

all:download_server

download_server:${SERVER_DIR}/*.h ${SERVER_DIR}/*.cpp DownloadProtocol.*
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

clean:
	rm download_server
