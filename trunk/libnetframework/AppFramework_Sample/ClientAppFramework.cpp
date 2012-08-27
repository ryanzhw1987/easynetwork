#include "ClientAppFramework.h"

#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "slog.h"


int ClientAppFramework::send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp)
{
    DefaultProtocolFamily *default_protocol_family = (DefaultProtocolFamily*)get_protocol_family();
	Protocol *protocol = default_protocol_family->create_protocol(cmd);
	send_protocol(socket_handle, protocol, has_resp);
	return 0;
}

//////////////////由应用层重写 接收协议函数//////////////////
int ClientAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)
{
	DefaultProtocol* default_protocol = (DefaultProtocol*)protocol;
	ProtocolType type = default_protocol->get_type();
	switch(type)
	{
	case PROTOCOL_SIMPLE:
		{
			SimpleCmd* simple_cmd = (SimpleCmd*)default_protocol->get_cmd();
			SLOG_DEBUG("receive server resp. simple cmd. recevie data:%s.", simple_cmd->get_data());
		}
		break;
	default:
		SLOG_DEBUG("reveive undefine cmd.");
		break;
	}

	//get_io_demuxer()->exit();
	return 0;
}

int ClientAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("client app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int ClientAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("client app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int ClientAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("client app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int ClientAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("client app on socket handle timeout. fd=%d", socket_handle);
	get_io_demuxer()->exit();
	return 0;
}

//应用层所使用的io复用
IODemuxer* ClientAppFramework::get_io_demuxer()
{
	static EpollDemuxer epoll_demuxer;
	return &epoll_demuxer;
}

//应用层所使用的协议族
ProtocolFamily* ClientAppFramework::get_protocol_family()
{
	static DefaultProtocolFamily default_protocol_family;
	return &default_protocol_family;
}

//////////////////////////////////////////  发送ping包  /////////////////////////////////
void PingHandler::register_handler()
{
	IODemuxer *io_demuxer = m_app_framework->get_io_demuxer();
	io_demuxer->register_event(-1, EVENT_PERSIST, 2000, this);
}

HANDLE_RESULT PingHandler::on_timeout(int fd)
{
	SLOG_DEBUG("ping handler timeout.");
	char buf[100];
	sprintf(buf, "ping cmd");
	int len = strlen(buf)+1;

	SimpleCmd *simple_cmd = new SimpleCmd;
	simple_cmd->set_data(buf, len);
	m_app_framework->send_cmd(m_socket_handle, (Command*)simple_cmd, false);

	return HANDLE_OK;
}

