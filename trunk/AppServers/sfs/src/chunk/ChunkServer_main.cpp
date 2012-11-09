/*
 * MTServerAppFramework_main.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "ChunkServer.h"
#include "ChunkWorker.h"
#include "ListenHandler.h"
#include "Socket.h"
#include "IODemuxerEpoll.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ListenSocket linsten_socket(3013);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	//chunk server
	ChunkServer chunk_server;
	chunk_server.start_instance();

	IODemuxer *io_demuxer = chunk_server.get_io_demuxer();

	//chunk worker pool
	ChunkWorkerPool server_pool(3);
	server_pool.start();

	//监听端口
	ListenHandler listen_handler(&server_pool);
	io_demuxer->register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//运行chunk server
	chunk_server.run_server();

	SLOG_UNINIT();
	return 0;
}

