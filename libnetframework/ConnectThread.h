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
#include "ListenHandler.h"
#include "Queue.h"

class ConnectThread;
class PipeListenHandler:public ListenHandler
{
public:
	PipeListenHandler(ConnectAccepter *connect_thread):ListenHandler(connect_thread){}
protected:
	int receive_connect(int listen_fd);
};

class ConnectThread:public Thread, public NetInterface
{
public:
	//实现ConnectAccepter:接收一个新的连接请求
	virtual bool accept(SocketHandle trans_fd);
public:
	ConnectThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager);
protected:
	//virtual void run()=0;
private:
	//线程安全队列
	Queue<SocketHandle> m_connect_queue;
	int m_pipe[2];
	PipeListenHandler m_listen_handler;
////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数   //////////
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



