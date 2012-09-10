/*
 * ConnectThreadPool.cpp
 *
 *  Created on: 2012-9-10
 *      Author: xl
 */

#include "ConnectThreadPool.h"

bool ConnectThreadPool::accept(SocketHandle trans_fd)
{
	unsigned int thread_num = get_thread_num();
	ConnectThread* connect_thread = (ConnectThread*)get_thread(m_current_thread_index);
	m_current_thread_index = (++m_current_thread_index)%thread_num;
	return connect_thread->accept(trans_fd);
}
