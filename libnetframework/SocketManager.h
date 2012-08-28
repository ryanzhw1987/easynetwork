#ifndef _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_
#define _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

#include "Socket.h"
#include "Protocol.h"
#include "IODemuxer.h"
#include "EventHandler.h"

#include <map>
#include <queue>
using std::queue;
using std::map;
typedef map<SocketHandle, Socket*> SocketMap;
typedef map<SocketHandle, queue<Protocol*> > SendTaskMap;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////                                            ////////
////////               SocketMnager类               ////////
////////                                            ////////
////////          1.创建主动连接                    ////////
////////          2.监听服务器端口                  //////// 
////////          3.提供应用层协议数据发送接收      ////////
////////          4.通知应用层链接超时/出错状态     //////// 
////////                                            ////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
class SocketManager
{
public:
    SocketManager(BlockMode block_mode=NOBLOCK);
    virtual ~SocketManager();
private:
    BlockMode m_block_mode;             //连接的阻塞模式

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////       链接创建处理         //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
    //监听某个端口, 成功返回0, 失败返回-1;
    int listen(int port);
    //创建主动传输连接
    virtual SocketHandle create_active_trans_socket(const char *ip, int port);

    //添加被动传输连接(主要由框架调用)
    virtual int add_passive_trans_socket(const char *peer_ip, SocketHandle socket_handle);
    //删除(主/被动)传输连接,取消掉堵塞在其队列上的协议,同时通知应用层(主要由框架调用)
    virtual int delete_trans_socket(SocketHandle socket_handle);
    //查找监听连接(主要由框架调用)
    virtual Socket* find_listen_socket(SocketHandle socket_handle);
    //查找(主/被动)传输连接.(主要由框架调用)
    virtual Socket* find_trans_socket(SocketHandle socket_handle);
protected:
    virtual Socket* new_listen_socket();         //创建具体的listen socket
    virtual EventHandler* new_listen_handler();  //创建具体的listen handler
    virtual Socket* new_trans_socket();          //创建具体的trans socket
    virtual EventHandler* new_trans_handler();   //创建具体的trans handler
	virtual int init_passive_trans_socket(SocketHandle socket_handle, BlockMode block_mode); //对添加的trans socket进行必要的初始化
private:
    ListenSocket *m_listen_socket;   //监听socket
	EventHandler *m_listen_handler;  //监听handler
	SocketMap m_trans_sockets_map;   //传输socket
	EventHandler *m_trans_handler;   //传输handler


////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////       协议发送处理         //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
	//添加协议到发送队列.成功返回0.失败返回-1,需要自行处理protocol.
    virtual int send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp);
	//获取等待队列中待发送的协议
    virtual Protocol* get_wait_to_send_protocol(SocketHandle socket_handle);
    //获取等待队列中待发送的协议个数
    virtual int get_wait_to_send_protocol_number(SocketHandle socket_handle);
    //取消所有待发送协议,同时调用on_protocol_send_error通知应用层
    virtual int cancal_wait_to_send_protocol(SocketHandle socket_handle);
private:
    //待发送的协议队列
    SendTaskMap m_send_tasks_map;


////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数   //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
    //////////////////由应用层重写 接收协议函数//////////////////
    virtual int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)=0;
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

	//应用层所使用的io复用
	virtual IODemuxer* get_io_demuxer()=0;
	//应用层所使用的协议族
	virtual ProtocolFamily* get_protocol_family()=0;
};


#endif // _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

