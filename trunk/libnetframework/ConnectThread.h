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

class ConnectThread:public NetInterface, public PipeThread<SocketHandle>
{
public:
	ConnectThread(bool detachable=true, unsigned int stack_size=0, int id=0):PipeThread<SocketHandle>(detachable, stack_size, id){}
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现PipeThread接口:注册通知事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);


/*NetInterface接口函数
////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写工厂方法函数   //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
protected:
	virtual IODemuxer* create_io_demuxer()=0;
	virtual void delete_io_demuxer(IODemuxer* io_demuxer)=0;
	virtual SocketManager* create_socket_manager()=0;
	virtual void delete_socket_manager(SocketManager* socket_manager)=0;
	virtual ProtocolFamily* create_protocol_family()=0;	//具体的协议由应用层决定,应用层必须提供该协议族
	virtual void delete_protocol_family(ProtocolFamily* protocol_family)=0;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数  //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
	//////////////////由应用层重写 接收协议函数//////////////////
	virtual int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送错误处理函数//////////
	virtual int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送成功处理函数//////////
	virtual int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 连接错误处理函数//////////////
	virtual int on_socket_handle_error(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 连接超时处理函数//////////////
	virtual int on_socket_handle_timeout(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 收到一个新的连接请求////////
	virtual int on_socket_handler_accpet(SocketHandle socket_handle){return 0;}
*/
};

#endif //_LIB_CONNECT_THREAD_H_



