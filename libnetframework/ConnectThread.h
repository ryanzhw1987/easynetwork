/*
 * ConnectThread.h
 *
 *  Created on: 2012-9-10
 *      Author: LiuYongjin
 */
#ifndef _LIB_CONNECT_THREAD_H_
#define _LIB_CONNECT_THREAD_H_

#include <unistd.h>

#include "PipeThread.h"
#include "NetInterface.h"
#include "EventHandler.h"
#include "Queue.h"

class ConnectThread:public PipeThread<SocketHandle>, public NetInterface
{
protected:
	//实现接口:线程实际运行的入口
	void run()
	{
		SLOG_INFO("ConnectThread[ID=%d] is running...", get_id());
		get_io_demuxer()->run_loop();
		SLOG_INFO("ConnectThread end...");
	}

	//实现接口:响应添加任务事件
	bool on_notify_add_task();
public:
	ConnectThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager, bool detachable=true, unsigned int stack_size=0, int id=0)
			:PipeThread<SocketHandle>(io_demuxer, detachable, stack_size, id)
			,NetInterface(io_demuxer, protocol_family, socket_manager){}
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



