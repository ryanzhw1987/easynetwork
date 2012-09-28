#include <stdio.h>
#include <string.h>

#include "ServerAppFramework.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ServerAppFramework app_server;
	app_server.start_instance();
	app_server.run_server();
	app_server.stop_instance();

	SLOG_UNINIT();
	return 0;
}



