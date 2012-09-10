#include "ServerAppFramework.h"

#include <stdio.h>
#include <string.h>
#include "slog.h"

//////////////////由应用层重写 接收协议函数//////////////////
int ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			string data = string_protocol->get_string();
			SLOG_DEBUG("receive string protocol from fd=%d. receive data:[%s], length=%d", socket_handle, data.c_str(), data.length());

			Protocol* resp_protocol = ((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
			string temp = "server receive data:";
			temp += data;
			((StringProtocol*)resp_protocol)->set_string(temp);
			send_protocol(socket_handle, resp_protocol);
		}
		break;
	default:
		SLOG_DEBUG("receive undefine protocol. ignore it.");
		break;
	}
	
	return 0;
}

int ServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int ServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("server app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
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
