/*
 * ConnectThread.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */


#include "slog.h"
#include "ConnectThread.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//管道可读后,接收新消息
int PipeListenHandler::receive_connect(int listen_fd)
{
	SLOG_DEBUG("receive connect");
	//接收消息,把管道到数据全部读取出来
	//很快,一般只循环一次;链接发得太快,导致很多消息堵塞...但是没有关系
	char buf[100];
	while(read(listen_fd, buf, 100) > 0)
		;
	return 0;  //虚假的链接,由connect_accepter来处理
}

////////////////// connect thread //////////////////
ConnectThread::ConnectThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
		:NetInterface(io_demuxer, protocol_family, socket_manager)
		,m_connect_queue(true)
		,m_listen_handler(this)
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
	io_demuxer->register_event(m_pipe[0], EVENT_READ|EVENT_PERSIST, -1, &m_listen_handler);
}

//通知线程收到新到链接请求
bool ConnectThread::accept(SocketHandle trans_fd)
{
	//外界(connect thread pool)调用
	if(trans_fd > 0)
	{
		SLOG_DEBUG("thread[ID=%d] accept new trans fd=%d", get_id(), trans_fd);
		if(m_connect_queue.push(trans_fd) == false)
			return false;

		//往管道写数据,通知connect thread
		if(write(m_pipe[1], "", 1) != 1)
			SLOG_WARN("notify connect thread to accept a new connect failed.");
		return true;
	}
	//PipeListenHandler调用
	else if(trans_fd == 0)
	{
		Queue<SocketHandle> trans_queue(false);
		m_connect_queue.transform(&trans_queue, false);
		SocketHandle trans_fd;
		while(trans_queue.pop(trans_fd))
		{

			SLOG_DEBUG("thread accept trans fd=%d", trans_fd);
			NetInterface::accept(trans_fd);
		}
	}
	else
	{
		SLOG_ERROR("connect thread accept error trans_fd:%d", trans_fd);
	}

	return true;
}


