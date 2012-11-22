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
			ProtocolFileInfo *protocol_fileinfo = (ProtocolFileInfo *)protocol;
			const string& fid = protocol_fileinfo->get_fid();
			bool query_chunkinfo = protocol_fileinfo->get_query_chunkinfo();
			SLOG_INFO("Thread[id=%d] receive FileInfo protocol.FID=%s, query=%d", get_thread_id(), fid.c_str(), query_chunkinfo?1:0);

			ProtocolFileInfoResp *protocol_fileinfo_resp = (ProtocolFileInfoResp *)protocol_family->create_protocol(PROTOCOL_FILE_INFO_RESP);
			assert(protocol_fileinfo_resp != NULL);

			ChunkInfo chunk_info;
			if(!query_chunkinfo)
			{
				protocol_fileinfo_resp->set_result(1);
				protocol_fileinfo_resp->set_fid(fid);
				chunk_info.path = fid + "_chunk0_location";
				chunk_info.chunk_addr = "127.0.0.1";
				chunk_info.port = 3013;
			}
			else
			{
				ChunkPingInfo chunk_ping_info;
				if(!get_chunk(chunk_ping_info))
				{
					protocol_fileinfo_resp->set_result(0);
				}
				else
				{
					protocol_fileinfo_resp->set_result(2);
					protocol_fileinfo_resp->set_fid(fid);
					chunk_info.chunk_addr = chunk_ping_info.addr;
					chunk_info.port = chunk_ping_info.port;
				}
			}

			protocol_fileinfo_resp->add_chunkinfo(chunk_info);
			if(!send_protocol(socket_handle, protocol_fileinfo_resp))
			{
				SLOG_ERROR("send FileInfoResp Protocol failed.");
				protocol_family->destroy_protocol(protocol_fileinfo_resp);
			}
			break;
		}
	case PROTOCOL_CHUNK_PING:    //响应chunk的ping包
		{
			ProtocolChunkPing *protocol_chunkping = (ProtocolChunkPing *)protocol;
			ChunkPingInfo chunk_info;
			chunk_info.id = protocol_chunkping->get_chunk_id();
			chunk_info.addr = protocol_chunkping->get_chunk_addr();
			chunk_info.port = protocol_chunkping->get_chunk_port();
			chunk_info.disk_space = protocol_chunkping->get_disk_space();
			chunk_info.disk_used = protocol_chunkping->get_disk_used();

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
				protocol_chunkping_resp->set_result(0);
			}
			else
			{
				add_chunk(chunk_info);
				protocol_chunkping_resp->set_result(1);
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

void MasterServer::add_chunk(ChunkPingInfo &chunk_info)
{
	map<string, ChunkPingInfo>::iterator it = m_chunk_manager.find(chunk_info.id);
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

bool MasterServer::get_chunk(ChunkPingInfo &chunk_info)
{
	SLOG_DEBUG("Thread[ID=%d] total chunk=%d.", get_thread_id(), m_chunk_manager.size());
	map<string, ChunkPingInfo>::iterator it = m_chunk_manager.begin();
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

