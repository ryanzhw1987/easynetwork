#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "ListenHandler.h"

#include "ServerAppFramework.h"

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	EpollDemuxer io_demuxer;
	DefaultProtocolFamily protocol_family;
	SocketManager socket_manager;
	ServerAppFramework app_server(&io_demuxer, &protocol_family, &socket_manager);

	//listen event
	ListenHandler listen_handler(&app_server);
	io_demuxer.register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);
	//timer event
	TimerHandler timer(&io_demuxer);
	io_demuxer.register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}



