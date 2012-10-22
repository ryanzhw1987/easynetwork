INC=-I/usr/local/include/libnetframework -I./
FLAGS=-D__SLOG_FULL__ -g #-static
LIBS=-lslog /usr/local/lib/libnetframework.a -lpthread

SERVER_DIR=./DownServer
CLIENT_DIR=./DownClient

all:${SERVER_DIR}/download_server ${CLIENT_DIR}/download_client

${SERVER_DIR}/download_server:${SERVER_DIR}/*.h ${SERVER_DIR}/*.cpp DownloadProtocol.*
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

${CLIENT_DIR}/download_client:${CLIENT_DIR}/*.h ${CLIENT_DIR}/*.cpp DownloadProtocol.*
	g++ $^ ${INC} ${LIBS} ${FLAGS} -o $@

clean:
	rm ${SERVER_DIR}/download_server ${CLIENT_DIR}/download_client
