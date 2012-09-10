/*
 * NetInterface.h
 *
 *  Created on: 2012-9-7
 *      Author: LiuYongjin
 */

#ifndef _LIB_NET_INTERFACE_H_
#define _LIB_NET_INTERFACE_H_


#include "IODemuxer.h"
#include "Protocol.h"
#include "SocketManager.h"
#include "EventHandler.h"
#include "ConnectAccepter.h"

#include <queue>
#include <map>
using std::queue;
using std::map;

typedef map<SocketHandle, queue<Protocol*> > ProtocolMap;
class NetInterface:public ConnectAccepter, public EventHandler
{
public:
	//实现ConnectAccepter:接收一个新的连接请求
	virtual bool accept(SocketHandle trans_fd);

public:
	//重写EventHandler:实现trans socket的读写
	virtual HANDLE_RESULT on_readable(int fd);
	virtual HANDLE_RESULT on_writeabble(int fd);
	virtual HANDLE_RESULT on_timeout(int fd);  //to do deal with timeout
	virtual HANDLE_RESULT on_error(int fd); //to do deal with error


public:
	NetInterface(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
	{
		m_io_demuxer=io_demuxer;
		m_protocol_family = protocol_family;
		m_socket_manager = socket_manager;
		m_socket_idle_timeout_ms = 12000;
	}

	virtual ~NetInterface();
	IODemuxer* get_io_demuxer(){return m_io_demuxer;}
	ProtocolFamily* get_protocol_family(){return m_protocol_family;}
	SocketManager* get_socket_manager(){return m_socket_manager;}

private:
	IODemuxer *m_io_demuxer;
	ProtocolFamily *m_protocol_family;
	SocketManager *m_socket_manager;
	int m_socket_idle_timeout_ms;  //socket 空闲超时时间.超过该时间链接将被断开. 默认12s

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////       协议发送处理         //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
	//获取主动链接
	virtual SocketHandle get_active_trans_socket(const char *ip, int port);

	//添加协议到发送队列.成功返回0.失败返回-1,需要自行处理protocol.
	virtual int send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp=false);
	//获取等待队列中待发送的协议
	virtual Protocol* get_wait_to_send_protocol(SocketHandle socket_handle);
	//获取等待队列中待发送的协议个数
	virtual int get_wait_to_send_protocol_number(SocketHandle socket_handle);
	//取消所有待发送协议,同时调用on_protocol_send_error通知应用层
	virtual int cancal_wait_to_send_protocol(SocketHandle socket_handle);
private:
	//待发送的协议队列
	ProtocolMap m_protocol_map;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数   //////////
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
};


#endif //_LIB_NET_INTERFACE_H_
