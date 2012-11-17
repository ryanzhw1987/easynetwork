#include "ClientAppFramework.h"

#include <stdio.h>
#include <string.h>

#include "slog.h"

bool ClientAppFramework::start_server()
{
	////Init NetInterface
	init_net_interface();

	////Add Your Codes From Here
	SocketHandle socket_handle = get_active_trans_socket("127.0.0.1", 3010);  //创建主动连接
	if(socket_handle == SOCKET_INVALID)
		return false;

	PingHandler ping_handler(this, socket_handle);
	ping_handler.register_handler();

	get_io_demuxer()->run_loop();

	return true;
}

ProtocolFamily* ClientAppFramework::create_protocol_family()
{
	return new StringProtocolFamily;
}

void ClientAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

bool ClientAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	int type = header->get_protocol_type();
	switch(type)
	{
	case PROTOCOL_STRING:
		{
			StringProtocol *str_protocol = (StringProtocol *)protocol;
			SLOG_INFO("client recv resp:[%s].", str_protocol->get_string().c_str());
		}
		break;
	default:
		SLOG_ERROR("receive undefine protocol. ignore it.");
		return false;
	}

	//get_io_demuxer()->exit();
	return true;
}

bool ClientAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("client app on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ClientAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("client app on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ClientAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("client app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool ClientAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("client app on socket handle timeout. fd=%d", socket_handle);
	get_io_demuxer()->exit();
	return true;
}

bool ClientAppFramework::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("client app on socket handle accpet. fd=%d", socket_handle);
	return true;
}

//////////////////////////////////////////  发送ping包  /////////////////////////////////
void PingHandler::register_handler()
{
	IODemuxer *io_demuxer = m_app_framework->get_io_demuxer();
	io_demuxer->register_event(-1, EVENT_PERSIST, 3000, this);
}

HANDLE_RESULT PingHandler::on_timeout(int fd)
{
	static int count = 0;
	SLOG_DEBUG("ping handler timeout. count=%d", ++count);

	DefaultProtocolFamily *protocol_family = (DefaultProtocolFamily *)m_app_framework->get_protocol_family();
	StringProtocol *str_protocol = (StringProtocol *)protocol_family->create_protocol(PROTOCOL_STRING);
	assert(str_protocol != NULL);

	string str = "client ping protocol";
	str_protocol->set_string(str);
	//发送协议
	if(!m_app_framework->send_protocol(m_socket_handle, str_protocol))
	{
		SLOG_ERROR("send ping protocol failed.");
		protocol_family->destroy_protocol(str_protocol);
	}

	return HANDLE_OK;
}

