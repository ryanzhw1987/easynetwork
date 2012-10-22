#include "ClientAppFramework.h"

#include <stdio.h>
#include <string.h>

#include "IODemuxerEpoll.h"
#include "slog.h"

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* ClientAppFramework::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ClientAppFramework::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* ClientAppFramework::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ClientAppFramework::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* ClientAppFramework::create_protocol_family()
{
	return new DefaultProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void ClientAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

//////////////////由应用层重写 接收协议函数//////////////////
int ClientAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)
{
	DefaultProtocol* default_protocol = (DefaultProtocol*)protocol;
	ProtocolType type = default_protocol->get_type();
	switch(type)
	{
	case PROTOCOL_STRING:
		{
			string resp = ((StringProtocol*)protocol)->get_string();
			SLOG_INFO("receive from server:[%s]", resp.c_str());
		}
		break;
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		break;
	}

	//get_io_demuxer()->exit();
	return 0;
}

int ClientAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("client app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return 0;
}

int ClientAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("client app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
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
	Protocol* resp_protocol = protocol_family->create_protocol(PROTOCOL_STRING);
	((StringProtocol*)resp_protocol)->set_string("This is a ping protocol");
	m_app_framework->send_protocol(m_socket_handle, resp_protocol);

	return HANDLE_OK;
}

