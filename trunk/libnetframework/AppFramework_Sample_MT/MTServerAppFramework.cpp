/*
 * MTServerAppFramework.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "MTServerAppFramework.h"
#include "slog.h"
#include "IODemuxerEpoll.h"

bool MTServerAppFramework::start_server()
{
	//初始化NetInterface
	init_net_interface();

	//// Add Your Codes From Here
	SLOG_INFO("Start server.");
	get_io_demuxer()->run_loop();
	return true;
}

ProtocolFamily* MTServerAppFramework::create_protocol_family()
{
	return new StringProtocolFamily;
}

void MTServerAppFramework::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

bool MTServerAppFramework::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_STRING:
		{
			StringProtocol* string_protocol = (StringProtocol*)protocol;
			string &recv_string = string_protocol->get_string();
			SLOG_INFO("thread[ID=%d] receive string protocol from fd=%d. receive data:[%s], length=%d", get_id(), socket_handle, recv_string.c_str(), recv_string.size());

			StringProtocol* resp_protocol = (StringProtocol*)((DefaultProtocolFamily*)get_protocol_family())->create_protocol(PROTOCOL_STRING);
			string send_string = "server receive data:";
			send_string += recv_string;
			((StringProtocol*)resp_protocol)->set_string(send_string);
			send_protocol(socket_handle, resp_protocol);
		}
		break;
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		return false;
	}

	return false;
}

bool MTServerAppFramework::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("server app on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MTServerAppFramework::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("server app on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MTServerAppFramework::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle error. fd=%d", socket_handle);
	return true;
}

bool MTServerAppFramework::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("server app on socket handle timeout. fd=%d", socket_handle);
	return true;
}

bool MTServerAppFramework::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("server app on socket handle accpet. fd=%d", socket_handle);
	return true;
}

///////////////////////////////  thread pool  //////////////////////////////////
Thread<SocketHandle>* MTServerAppFrameworkPool::create_thread()
{
	MTServerAppFramework *app = new MTServerAppFramework;
	return app;
}

/////////////////////////////// Timer Handler  /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_INFO("timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	return HANDLE_OK;
}
