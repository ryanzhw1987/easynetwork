/*
 * ConnectThread.cpp
 *
 *  Created on: 2012-9-11
 *      Author: LiuYongjin
 */


#include "slog.h"
#include "ConnectThread.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

////////////////// connect thread //////////////////
ConnectThread::ConnectThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
		:NetInterface(io_demuxer, protocol_family, socket_manager)
		,m_pipe_handler(this)
{
	if (pipe(m_pipe))
	{
		SLOG_ERROR("create pipe errer when creating connect thread");
		assert(0);
	}

	int flags = fcntl(m_pipe[0], F_GETFL, 0);
	if(flags != -1 )
	{
		flags |= O_NONBLOCK;
		if(fcntl(m_pipe[0], F_SETFL, flags) == -1)
			SLOG_ERROR("set pipe[0] no block failed. errno=%d", errno);
	}

	//注册管道读事件
	io_demuxer->register_event(m_pipe[0], EVENT_READ|EVENT_PERSIST, -1, &m_pipe_handler);
}

bool ConnectThread::notify_add_task()
{
	//往管道写数据,通知connect thread
	if(write(m_pipe[1], "", 1) != 1)
		SLOG_WARN("notify connect thread to accept a new connect failed.");
	return true;
}

bool ConnectThread::do_task()
{
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task",get_id(), this);
	Queue<SocketHandle> trans_queue(false);
	get_task_queue()->transform(&trans_queue, false);

	SocketHandle trans_fd;
	while(trans_queue.pop(trans_fd))
	{
		SLOG_DEBUG("thread accept trans fd=%d", trans_fd);
		if(accept(trans_fd)==false)
			SLOG_ERROR("connect thread accept fd=%d failed", trans_fd);
	}
}
