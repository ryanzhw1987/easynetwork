/*
 * DownloadManager.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadManager.h"
#include "DownloadProtocol.h"

///////////////////////////////  thread pool  //////////////////////////////////
Thread<DownloadTask*>* DownloadThreadPool::create_thread()
{
	EpollDemuxer *io_demuxer = new EpollDemuxer;
	DefaultProtocolFamily *protocol_family = new DefaultProtocolFamily;
	SocketManager *socket_manager = new SocketManager;
	return new DownloadThread(io_demuxer, protocol_family, socket_manager);
}


DownloadThread::DownloadThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
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

/////////////////////////////// thread 的接口  /////////////////////////////////////////
bool DownloadThread::notify_add_task()
{
	//往管道写数据,通知connect thread
	if(write(m_pipe[1], "", 1) != 1)
		SLOG_WARN("notify connect thread to accept a new connect failed.");
	return true;
}

bool DownloadThread::do_task()
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
void DownloadThread::run()
{
	SLOG_INFO("MTServerAppFramework[ID=%d] is running...", get_id());
	get_io_demuxer()->run_loop();
	SLOG_INFO("MTServerAppFramework end...");
}

////////////////// NetInterface的接口 由应用层重写 接收协议函数//////////////////
int DownloadThread::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_REQUEST_SIZE:
	{
		RespondSize* temp_protocol = (RespondSize*)protocol;
		SLOG_INFO("receive RespondSize[file=%s, size=%ld]", temp_protocol->get_file_name().c_str(), temp_protocol->get_size());
		break;
	}
	case PROTOCOL_REQUEST_SIZE:
	{
		RespondData* temp_protocol = (RespondData*)protocol;
		SLOG_INFO("receive RespondData[file=%s]", temp_protocol->get_file_name().c_str());
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	return 0;
}

int DownloadThread::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadThread::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int DownloadThread::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int DownloadThread::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}

