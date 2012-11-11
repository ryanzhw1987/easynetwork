/*
 * SFSClient.cpp
 *
 *  Created on: 2012-11-8
 *      Author: LiuYongJin
 */

#include "SFSFile.h"
#include "Socket.h"
#include "slog.h"
#include "SFSProtocolFamily.h"
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
	//协议数据
	ProtocolFileInfo* protocol_fileinfo = (ProtocolFileInfo*)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO);
	protocol_fileinfo->set_fid(fid);
	protocol_fileinfo->set_query_chunkinfo(query_chunkinfo);

	ProtocolFileInfoResp *protocol_fileinfo_resp =NULL;
	protocol_fileinfo_resp = (ProtocolFileInfoResp *)query_master(protocol_fileinfo);
	fileinfo->result = 0;
	if(protocol_fileinfo_resp != NULL)
	{
		fileinfo->result = protocol_fileinfo_resp->get_result();
		fileinfo->fid = protocol_fileinfo_resp->get_fid();
		fileinfo->size = protocol_fileinfo_resp->get_fielsize();
		fileinfo->chunkinfo = protocol_fileinfo_resp->get_chunkinfo();
	}

	m_protocol_family.destroy_protocol(protocol_fileinfo);
	m_protocol_family.destroy_protocol(protocol_fileinfo_resp);

	return fileinfo->result == 0?false:true;
}

bool File::store(string &local_file)
{
	string fid="AAACCCDDD";
	FileInfo fileinfo;
	if(file_info(&fileinfo, fid, true))
	{
		printf("result:%d\nFID:%s\nFileSize:%lld\n", fileinfo.result, fileinfo.fid.c_str(), fileinfo.size);
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			printf("ChunkInfo:\n\tChunkPath:%s\n\tChunkAdd:%s\n\tChunkPort:%d\n",it->path.c_str(), it->chunk_addr.c_str(), it->port);
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
		printf("file already exist. FID:%s\nFileSize:%lld\n", fileinfo.fid.c_str(), fileinfo.size);
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			printf("ChunkInfo:\n\tChunkPath:%s\n\tChunkAdd:%s\n\tChunkPort:%d\n",it->path.c_str(), it->chunk_addr.c_str(), it->port);
		break;
	}
	case 2:
	{
		printf("request chunk to store file. fid=%s.", fid.c_str());
		vector<ChunkInfo>::iterator it;
		for(it=fileinfo.chunkinfo.begin(); it!=fileinfo.chunkinfo.end(); ++it)
			query_chunk_store(local_file, fid, it->chunk_addr, it->port);
		break;
	}
	default:
	{
		printf("unknow result value. result=%d", fileinfo.result);
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

bool File::query_chunk_store(string &local_file, string &fid, string &chunk_addr, int chunk_port)
{
	TransSocket trans_socket(m_master_addr.c_str(), m_master_port);
	if(!trans_socket.open(1000))
	{
		SLOG_ERROR("connect sfs failed.");
		return false;
	}

	int fp = open(local_file.c_str(), O_RDONLY);
	if(fp == -1)
	{
		SLOG_ERROR("open file error.");
		return false;
	}
	struct stat file_stat;
	if(fstat(fp, &file_stat) == -1)
	{
		SLOG_ERROR("stat file error. errno=%d(%s)", errno, strerror(errno));
		close(fp);
		return false;
	}

	int READ_SIZE = 4096;
	string filename = local_file.substr(local_file.find_last_of('/')+1);
	uint64_t filesize = file_stat.st_size;
	uint64_t seg_offset = 0;
	int seg_index=0;
	int seg_size = 0;
	bool seg_finished = false;

	while(seg_offset < filesize)
	{
		seg_size = filesize-seg_offset;
		if(seg_size > READ_SIZE)
			seg_size = READ_SIZE;
		else
			seg_finished = true;

		ByteBuffer byte_buffer(seg_size+200);
		ProtocolStore *protocol_store = (ProtocolStore *)m_protocol_family.create_protocol(PROTOCOL_STORE);
		assert(protocol_store != NULL);

		//设置协议字段
		protocol_store->set_fid(fid);
		protocol_store->set_file_name(filename);
		protocol_store->set_file_size(filesize);
		protocol_store->set_seg_offset(seg_offset);
		protocol_store->set_seg_index(seg_index++);
		protocol_store->set_seg_size(seg_size);
		protocol_store->set_seg_finished(seg_finished);


		//1 预留头部空间
		int header_length;
		ProtocolHeader *header = protocol_store->get_protocol_header();
		header_length=header->get_header_length();
		byte_buffer.reserve(header_length);
		//2 编码协议体
		protocol_store->encode_body(&byte_buffer);
		//3 添加数据
		char *data_buffer = byte_buffer.get_append_buffer(seg_size);
		if(read(fp, data_buffer, seg_size) != seg_size)
		{
			SLOG_ERROR("read file error. errno=%d(%s).", errno, strerror(errno));
			m_protocol_family.destroy_protocol(protocol_store);
			close(fp);
			return false;
		}
		seg_offset += seg_size;
		byte_buffer.set_append_size(seg_size);
		//4. 编码协议头
		int body_length = byte_buffer.size()-header_length;
		char *header_buffer = byte_buffer.get_data(0, header_length);
		header->encode(header_buffer, body_length);
		//5. 发送数据
		if(trans_socket.send_data_all(byte_buffer.get_data(), byte_buffer.size()) == TRANS_ERROR)
		{
			SLOG_ERROR("send data error");
			m_protocol_family.destroy_protocol(protocol_store);
			close(fp);
			return false;
		}
		m_protocol_family.destroy_protocol(protocol_store);

		//接收数据
		ProtocolStoreResp *protocol_store_resp = (ProtocolStoreResp *)m_protocol_family.create_protocol(PROTOCOL_STORE_RESP);
		if(TransProtocol::recv_protocol(&trans_socket, protocol_store_resp))
		{

		}
		m_protocol_family.destroy_protocol(protocol_store_resp);
	}
	close(fp);
	return true;
}
