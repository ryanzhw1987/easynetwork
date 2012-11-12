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
	SFSProtocolFamily* protocol_family = (SFSProtocolFamily*)get_protocol_family();
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_STORE:    //client 请求存储文件
	{
		ProtocolStore *protocol_store = (ProtocolStore *)protocol;
		const string &fid = protocol_store->get_fid();
		const string &filename = protocol_store->get_file_name();
		uint64_t filesize = protocol_store->get_file_size();
		uint64_t segoffset = protocol_store->get_seg_offset();
		int segindex = protocol_store->get_seg_index();
		int segsize = protocol_store->get_seg_size();
		bool segfinished = protocol_store->get_seg_finished();

		SLOG_INFO("receive Store Protocol. FID=%s, Filename=%s, Filesize=%lld, SegOffset=%lld, SegIndex=%d, SegSize=%d, SegFinished=%d"
					,fid.c_str(), filename.c_str(), filesize, segoffset, segindex, segsize, segfinished?1:0);

		ProtocolStoreResp* protocol_store_resp = (ProtocolStoreResp*)protocol_family->create_protocol(PROTOCOL_STORE_RESP);
		assert(protocol_store_resp != NULL);
		protocol_store_resp->set_result(1);
		protocol_store_resp->set_fid(fid);
		string chunk_path = fid+"_chunk0_0_filesize";
		protocol_store_resp->set_chunk_path(chunk_path);

		if(!send_protocol(socket_handle, protocol_store_resp))
		{
			SLOG_ERROR("send StoreResp Protocol failed.");
			protocol_family->destroy_protocol(protocol_store_resp);
		}
		break;
	}
	case PROTOCOL_CHUNK_REPORT_RESP:    //master回复上报存储结果
	{
		break;
	}
	default:
		SLOG_WARN("receive undefine protocol. ignore it.");
		return false;
	}

	return true;
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
