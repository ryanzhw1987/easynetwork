#include <stdio.h>
#include <string.h>

#include "ServerAppFramework.h"

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);

	ServerAppFramework app_server;
	app_server.init_instance();
	app_server.run_server();
	app_server.uninit_instance();

	SLOG_UNINIT();
	return 0;
}



