/*
 * DownloadServer.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "MasterServer.h"
#include "IODemuxerEpoll.h"
#include "SFSProtocolFamily.h"
#include "slog.h"
#include <stdio.h>

/////////////////////////////////////// MasterServer ///////////////////////////////////////
bool MasterServer::start_server()
{
	//Init NetInterface
	init_net_interface();
	set_thread_ready();

	////Add your codes here
	///////////////////////
	get_io_demuxer()->run_loop();
	return true;
}

ProtocolFamily* MasterServer::create_protocol_family()
{
	return new SFSProtocolFamily;
}

void MasterServer::delete_protocol_family(ProtocolFamily* protocol_family)
{
	delete protocol_family;
}

bool MasterServer::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol)
{
	SFSProtocolFamily* protocol_family = (SFSProtocolFamily*)get_protocol_family();
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
	{
	case PROTOCOL_FILE_INFO:    //响应FileInfo请求
		{
			ProtocolFileInfoReq *protocol_file_info_req = (ProtocolFileInfoReq *)protocol;
			const string& fid = protocol_file_info_req->get_fid();
			bool query_chunkinfo = protocol_file_info_req->get_query_chunkinfo();
			SLOG_INFO("Thread[id=%d] receive FileInfo protocol.FID=%s, query=%d", get_thread_id(), fid.c_str(), query_chunkinfo?1:0);

			ProtocolFileInfo *protocol_fileinfo = (ProtocolFileInfo *)protocol_family->create_protocol(PROTOCOL_FILE_INFO);
			assert(protocol_fileinfo != NULL);

			FileInfo& file_info = protocol_fileinfo->get_fileinfo();
			if(!query_chunkinfo)
			{
				protocol_fileinfo->set_result(0);
				file_info.fid = fid;
				file_info.name = "test_name";
				file_info.size = 12345;
				ChunkPath chunk_path;
				chunk_path.id = "chunk0";
				chunk_path.addr = "127.0.0.1";
				chunk_path.port = 3013;
				chunk_path.index = 0;
				chunk_path.offset = 0;
				file_info.add_path(chunk_path);
			}
			else
			{

				file_info.fid = fid;
				file_info.name = ""; //无效
				file_info.size = 0;  //无效
				protocol_fileinfo->set_result(2);

				ChunkPath chunk_path;
				ChunkInfo chunk_info;
				if(get_chunk(chunk_info))
				{
					protocol_fileinfo->set_result(0);
					chunk_path.id = chunk_info.id;
					chunk_path.addr = chunk_info.addr;
					chunk_path.port = chunk_info.port;
					chunk_path.index = 0;  //无效
					chunk_path.offset = 0; //无效
					file_info.add_path(chunk_path);
				}
			}

			if(!send_protocol(socket_handle, protocol_fileinfo))
			{
				SLOG_ERROR("send FileInfoResp Protocol failed.");
				protocol_family->destroy_protocol(protocol_fileinfo);
			}
			break;
		}
	case PROTOCOL_CHUNK_PING:    //响应chunk的ping包
		{
			ProtocolChunkPing *protocol_chunkping = (ProtocolChunkPing *)protocol;
			ChunkInfo& chunk_info = protocol_chunkping->get_chunk_info();

			SLOG_INFO("Thread[ID=%d] receive ChunkPing protocol.ChunkId=%s, ChunkAddr=%s, ChunkPort=%d, DiskSpace=%lld, DiskUsed=%lld"
						,get_thread_id()
						,chunk_info.id.c_str()
						,chunk_info.addr.c_str()
						,chunk_info.port
						,chunk_info.disk_space
						,chunk_info.disk_used);

			ProtocolChunkPingResp *protocol_chunkping_resp = (ProtocolChunkPingResp *)protocol_family->create_protocol(PROTOCOL_CHUNK_PING_RESP);

			if(chunk_info.id == "" || chunk_info.addr == "")
			{
				SLOG_ERROR("Thread[ID=%d] chunk id or addr is empty.", get_thread_id());
				protocol_chunkping_resp->set_result(1);
			}
			else
			{
				add_chunk(chunk_info);
				protocol_chunkping_resp->set_result(0);
			}

			if(!send_protocol(socket_handle, protocol_chunkping_resp))
			{
				protocol_family->destroy_protocol(protocol_chunkping_resp);
				SLOG_ERROR("Thread[ID=%d] send Protocol_Chunkping_Resp failed.", get_thread_id());
			}
			break;
		}
	default:
		{
			SLOG_WARN("Thread[ID=%d] receive undefine protocol. ignore it.", get_thread_id());
			return false;
		}
	}

	return true;
}

bool MasterServer::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("Thread[ID=%d] send protocol[details=%s] error. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MasterServer::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_INFO("Thread[ID=%d] send protocol[details=%s] succ. fd=%d, protocol=%x", get_thread_id(), protocol->details(), socket_handle, protocol);
	//Add your code to handle the protocol
	//////////////////////////////////////

	get_protocol_family()->destroy_protocol(protocol);
	return true;
}

bool MasterServer::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket error. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket error
	//////////////////////////////////////////

	return true;
}

bool MasterServer::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_INFO("Thread[ID=%d] handle socket timeout. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle the socket timeout
	////////////////////////////////////////////

	return true;
}

bool MasterServer::on_socket_handler_accpet(SocketHandle socket_handle)
{
	SLOG_DEBUG("Thread[ID=%d] handle new socket. fd=%d", get_thread_id(), socket_handle);
	//Add your code to handle new socket
	////////////////////////////////////

	return true;
}

void MasterServer::add_chunk(ChunkInfo &chunk_info)
{
	map<string, ChunkInfo>::iterator it = m_chunk_manager.find(chunk_info.id);
	if(it == m_chunk_manager.end())
	{
		SLOG_DEBUG("Thread[ID=%d] add a new chunk info. chunk ID=%s. total chunk:%d.", get_thread_id(), chunk_info.id.c_str(), m_chunk_manager.size());
		m_chunk_manager.insert(std::make_pair(chunk_info.id, chunk_info));
	}
	else
	{
		SLOG_DEBUG("Thread[ID=%d] update a chunk info. chunk ID=%s. total chunk=%d.", get_thread_id(), chunk_info.id.c_str(), m_chunk_manager.size());
		it->second = chunk_info;
	}
}

bool MasterServer::get_chunk(ChunkInfo &chunk_info)
{
	SLOG_DEBUG("Thread[ID=%d] total chunk=%d.", get_thread_id(), m_chunk_manager.size());
	map<string, ChunkInfo>::iterator it = m_chunk_manager.begin();
	if(it != m_chunk_manager.end())
	{
		chunk_info = it->second;
		return true;
	}

	return false;
}

/////////////////////////////////////// MasterThreadPool ///////////////////////////////////////
Thread<SocketHandle>* MasterThreadPool::create_thread()
{
	MasterServer* temp = new MasterServer();
	temp->set_idle_timeout(30000);
	return (Thread<SocketHandle>*)temp;
}

