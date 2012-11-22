/*
 * DownloadServer_main.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "MasterServer.h"
#include "ListenHandler.h"
#include "Socket.h"
#include "IODemuxerEpoll.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ListenSocket linsten_socket(3012);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	MasterThreadPool server_pool(1);
	server_pool.start();

	//listen event
	ListenHandler listen_handler(&server_pool);
	EpollDemuxer io_demuxer;
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}


