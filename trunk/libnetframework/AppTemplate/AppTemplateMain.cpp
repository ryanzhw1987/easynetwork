/*
 * AppTemplateMain.h
 *
 *  Created on: #CreateDate#
 *      Author: #Author#
 */

#include "AppTemplate.h"
#include "slog.h"

int main(int argc, char *agrv[])
{
	SLOG_INIT(NULL);

	AppTemplate server;
	server.start_server();

	SLOG_UNINIT();
	return 0;
}



