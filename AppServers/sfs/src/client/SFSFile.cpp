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

int File::get_file_info(FileInfo &fileinfo, string &fid, bool query_chunkpath/*=false*/)
{
	//协议数据
	ProtocolFileInfoReq* protocol_fileinfo_req = (ProtocolFileInfoReq*)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO_REQ);
	assert(protocol_fileinfo_req != NULL);
	protocol_fileinfo_req->set_fid(fid);
	protocol_fileinfo_req->set_query_chunkpath(query_chunkpath);
	//回复
	ProtocolFileInfo *protocol_fileinfo = (ProtocolFileInfo *)query_master(protocol_fileinfo_req);
	m_protocol_family.destroy_protocol(protocol_fileinfo_req);
	if(protocol_fileinfo == NULL)
		return 0;

	int result = protocol_fileinfo->get_result();
	if(result != 0)
		fileinfo = protocol_fileinfo->get_fileinfo();

	m_protocol_family.destroy_protocol(protocol_fileinfo);

	return result;
}

bool File::save_file(FileInfo &fileinfo, string &local_file)
{
	string fid="AAACCCDDD";

	int result = get_file_info(fileinfo, fid, true);
	switch(result)
	{
	case 0:
		{
			SLOG_ERROR("get file info failed. fid=%s.", fid.c_str());
			return false;
		}
	case 1:
		{
			SLOG_INFO("file already exist. fid:%s, name:%s, size:%lld.", fileinfo.fid.c_str(), fileinfo.name.c_str(), fileinfo.size);
			vector<ChunkPath>::iterator it;
			for(it=fileinfo.path_list.begin(); it!=fileinfo.path_list.end(); ++it)
				SLOG_INFO("chunk path: %s_%d_%lld addr:%s, port=%d\n",it->id.c_str(), it->index, it->offset, it->addr.c_str(), it->port);
			break;
		}
	case 2:
		{
			vector<ChunkPath>::iterator it;
			for(it=fileinfo.path_list.begin(); it!=fileinfo.path_list.end(); ++it)
			{
				SLOG_INFO("request chunk to store file. fid=%s. chunk add:%s, chunk port:%d", fid.c_str(), it->addr.c_str(), it->port);
				if(send_file_to_chunk(local_file, fid, it->addr, it->port))
					break;
			}
			break;
		}
	default:
		{
			SLOG_WARN("unknow result value. result=%d.", result);
			return false;
		}
	}

	return true;
}

bool File::get_file(string &fid, string &local_file)
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
	ProtocolFileInfo *protocol_fileinfo = (ProtocolFileInfo *)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO);
	assert(protocol_fileinfo != NULL);
	if(!TransProtocol::recv_protocol(&trans_socket, protocol_fileinfo))
	{
		m_protocol_family.destroy_protocol(protocol_fileinfo);
		protocol_fileinfo = NULL;
	}
	return protocol_fileinfo;
}

bool File::send_file_protocol_to_chunk(TransSocket* trans_socket, ProtocolFile *protocol_file, ByteBuffer *byte_buffer, int fd)
{
	byte_buffer->clear();

	//1 预留头部空间
	int header_length;
	ProtocolHeader *header = protocol_file->get_protocol_header();
	header_length = header->get_header_length();
	byte_buffer->reserve(header_length);
	//2 编码协议体
	if(!protocol_file->encode_body(byte_buffer))
	{
		SLOG_ERROR("encode body error");
		return false;
	}
	//3 添加数据
	FileSeg& file_seg = protocol_file->get_file_seg();
	char *data_buffer = byte_buffer->get_append_buffer(file_seg.size);
	if(read(fd, data_buffer, file_seg.size) != file_seg.size)
	{
		SLOG_ERROR("read file error. errno=%d(%s).", errno, strerror(errno));
		return false;
	}
	byte_buffer->set_append_size(file_seg.size);
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

bool File::send_file_to_chunk(string &local_file, string &fid, string &chunk_addr, int chunk_port)
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
	int seg_size = 0;
	bool seg_finished = false;
	bool result = true;

	string chunk_path;
	ByteBuffer byte_buffer(2048);
	ProtocolFile *protocol_file = (ProtocolFile *)m_protocol_family.create_protocol(PROTOCOL_FILE);
	assert(protocol_file != NULL);
	while(seg_offset < filesize)
	{
		byte_buffer.clear();
		seg_size = filesize-seg_offset;
		if(seg_size > READ_SIZE)
			seg_size = READ_SIZE;
		else
			seg_finished = true;

		//设置协议字段
		protocol_file->set_result(0);
		FileSeg &file_seg = protocol_file->get_file_seg();
		file_seg.fid = fid;
		file_seg.filesize = filesize;
		file_seg.size = seg_size;
		file_seg.end = seg_finished;
		seg_offset += seg_size;

		if(!send_file_protocol_to_chunk(&trans_socket, protocol_file, &byte_buffer, fd))
		{
			result = false;
			break;
		}

		//接收数据
		ProtocolStoreResult *protocol_store_resp = (ProtocolStoreResult *)m_protocol_family.create_protocol(PROTOCOL_STORE_RESULT);
		if(!TransProtocol::recv_protocol(&trans_socket, protocol_store_resp))
		{
			result = false;
			m_protocol_family.destroy_protocol(protocol_store_resp);
			break;
		}

		int resp_result = protocol_store_resp->get_result();
		chunk_path = protocol_store_resp->get_chunk_path();
		m_protocol_family.destroy_protocol(protocol_store_resp);
		if(resp_result != 0) //存储失败
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
