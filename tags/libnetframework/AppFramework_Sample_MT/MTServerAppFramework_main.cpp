/*
 * MTServerAppFramework_main.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "MTServerAppFramework.h"
#include "ListenHandler.h"
#include "Socket.h"
#include "IODemuxerEpoll.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}


	MTServerAppFrameworkPool server_pool(3);
	server_pool.start();

	//listen event
	ListenHandler listen_handler(&server_pool);
	EpollDemuxer io_demuxer;
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//timer event
	TimerHandler timer(&io_demuxer);
	io_demuxer.register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}

