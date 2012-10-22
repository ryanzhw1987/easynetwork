#ifndef CLIENT_APP_FRAMEWORK_H_LiuYongjin_20120810
#define CLIENT_APP_FRAMEWORK_H_LiuYongjin_20120810

#include "SocketManager.h"
#include "ProtocolDefault.h"
#include "NetInterface.h"

class ClientAppFramework: public NetInterface
{
protected:
	//////////////////由应用层重写 创建IODemuxer//////////////////
	virtual IODemuxer* create_io_demuxer();
	//////////////////由应用层重写 销毁IODemuxer//////////////////
	virtual void delete_io_demuxer(IODemuxer* io_demuxer);
	//////////////////由应用层重写 创建SocketManager//////////////
	virtual SocketManager* create_socket_manager();
	//////////////////由应用层重写 销毁SocketManager//////////////
	virtual void delete_socket_manager(SocketManager* socket_manager);
	//////////////////由应用层重写 创建具体的协议族//////////////
	virtual ProtocolFamily* create_protocol_family();
	//////////////////由应用层重写 销毁协议族////////////////////
	virtual void delete_protocol_family(ProtocolFamily* protocol_family);

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


class PingHandler:public EventHandler
{
public:
	PingHandler(ClientAppFramework *app_framework, SocketHandle socket_handle):m_app_framework(app_framework), m_socket_handle(socket_handle){}
	void register_handler();
	HANDLE_RESULT on_timeout(int fd);
private:
	ClientAppFramework *m_app_framework;
	SocketHandle m_socket_handle;
};

#endif //APP_FRAMEWORK_H_LiuYongjin_20120810


