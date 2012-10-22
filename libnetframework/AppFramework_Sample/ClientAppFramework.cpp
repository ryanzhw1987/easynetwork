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
	return new StringProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void ClientAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

//////////////////由应用层重写 接收协议函数//////////////////
bool ClientAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	int type = header->get_protocol_type();
	switch(type)
	{
	case PROTOCOL_STRING:
		{
			int length = 0;
			char *recv_buffer = protocol->get_body_raw_data(length);
			SLOG_INFO("client recv resp:[%s], length=%d", recv_buffer, length);
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
	SLOG_ERROR("client app on send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ClientAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("client app on send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
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
	Protocol* protocol = protocol_family->create_protocol(PROTOCOL_STRING);
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	IOBuffer *send_buffer = new IOBuffer;
	char *header_buffer = send_buffer->write_open(header->get_header_length());
	send_buffer->write_close(header->get_header_length());
	char *body_buffer = send_buffer->write_open(100);
	sprintf(body_buffer, "client ping cmd");
	int size = strlen(body_buffer)+1;
	send_buffer->write_close(size);
	header->encode(header_buffer, size);
	protocol->attach_raw_data(send_buffer);
	m_app_framework->send_protocol(m_socket_handle, protocol);

	return HANDLE_OK;
}

