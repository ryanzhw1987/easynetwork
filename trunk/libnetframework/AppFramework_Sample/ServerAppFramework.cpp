#include "ServerAppFramework.h"
#include <stdio.h>
#include <string.h>

#include "slog.h"
#include "Socket.h"
#include "ListenHandler.h"
#include "StringProtocolFamily.h"
#include "IODemuxerEpoll.h"

bool ServerAppFramework::run_server()
{
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
	//TimerHandler timer(get_io_demuxer());
	//get_io_demuxer()->register_event(-1, EVENT_INVALID, 3000, &timer);

	//run server forever
	get_io_demuxer()->run_loop();

	return true;
}

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* ServerAppFramework::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ServerAppFramework::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* ServerAppFramework::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ServerAppFramework::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* ServerAppFramework::create_protocol_family()
{
	return new StringProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void ServerAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

//////////////////由应用层重写 接收协议函数//////////////////
bool ServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			int recv_length = 0;
			char *recv_data = protocol->get_body_raw_data(recv_length);
			SLOG_INFO("receive string protocol from fd=%d. receive data:[%s], length=%d", socket_handle, recv_data, recv_length);

			Protocol* resp_protocol = ((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
			header = (DefaultProtocolHeader*)resp_protocol->get_protocol_header();
			int header_length = header->get_header_length();
			ByteBuffer *send_buffer = new ByteBuffer;
			//预留协议头空间
			send_buffer->get_append_buffer(header_length);
			send_buffer->set_append_size(header->get_header_length());
			//编码协议体
			char *body_buffer = send_buffer->get_append_buffer(recv_length+100);
			snprintf(body_buffer, recv_length+100, "server receive data:[%s]", recv_data);
			int body_length = strlen(body_buffer)+1;
			send_buffer->set_append_size(body_length);
			//编码协议头
			char *header_buffer = send_buffer->get_data();
			header->encode(header_buffer, body_length);
			//attach编码后的数据
			resp_protocol->attach_raw_data(send_buffer);
			//发送协议
			send_protocol(socket_handle, resp_protocol);
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

//////////////////////////////  Timer Handler  /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_DEBUG("server timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);

	return HANDLE_OK;
}

