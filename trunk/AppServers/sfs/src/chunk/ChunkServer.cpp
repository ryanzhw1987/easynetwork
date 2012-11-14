#include "ChunkServer.h"
#include "IODemuxerEpoll.h"
#include "slog.h"
#include "ChunkWorker.h"
#include "ListenHandler.h"
#include "Socket.h"

#include <stdio.h>
#include <string.h>

ChunkServer::ChunkServer()
{
	m_master_socket_handle = SOCKET_INVALID;
}

ChunkServer::~ChunkServer()
{}

bool ChunkServer::start_server()
{
	//Init NetInterface
	init_net_interface();

	////Add your codes here
	///////////////////////
	IODemuxer *io_demuxer = get_io_demuxer();
	//监听端口
	ListenSocket linsten_socket(3013);
	if(!linsten_socket.open())
	{
		SLOG_ERROR("listen on port:3010 error.");
		return false;
	}

	//chunk worker pool
	ChunkWorkerPool server_pool(3);
	server_pool.start();
	ListenHandler listen_handler(&server_pool);
	io_demuxer->register_event(linsten_socket.get_handle(), EVENT_READ|EVENT_PERSIST, -1, &listen_handler);

	//注册定时器
	if(io_demuxer->register_event(-1, EVENT_PERSIST, 3000, this) == -1)
	{
		SLOG_ERROR("register timer handler failed.");
		return false;
	}

	//创建到master的链接
	m_master_socket_handle = get_active_trans_socket("127.0.0.1", 3012);  //创建主动连接
	if(m_master_socket_handle == SOCKET_INVALID)
	{
		SLOG_ERROR("connect master failed");
		return false;
	}

	get_io_demuxer()->run_loop();
	return true;
}

ProtocolFamily* ChunkServer::create_protocol_family()
{
	return new SFSProtocolFamily;
}

void ChunkServer::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

//定时发送ping包
HANDLE_RESULT ChunkServer::on_timeout(int fd)
{
	//发送ping包到master
	SFSProtocolFamily* protocol_family = (SFSProtocolFamily*)get_protocol_family();
	ProtocolChunkPing *protocol_chunk_ping = (ProtocolChunkPing *)protocol_family->create_protocol(PROTOCOL_CHUNK_PING);
	assert(protocol_chunk_ping != NULL);

	string chunk_id = "chunk0";
	uint64_t disk_space = 123456789;
	uint64_t disk_used = 2342234;

	protocol_chunk_ping->set_chunk_id(chunk_id);
	protocol_chunk_ping->set_chunk_port(3013);
	protocol_chunk_ping->set_disk_space(disk_space);
	protocol_chunk_ping->set_disk_used(disk_used);

	if(!send_protocol(m_master_socket_handle, protocol_chunk_ping))
	{
		SLOG_ERROR("send ChunkPing to master failed.");
		protocol_family->destroy_protocol(protocol_chunk_ping);
	}

	return HANDLE_OK;
}

//////////////////由应用层重写 接收协议函数//////////////////
bool ChunkServer::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)protocol->get_protocol_header();
	int type = header->get_protocol_type();
	switch(type)
	{
	case PROTOCOL_CHUNK_PING_RESP:
	{
		ProtocolChunkPingResp *protocol_chunkping_resp = (ProtocolChunkPingResp *)protocol;
		SLOG_INFO("receive ChunPingResp Protocol from master. result=%d.", protocol_chunkping_resp->get_result());
		break;
	}
	default:
		SLOG_ERROR("receive undefine protocol. ignore it.");
		return false;
	}

	return true;
}

bool ChunkServer::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("Send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkServer::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("Send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkServer::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("Handle socket error. fd=%d", socket_handle);
	//Add your code to handle the socket error
	//////////////////////////////////////////

	return true;
}

bool ChunkServer::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("Handle socket timeout. fd=%d", socket_handle);
	//Add your code to handle the socket timeout
	////////////////////////////////////////////

	return true;
}

bool ChunkServer::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("Handle new socket. fd=%d", socket_handle);
	//Add your code to handle new socket
	////////////////////////////////////

	return true;
}

