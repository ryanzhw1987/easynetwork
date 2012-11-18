#include "ServerAppFramework.h"
#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "Socket.h"
#include "ListenHandler.h"
#include "StringProtocolFamily.h"

bool ServerAppFramework::start_server()
{
	////Init NetInterface
	init_net_interface();

	////Add Your Codes From Here
	ListenSocket linsten_socket(3010);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return -1;
	}

	//listen event
	ListenHandler listen_handler(this);
	get_io_demuxer()->register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);
	//timer event
	TimerHandler timer(get_io_demuxer());
	get_io_demuxer()->register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	get_io_demuxer()->run_loop();

	return true;
}

ProtocolFamily* ServerAppFramework::create_protocol_family()
{
	return new StringProtocolFamily;
}

void ServerAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

bool ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	DefaultProtocolFamily* protocol_family = (DefaultProtocolFamily *)get_protocol_family();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			SLOG_INFO("receive string protocol from fd=%d. receive data:[%s].", socket_handle, string_protocol->get_string().c_str());

			StringProtocol *resp_protocol = (StringProtocol *)protocol_family->create_protocol(PROTOCOL_STRING);
			string str = "server respond";
			resp_protocol->set_string(str);
			//发送协议
			if(!send_protocol(socket_handle, resp_protocol))
			{
				SLOG_ERROR("send protocol failed.");
				protocol_family->destroy_protocol(resp_protocol);
			}
		}
		break;
	default:
		SLOG_ERROR("receive undefine protocol. ignore it.");
		return false;
	}

	return true;
}

bool ServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol[detail=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("server app on send protocol[detail=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ServerAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool ServerAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle timeout. fd=%d", socket_handle);
	return true;
}

bool ServerAppFramework::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle accpet. fd=%d", socket_handle);
	return true;
}

//////////////////////////////  Timer Handler  /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_DEBUG("server timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);

	return HANDLE_OK;
}

