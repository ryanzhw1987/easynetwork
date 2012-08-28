#include <stdio.h>
#include <string.h>
#include "IODemuxerEpoll.h"
#include "SocketManager.h"
#include "ProtocolDefault.h"

#include "slog.h"

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

//应用程序框架
//重写父类成员函数recv_protocol,实现业务层逻辑
class AppFramework: public SocketManager
{
public:
	int send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp)
	{
		DefaultProtocolFamily *protocol_family = (DefaultProtocolFamily*)get_protocol_family();
		DefaultProtocol *protocol = (DefaultProtocol*)protocol_family->create_protocol(cmd);
		send_protocol(socket_handle, protocol, has_resp);
		return 0;
	}

	//重写父类函数,实现业务层逻辑
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)
	{
		DefaultProtocol* default_protocol = (DefaultProtocol*)protocol;
		ProtocolType type = default_protocol->get_type();
		switch(type)
		{
		case PROTOCOL_SIMPLE:
			{
				SimpleCmd* simple_cmd = (SimpleCmd*)default_protocol->get_cmd();
				SLOG_DEBUG("receive simple cmd. recevie data:%s.", simple_cmd->get_data());

				SimpleCmd* cmd = new SimpleCmd();
				char str[1024];
				sprintf(str,"server resp.[receive data:%s]",simple_cmd->get_data());
				int size = strlen(str)+1;
				cmd->set_data(str, size);

				send_cmd(socket_handle, cmd, false);				
			}
			break;
		default:
			SLOG_DEBUG("reveive undefine cmd.");
			break;
		}
		
		return 0;
	}

	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
	{
		SLOG_ERROR("app send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
		delete protocol;
		return 0;
	}

	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
	{
		SLOG_DEBUG("app send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
		delete protocol;
		return 0;
	}

	int on_socket_handle_error(SocketHandle socket_handle)
	{
		SLOG_DEBUG("app socket handle error. fd=%d", socket_handle);
		return 0;
	}

	int on_socket_handle_timeout(SocketHandle socket_handle)
	{
		SLOG_DEBUG("app socket handle error. fd=%d", socket_handle);
		return 0;
	}

	//应用层所使用的io复用
	IODemuxer* get_io_demuxer()
	{
		static EpollDemuxer epoll_demuxer;
		return &epoll_demuxer;
	}

	//应用层所使用的协议族
	ProtocolFamily* get_protocol_family()
	{
		static DefaultProtocolFamily default_protocol_family;
		return &default_protocol_family;
	}
};

int main()
{
	SLOG_INIT(SLOG_LEVEL_DEBUG, NULL, 0);
	
	AppFramework app_framework;
	//SocketManager *socket_manager = new SocketManager(&demuxer, &protocol_family);
	SocketManager *socket_manager = &app_framework;
	
	if(socket_manager->listen(3010) == -1)
	{
		return -1;
	}

	IODemuxer *io_demuxer = app_framework.get_io_demuxer();
	TimerHandler timer(io_demuxer);	
	io_demuxer->register_event(-1, EVENT_INVALID, 3000, &timer);	

	io_demuxer->run_loop();

	SLOG_UNINIT();
	return 0;
}



