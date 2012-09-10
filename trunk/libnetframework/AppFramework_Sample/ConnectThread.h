/*
 * ConnectThread.h
 *
 *  Created on: 2012-9-10
 *      Author: xl
 */
#ifndef _LIB_CONNECT_THREAD_H_
#define _LIB_CONNECT_THREAD_H_

#include "Thread.h"
#include "ConnectAccepter.h"
#include "Queue.h"

class ConnectThread:public Thread, public ConnectAccepter
{
public:
	//实现ConnectAccepter:接收一个新的连接请求
	bool accept(SocketHandle trans_fd){return m_connect_queue.push(trans_fd);}
public:
	ConnectThread():m_connect_queue(true){}
private:
	//线程安全队列
	Queue<SocketHandle> m_connect_queue;
};

#endif //_LIB_CONNECT_THREAD_H_



