/*
 * MTServerAppFramework.cpp
 *
 *  Created on: 2012-9-11
 *      Author: xl
 */

#include "ChunkWorker.h"
#include "IODemuxerEpoll.h"
#include "SFSProtocolFamily.h"
#include "slog.h"

//////////////////由应用层重写 创建IODemuxer//////////////////
IODemuxer* ChunkWorker::create_io_demuxer()
{
	return new EpollDemuxer;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ChunkWorker::delete_io_demuxer(IODemuxer* io_demuxer)
{
	delete io_demuxer;
}
//////////////////由应用层重写 创建SocketManager//////////////
SocketManager* ChunkWorker::create_socket_manager()
{
	return new SocketManager;
}
//////////////////由应用层重写 销毁IODemuxer//////////////////
void ChunkWorker::delete_socket_manager(SocketManager* socket_manager)
{
	delete socket_manager;
}
///////////////////  由应用层实现 创建协议族  //////////////////////////
ProtocolFamily* ChunkWorker::create_protocol_family()
{
	return new SFSProtocolFamily;
}
///////////////////  由应用层实现 销毁协议族  //////////////////////////
void ChunkWorker::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

//////////////////由应用层重写 接收协议函数//////////////////
bool ChunkWorker::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{

	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		return false;
	}

	return false;
}

bool ChunkWorker::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("on send protocol[details=%s] error. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkWorker::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("on send protocol[details=%s] succ. fd=%d, protocol=%x", protocol->details(), socket_handle, protocol);
	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkWorker::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("on socket handle error. fd=%d", socket_handle);
	return true;
}

bool ChunkWorker::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("on socket handle timeout. fd=%d", socket_handle);
	return true;
}

///////////////////////////////  thread pool  //////////////////////////////////
Thread<SocketHandle>* ChunkWorkerPool::create_thread()
{
	ChunkWorker *chunk_worker = new ChunkWorker;
	chunk_worker->start_instance();
	return chunk_worker;
}

/////////////////////////////// Timer Handler  /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_INFO("timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	return HANDLE_OK;
}
