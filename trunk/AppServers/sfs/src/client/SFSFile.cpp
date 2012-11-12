/*
 * SFSClient.cpp
 *
 *  Created on: 2012-11-8
 *      Author: LiuYongJin
 */

#include "SFSFile.h"
#include "slog.h"
#include "TransProtocol.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace SFS;

File::File(string &master_addr, int master_port, int n_replica)
	:m_master_addr(master_addr)
	,m_master_port(master_port)
	,m_n_replica(n_replica)
{}

bool File::file_info(FileInfo *fileinfo, string &fid, bool query_chunkinfo/*=false*/)
{
	if(fileinfo == NULL)
		return false;

	//协议数据
	ProtocolFileInfo* protocol_fileinfo = (ProtocolFileInfo*)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO);
	assert(protocol_fileinfo != NULL);
	protocol_fileinfo->set_fid(fid);
	protocol_fileinfo->set_query_chunkinfo(query_chunkinfo);
	//回复
	ProtocolFileInfoResp *protocol_fileinfo_resp = (ProtocolFileInfoResp *)query_master(protocol_fileinfo);
	m_protocol_family.destroy_protocol(protocol_fileinfo);
	if(protocol_fileinfo_resp == NULL)
		return false;
	fileinfo->result = protocol_fileinfo_resp->get_result();
	fileinfo->fid = protocol_fileinfo_resp->get_fid();
	fileinfo->size = protocol_fileinfo_resp->get_fielsize();
	fileinfo->chunkinfo = protocol_fileinfo_resp->get_chunkinfo();
	m_protocol_family.destroy_protocol(protocol_fileinfo_resp);

	return true;
}

bool File::store(string &local_file)
{
	string fid="AAACCCDDD";
	FileInfo fileinfo;
	if(file_info(&fileinfo, fid, true))
	{
		SLOG_DEBUG("result:%d FID:%s FileSize:%lld.", fileinfo.result, fileinfo.fid.c_str(), fileinfo.size);
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			SLOG_DEBUG("ChunkInfo:ChunkPath:%s ChunkAdd:%s ChunkPort:%d.",it->path.c_str(), it->chunk_addr.c_str(), it->port);
	}

	switch(fileinfo.result)
	{
	case 0:
	{
		printf("store failed.");
		return false;
	}
	case 1:
	{
		SLOG_DEBUG("file already exist. FID:%s FileSize:%lld.", fileinfo.fid.c_str(), fileinfo.size);
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			SLOG_DEBUG("ChunkInfo:ChunkPath:%s ChunkAdd:%s ChunkPort:%d.",it->path.c_str(), it->chunk_addr.c_str(), it->port);
		break;
	}
	case 2:
	{
		SLOG_DEBUG("request chunk to store file. fid=%s.", fid.c_str());
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			query_chunk_store(local_file, fid, it->chunk_addr, it->port);
		break;
	}
	default:
	{
		SLOG_WARN("unknow result value. result=%d.", fileinfo.result);
		return false;
	}
	}

	return true;
}

bool File::retrieve(string &fid, ByteBuffer *out_buf)
{
	return true;
}

///////////////////////////////////////////////////////
Protocol* File::query_master(Protocol *protocol)
{
	TransSocket trans_socket(m_master_addr.c_str(), m_master_port);
	if(!trans_socket.open(1000))
	{
		SLOG_ERROR("connect sfs failed.");
		return NULL;
	}

	//发送协议
	if(!TransProtocol::send_protocol(&trans_socket, protocol))
		return NULL;
	//接收协议
	ProtocolFileInfoResp *protocol_fileinfo_resp = (ProtocolFileInfoResp *)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO_RESP);
	assert(protocol_fileinfo_resp != NULL);
	if(!TransProtocol::recv_protocol(&trans_socket, protocol_fileinfo_resp))
	{
		m_protocol_family.destroy_protocol(protocol_fileinfo_resp);
		protocol_fileinfo_resp = NULL;
	}
	return protocol_fileinfo_resp;
}

