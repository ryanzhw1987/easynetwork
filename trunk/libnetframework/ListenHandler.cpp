/*
 * ListenHandler.cpp
 *
 *  Created on: 2012-9-5
 *      Author: Administrator
 */
#include "ListenHandler.h"
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include "slog.h"

////////////////////////////////////////////////////////////
///////////          ListenHandler         //////////
////////////////////////////////////////////////////////////
HANDLE_RESULT ListenHandler::on_readable(int fd)
{
	int new_fd = accept(fd, NULL, 0);
	if(new_fd == -1)
	{
		if(errno==EAGAIN || errno==EINPROGRESS || errno==EINTR)  //被中断
			return HANDLE_OK;

		SLOG_ERROR("accept client socket failed. errno=%d", errno);
		return HANDLE_ERROR;
	}

	assert(m_connect_accepter != NULL);
	if(m_connect_accepter->accept(new_fd) == false)
	{
		SLOG_ERROR("connect_accepter accepts new connection failed. close connection. fd=%d", new_fd);
		close(new_fd);
	}

	return HANDLE_OK;
}
