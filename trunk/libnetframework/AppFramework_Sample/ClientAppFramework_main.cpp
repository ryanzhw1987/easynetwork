#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "ClientAppFramework.h"

int main()
{
	SLOG_INIT(NULL);

	ClientAppFramework app_framework;
	app_framework.start_server();

	SLOG_UNINIT();
	return 0;
}



