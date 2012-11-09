#include "ChunkServer.h"
#include "IODemuxerEpoll.h"
#include "slog.h"

#include <stdio.h>
#include <string.h>

ChunkServer::ChunkServer()
{
	m_master_socket_handle = SOCKET_INVALID;
}

ChunkServer::~ChunkServer()
{}

bool ChunkServer::run_server()
{
	//注册定时器
	IODemuxer *io_demuxer = get_io_demuxer();
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

	io_demuxer->run_loop();
	return true;
}

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* ChunkServer::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ChunkServer::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* ChunkServer::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ChunkServer::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* ChunkServer::create_protocol_family()
{
	return new SFSProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
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
	SLOG_ERROR("on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkServer::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkServer::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("on socket handle error. fd=%d", socket_handle);
	return true;
}

bool ChunkServer::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("on socket handle timeout. fd=%d", socket_handle);
	return true;
}


