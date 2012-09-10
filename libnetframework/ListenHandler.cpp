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
	int new_fd = receive_connect(fd);
	if(new_fd == -1)
		return HANDLE_OK;

	assert(m_connect_accepter != NULL);
	if(m_connect_accepter->accept(new_fd) == false)
	{
		SLOG_ERROR("connect_accepter accepts new connection failed. close connection. fd=%d", new_fd);
		close(new_fd);
	}

	return HANDLE_OK;
}

int ListenHandler::receive_connect(int listen_fd)
{
	int fd = accept(listen_fd, NULL, 0);
	if(fd == -1)
	{
		if(errno==EAGAIN || errno==EINPROGRESS || errno==EINTR)  //被中断
			SLOG_WARN("accept client socket interrupted. errno=%d", errno);
		else
			SLOG_ERROR("accept client socket error. errno=%d", errno);
	}
	return fd;
}
