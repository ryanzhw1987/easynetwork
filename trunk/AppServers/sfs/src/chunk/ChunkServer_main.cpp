/*
 * MTServerAppFramework_main.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "ChunkServer.h"

int main()
{
	SLOG_INIT("./config/slog.config");

	ChunkServer chunk_server;
	chunk_server.start_server();

	SLOG_UNINIT();
	return 0;
}

