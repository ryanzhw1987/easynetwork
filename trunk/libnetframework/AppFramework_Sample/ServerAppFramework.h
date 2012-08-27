#ifndef SERVER_APP_FRAMEWORK_H_LiuYongjin_20120810
#define SERVER_APP_FRAMEWORK_H_LiuYongjin_20120810

#include "SocketManager.h"
#include "ProtocolDefault.h"
class ServerAppFramework: public SocketManager
{
public:
    //////////////////��Ӧ�ò���д ����Э�麯��//////////////////
    int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete);
    //////////////////��Ӧ�ò���д Э�鷢�ʹ�������//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////��Ӧ�ò���д Э�鷢�ͳɹ�������//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
    //////////////////��Ӧ�ò���д ���Ӵ�������//////////////
    int on_socket_handle_error(SocketHandle socket_handle);
    //////////////////��Ӧ�ò���д ���ӳ�ʱ������//////////////
    int on_socket_handle_timeout(SocketHandle socket_handle);

	//Ӧ�ò���ʹ�õ�io����
	IODemuxer* get_io_demuxer();
	//Ӧ�ò���ʹ�õ�Э����
	ProtocolFamily* get_protocol_family();
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


