/*
 * ConnectThreadPool.h
 *
 *  Created on: 2012-9-10
 *      Author: LiuYongjin
 */
#ifndef _LIB_CONNECT_THREAD_POOL_H_
#define _LIB_CONNECT_THREAD_POOL_H_

#include "ThreadPool.h"
#include "ConnectAccepter.h"

class ConnectThreadPool:public ThreadPool<SocketHandle>, public ConnectAccepter
{
public:
	//实现ConnectAccepter:接收一个新的连接请求
	bool accept(SocketHandle trans_fd);
public:
	ConnectThreadPool(unsigned int thread_num):ThreadPool<SocketHandle>(thread_num){}
};

inline
bool ConnectThreadPool::accept(SocketHandle trans_fd)
{
	SLOG_DEBUG("connect thread pool accept fd=%d", trans_fd);
	return add_task(trans_fd);
}

#endif //_LIB_CONNECT_THREAD_POOL_H_



