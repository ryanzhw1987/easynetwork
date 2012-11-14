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

///////////////////////////////  ChunkWorker  //////////////////////////////////
bool ChunkWorker::start_server()
{
	//Init NetInterface
	init_net_interface();
	set_thread_ready();

	////Add your codes here
	///////////////////////
	get_io_demuxer()->run_loop();
	return true;
}

ProtocolFamily* ChunkWorker::create_protocol_family()
{
	return new SFSProtocolFamily;
}

void ChunkWorker::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

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
	SLOG_ERROR("Thread[ID=%d] send protocol[details=%s] error. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkWorker::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("Thread[ID=%d] send protocol[details=%s] succ. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool ChunkWorker::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket error. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket error
	//////////////////////////////////////////

	return true;
}

bool ChunkWorker::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket timeout. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket timeout
	////////////////////////////////////////////

	return true;
}

bool ChunkWorker::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("Thread[ID=%d] handle new socket. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle new socket
	////////////////////////////////////

	return true;
}

///////////////////////////////  ChunkWorkerPool  //////////////////////////////////
Thread<SocketHandle>* ChunkWorkerPool::create_thread()
{
	ChunkWorker *chunk_worker = new ChunkWorker;
	return chunk_worker;
}

/////////////////////////////// Timer Handler  /////////////////////////////////
HANDLE_RESULT TimerHandler::on_timeout(int fd)
{
	SLOG_INFO("timer timeout...");
	m_demuxer->register_event(-1, EVENT_INVALID, 3000, this);
	return HANDLE_OK;
}
