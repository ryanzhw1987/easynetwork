#include "ServerAppFramework.h"

#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "slog.h"


int ServerAppFramework::send_cmd(SocketHandle socket_handle, Command* cmd, bool has_resp)
{
    DefaultProtocolFamily *default_protocol_family = (DefaultProtocolFamily*)get_protocol_family();
	Protocol *protocol = default_protocol_family->create_protocol(cmd);
	send_protocol(socket_handle, protocol, has_resp);
	return 0;
}
	
//////////////////由应用层重写 接收协议函数//////////////////
int ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)
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

int ServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int ServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int ServerAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle error. fd=%d", socket_handle);
	return 0;
}

int ServerAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle timeout. fd=%d", socket_handle);
	return 0;
}

//应用层所使用的io复用
IODemuxer* ServerAppFramework::get_io_demuxer()
{
	static EpollDemuxer epoll_demuxer;
	return &epoll_demuxer;
}

//应用层所使用的协议族
ProtocolFamily* ServerAppFramework::get_protocol_family()
{
	static DefaultProtocolFamily default_protocol_family;
	return &default_protocol_family;
}
