#ifndef CLIENT_APP_FRAMEWORK_H_LiuYongjin_20120810
#define CLIENT_APP_FRAMEWORK_H_LiuYongjin_20120810

#include "SocketManager.h"
#include "StringProtocolFamily.h"
#include "NetInterface.h"

class ClientAppFramework: public NetInterface
{
protected:
	////由应用层实现----创建具体的协议族
	virtual ProtocolFamily* create_protocol_family();
	////由应用层实现----销毁协议族
	virtual void delete_protocol_family(ProtocolFamily* protocol_family);

	////由应用层实现----接收协议函数
	bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol);
	////由应用层实现----协议发送错误处理函数
	bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	////由应用层实现----协议发送成功处理函数
	bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	////由应用层实现----连接错误处理函数
	bool on_socket_handle_error(SocketHandle socket_handle);
	////由应用层实现----连接超时处理函数
	bool on_socket_handle_timeout(SocketHandle socket_handle);
	////由应用层实现----已经收到一个新的连接请求
	virtual bool on_socket_handler_accpet(SocketHandle socket_handle);
public:
	////由应用层实现----net interface实例启动入口
	bool start_server();
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


