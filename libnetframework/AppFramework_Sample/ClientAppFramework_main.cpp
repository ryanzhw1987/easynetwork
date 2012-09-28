#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "ClientAppFramework.h"
#include "ProtocolDefault.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ClientAppFramework app_framework;  //异步
	app_framework.start_instance();

	SocketHandle socket_handle = app_framework.get_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
	if(socket_handle == SOCKET_INVALID)
		return -1;

	PingHandler ping_handler(&app_framework, socket_handle);
	ping_handler.register_handler();

	app_framework.get_io_demuxer()->run_loop();

	app_framework.stop_instance();
	SLOG_UNINIT();
	return 0;
}