bool File::send_store_protocol(TransSocket* trans_socket, ProtocolStore *protocol_store, ByteBuffer *byte_buffer, int fd)
{
	byte_buffer->clear();

	//1 预留头部空间
	int header_length;
	ProtocolHeader *header = protocol_store->get_protocol_header();
	header_length = header->get_header_length();
	byte_buffer->reserve(header_length);
	//2 编码协议体
	if(!protocol_store->encode_body(byte_buffer))
	{
		SLOG_ERROR("encode body error");
		return false;
	}
	//3 添加数据
	int seg_size = protocol_store->get_seg_size();
	char *data_buffer = byte_buffer->get_append_buffer(seg_size);
	if(read(fd, data_buffer, seg_size) != seg_size)
	{
		SLOG_ERROR("read file error. errno=%d(%s).", errno, strerror(errno));
		return false;
	}
	byte_buffer->set_append_size(seg_size);
	//4. 编码协议头
	int body_length = byte_buffer->size()-header_length;
	char *header_buffer = byte_buffer->get_data(0, header_length);
	if(!header->encode(header_buffer, body_length))
	{
		SLOG_ERROR("encode header error");
		return false;
	}
	//5. 发送数据
	if(trans_socket->send_data_all(byte_buffer->get_data(), byte_buffer->size()) == TRANS_ERROR)
	{
		SLOG_ERROR("send data error");
		return false;
	}

	return true;
}

bool File::query_chunk_store(string &local_file, string &fid, string &chunk_addr, int chunk_port)
{
	TransSocket trans_socket(chunk_addr.c_str(), chunk_port);
	if(!trans_socket.open(1000))
	{
		SLOG_ERROR("connect sfs failed.");
		return false;
	}

	int fd = open(local_file.c_str(), O_RDONLY);
	if(fd == -1)
	{
		SLOG_ERROR("open file error.");
		return false;
	}
	struct stat file_stat;
	if(fstat(fd, &file_stat) == -1)
	{
		SLOG_ERROR("stat file error. errno=%d(%s)", errno, strerror(errno));
		close(fd);
		return false;
	}

	int READ_SIZE = 4096;
	string filename = local_file.substr(local_file.find_last_of('/')+1);
	uint64_t filesize = file_stat.st_size;
	uint64_t seg_offset = 0;
	int seg_index=0;
	int seg_size = 0;
	bool seg_finished = false;
	bool result = true;

	string chunk_path;
	ByteBuffer byte_buffer(2048);
	ProtocolStore *protocol_store = (ProtocolStore *)m_protocol_family.create_protocol(PROTOCOL_STORE);
	assert(protocol_store != NULL);
	while(seg_offset < filesize)
	{
		byte_buffer.clear();
		seg_size = filesize-seg_offset;
		if(seg_size > READ_SIZE)
			seg_size = READ_SIZE;
		else
			seg_finished = true;

		//设置协议字段
		protocol_store->set_fid(fid);
		protocol_store->set_file_name(filename);
		protocol_store->set_file_size(filesize);
		protocol_store->set_seg_offset(seg_offset);
		protocol_store->set_seg_index(seg_index++);
		protocol_store->set_seg_size(seg_size);
		protocol_store->set_seg_finished(seg_finished);
		seg_offset += seg_size;

		if(!send_store_protocol(&trans_socket, protocol_store, &byte_buffer, fd))
		{
			result = false;
			break;
		}

		//接收数据
		ProtocolStoreResp *protocol_store_resp = (ProtocolStoreResp *)m_protocol_family.create_protocol(PROTOCOL_STORE_RESP);
		if(!TransProtocol::recv_protocol(&trans_socket, protocol_store_resp))
		{
			result = false;
			m_protocol_family.destroy_protocol(protocol_store_resp);
			break;
		}

		int resp_result = protocol_store_resp->get_result();
		chunk_path = protocol_store_resp->get_chunk_path();
		m_protocol_family.destroy_protocol(protocol_store_resp);
		if(resp_result == 0) //存储失败
		{
			result = false;
			break;
		}
	}

	if(result == true)
		SLOG_INFO("Store succ. ChunkPath=%s.", chunk_path.c_str());

	m_protocol_family.destroy_protocol(protocol_store);
	close(fd);
	return result;
}
