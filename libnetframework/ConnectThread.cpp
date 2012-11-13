/*
 * ConnectThread.cpp
 *
 *  Created on: 2012-9-11
 *      Author: LiuYongjin
 */

#include "ConnectThread.h"
#include "slog.h"

//实现接口:线程实际运行的入口
void ConnectThread::run_thread()
{
	SLOG_INFO("ConnectThread[ID=%d] is running...", get_thread_id());
	//Start App Server(NetInterface)
	start_server();

	SLOG_INFO("ConnectThread end...");
}

//实现接口:响应添加任务事件
bool ConnectThread::on_notify_add_task()
{
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task", get_thread_id(), this);
	SocketHandle trans_fd;
	while(get_task(trans_fd))
	{
		SLOG_DEBUG("thread accept trans fd=%d", trans_fd);
		if(NetInterface::accept(trans_fd) == false)
			SLOG_ERROR("connect thread accept fd=%d failed", trans_fd);
	}
	return true;
}

bool ConnectThread::register_notify_handler(int read_pipe, EVENT_TYPE event_type, EventHandler* event_handler)
{
	IODemuxer* io_demuxer = get_io_demuxer();
	return io_demuxer->register_event(read_pipe,event_type,-1,event_handler)==0?true:false;
}

