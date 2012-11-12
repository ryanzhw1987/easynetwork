/*
 * NetInterface.h
 *
 *  Created on: 2012-9-7
 *      Author: LiuYongjin
 */

#ifndef _LIB_NET_INTERFACE_H_
#define _LIB_NET_INTERFACE_H_


#include "IODemuxer.h"
#include "ProtocolFamily.h"
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
	virtual HANDLE_RESULT on_writeable(int fd);
	virtual HANDLE_RESULT on_timeout(int fd);  //to do deal with timeout
	virtual HANDLE_RESULT on_error(int fd); //to do deal with error
public:
	NetInterface();
	virtual ~NetInterface();
    //应用层在创建实例之后必须调用该函数来初始化实例
	bool start_instance();
	virtual bool on_start_instance(){return true;}
	//应用层在销毁实例之前必须调用该函数来反初始化实例
	bool stop_instance();
	virtual bool on_stop_instance(){return true;}

	IODemuxer* get_io_demuxer(){return m_io_demuxer;}
	ProtocolFamily* get_protocol_family(){return m_protocol_family;}
	SocketManager* get_socket_manager(){return m_socket_manager;}
	void set_idle_timeout(int timeout_ms){m_socket_idle_timeout_ms = timeout_ms;}
private:
	IODemuxer *m_io_demuxer;
	SocketManager *m_socket_manager;
	ProtocolFamily *m_protocol_family;
	int m_socket_idle_timeout_ms;  //socket 空闲超时时间.超过该时间链接将被断开. 默认12s
	//待发送的协议队列
	ProtocolMap m_protocol_map;
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
	//释放链接
	virtual bool release_trans_socket(SocketHandle socket_handle);

	//添加协议到发送队列.成功返回true.失败返回false,需要自行处理protocol.
	virtual bool send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp=false);
	//获取等待队列中待发送的协议
	virtual Protocol* get_wait_to_send_protocol(SocketHandle socket_handle);
	//获取等待队列中待发送的协议个数
	virtual int get_wait_to_send_protocol_number(SocketHandle socket_handle);
	//取消所有待发送协议,同时调用on_protocol_send_error通知应用层
	virtual int cancal_wait_to_send_protocol(SocketHandle socket_handle);

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写工厂方法函数   //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
protected:
	//////////////////由应用层重写 创建IODemuxer//////////////////
	virtual IODemuxer* create_io_demuxer()=0;
	//////////////////由应用层重写 销毁IODemuxer//////////////////
	virtual void delete_io_demuxer(IODemuxer* io_demuxer)=0;
	//////////////////由应用层重写 创建SocketManager//////////////
	virtual SocketManager* create_socket_manager()=0;
	//////////////////由应用层重写 销毁SocketManager//////////////
	virtual void delete_socket_manager(SocketManager* socket_manager)=0;
	//////////////////由应用层重写 创建具体的协议族//////////////
	virtual ProtocolFamily* create_protocol_family()=0;
	//////////////////由应用层重写 销毁协议族////////////////////
	virtual void delete_protocol_family(ProtocolFamily* protocol_family)=0;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数   //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
protected:
	//返回值:true:成功, false:失败
	//////////////////由应用层重写 接收协议函数//////////////////
	virtual bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)=0; //应用层设置detatch_protocol为true时,由应用层负责销毁protocol
	//////////////////由应用层重写 协议发送错误处理函数//////////
	virtual bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送成功处理函数//////////
	virtual bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 连接错误处理函数//////////////
	virtual bool on_socket_handle_error(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 连接超时处理函数//////////////
	virtual bool on_socket_handle_timeout(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 收到一个新的连接请求////////
	virtual bool on_socket_handler_accpet(SocketHandle socket_handle){return 0;}
};


#endif //_LIB_NET_INTERFACE_H_
