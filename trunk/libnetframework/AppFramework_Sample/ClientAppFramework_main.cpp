#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "ClientAppFramework.h"
#include "IODemuxerEpoll.h"
#include "ProtocolDefault.h"

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	EpollDemuxer io_demuxer;
	DefaultProtocolFamily protocol_family;
	ClientAppFramework app_framework(&io_demuxer, &protocol_family);  //异步

	SocketHandle socket_handle = app_framework.create_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
    if(socket_handle == SOCKET_INVALID)
        return -1;

	PingHandler ping_handler(&app_framework, socket_handle);
	ping_handler.register_handler();

	io_demuxer.run_loop();

	SLOG_UNINIT();
	return 0;
}



