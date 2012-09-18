/*
 * ConnectThread.cpp
 *
 *  Created on: 2012-9-11
 *      Author: LiuYongjin
 */

#include "ConnectThread.h"
#include "slog.h"

bool ConnectThread::on_notify_add_task()
{
	SLOG_DEBUG("Thread[ID=%d,Addr=%x] do task",get_id(), this);

	SocketHandle trans_fd;
	while(get_task(trans_fd))
	{
		SLOG_DEBUG("thread accept trans fd=%d", trans_fd);
		if(NetInterface::accept(trans_fd) == false)
			SLOG_ERROR("connect thread accept fd=%d failed", trans_fd);
	}

	return true;
}
