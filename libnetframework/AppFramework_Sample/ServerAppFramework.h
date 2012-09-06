#ifndef SERVER_APP_FRAMEWORK_H_LiuYongjin_20120810
#define SERVER_APP_FRAMEWORK_H_LiuYongjin_20120810

#include "SocketManager.h"
#include "ProtocolDefault.h"

class ServerAppFramework: public SocketManager
{
public:
	ServerAppFramework(IODemuxer *io_demuxer, ProtocolFamily *protocol_family):SocketManager(io_demuxer, protocol_family){}
    //////////////////由应用层重写 接收协议函数//////////////////
    int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete);
    //////////////////由应用层重写 协议发送错误处理函数//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
    //////////////////由应用层重写 连接错误处理函数//////////////
    int on_socket_handle_error(SocketHandle socket_handle);
    //////////////////由应用层重写 连接超时处理函数//////////////
    int on_socket_handle_timeout(SocketHandle socket_handle);

public:    
    int send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp);
};

class TimerHandler:public EventHandler
{
public:
	TimerHandler(IODemuxer *demuxer):m_demuxer(demuxer){;}
	HANDLE_RESULT on_timeout(int fd)
	{
		SLOG_DEBUG("timer timeout...");
		m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	
		return HANDLE_OK;
	}
private:
	IODemuxer *m_demuxer;
};

#endif //APP_FRAMEWORK_H_LiuYongjin_20120810


