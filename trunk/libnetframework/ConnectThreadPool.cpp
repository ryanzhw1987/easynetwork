/*
 * ConnectThreadPool.cpp
 *
 *  Created on: 2012-9-10
 *      Author: xl
 */

#include "ConnectThreadPool.h"
#include "ConnectThread.h"

bool ConnectThreadPool::accept(SocketHandle trans_fd)
{
	SLOG_DEBUG("connect thread pool accept fd=%d", trans_fd);
	unsigned int thread_num = get_thread_num();
	if(thread_num <= 0)
		return false;

	ConnectThread* connect_thread = (ConnectThread*)get_thread(m_current_thread_index);
	SLOG_DEBUG("trans fd=%d dispatch to thread[ID=%d]",trans_fd, m_current_thread_index);

	m_current_thread_index = (++m_current_thread_index)%thread_num;
	return connect_thread->accept(trans_fd);
}
