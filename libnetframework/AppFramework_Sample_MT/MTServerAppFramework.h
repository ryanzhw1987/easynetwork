/*
 * ServerAppFramework.h
 *
 *  Created on: 2012-9-11
 *      Author: LiuYongjin
 */

#ifndef _LIB_SERVER_APPFRAMEWORK_H_
#define _LIB_SERVER_APPFRAMEWORK_H_

#include "ConnectThread.h"
#include "ConnectThreadPool.h"
#include "ProtocolDefault.h"

class MTServerAppFramework:public ConnectThread
{
public:
	MTServerAppFramework(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager* socket_manager)
			:ConnectThread(io_demuxer, protocol_family, socket_manager)
	{}

	//////////////////由应用层重写 接收协议函数//////////////////
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	int on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	int on_socket_handle_timeout(SocketHandle socket_handle);
};

class MTServerAppFrameworkPool:public ConnectThreadPool
{
public:
	MTServerAppFrameworkPool(unsigned int thread_num):ConnectThreadPool(thread_num){}
protected:
	//实现创建一个线程
	Thread<SocketHandle>* create_thread();
};

class TimerHandler:public EventHandler
{
public:
	TimerHandler(IODemuxer *demuxer):m_demuxer(demuxer){;}
	HANDLE_RESULT on_timeout(int fd);
private:
	IODemuxer *m_demuxer;
};

#endif

