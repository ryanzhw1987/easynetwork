/*
 * ConnectThreadPool.h
 *
 *  Created on: 2012-9-10
 *      Author: xl
 */
#ifndef _LIB_ConnectThreadPool_H_
#define _LIB_ConnectThreadPool_H_

#include "ThreadPool.h"
#include "ConnectAccepter.h"

class ConnectThreadPool:public ThreadPool, public ConnectAccepter
{
public:
	//实现ConnectAccepter:接收一个新的连接请求
	bool accept(SocketHandle trans_fd);
public:
	ConnectThreadPool(unsigned int thread_num):ThreadPool(thread_num),m_current_thread_index(0){}
protected:
	//创建一个线程
	//virtual Thread* create_thread()=0;
private:
	unsigned int m_current_thread_index;
};

#endif //_LIB_ConnectThreadPool_H_



