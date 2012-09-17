/*
 * ConnectThread.h
 *
 *  Created on: 2012-9-10
 *      Author: LiuYongjin
 */
#ifndef _LIB_CONNECT_THREAD_H_
#define _LIB_CONNECT_THREAD_H_

#include <unistd.h>

#include "Thread.h"
#include "NetInterface.h"
#include "EventHandler.h"
#include "Queue.h"

class ThreadPipeHandler:public EventHandler
{
public: //实现EventHandler的接口
	virtual HANDLE_RESULT on_readable(int fd)
	{
		//接收消息,把管道到数据全部读取出来
		//很快,一般只循环一次;链接发得太快,导致很多消息堵塞...但是没有关系
		char buf[100];
		while(read(fd, buf, 100) > 0)
			;
		SLOG_DEBUG("Thread[ID=%d, Addr=%x] pipe fd=%d reable able", m_thread->get_id(),m_thread,fd);
		m_thread->do_task(); //处理任务
		return HANDLE_OK;
	}
public:
	ThreadPipeHandler(Thread<SocketHandle> *thread):m_thread(thread){}
private:
	Thread<SocketHandle> *m_thread;
};

class ConnectThread:public Thread<SocketHandle>, public NetInterface
{
public://实现Thread的接口
	bool do_task();
protected://实现Thread的接口
	bool notify_add_task();

public:
	ConnectThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager);
private:
	int m_pipe[2];
	ThreadPipeHandler m_pipe_handler;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数  //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//public:
	//////////////////由应用层重写 接收协议函数//////////////////
//	virtual int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送错误处理函数//////////
//	virtual int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送成功处理函数//////////
//	virtual int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 连接错误处理函数//////////////
//	virtual int on_socket_handle_error(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 连接超时处理函数//////////////
//	virtual int on_socket_handle_timeout(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 收到一个新的连接请求////////
//	virtual int on_socket_handler_accpet(SocketHandle socket_handle){return 0;}
};

#endif //_LIB_CONNECT_THREAD_H_



