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

using namespace SFS;

File::File(string &master_addr, int master_port, int n_replica)
	:m_master_addr(master_addr)
	,m_master_port(master_port)
	,m_n_replica(n_replica)
{}

bool File::file_info(string &fid, FileInfo &file_info)
{
	//协议数据
	ProtocolFileInfo* protocol_fileinfo = (ProtocolFileInfo*)m_protocol_family.create_protocol(PROTOCOL_FILE_INFO);
	protocol_fileinfo->set_fid(fid);
	protocol_fileinfo->set_query_chunkinfo(false);	//当没有文件信息时,不用返回chunk info

	ProtocolFileInfoResp *protocol_fileinfo_resp =NULL;
	protocol_fileinfo_resp = (ProtocolFileInfoResp *)query_master(protocol_fileinfo);
	file_info.result = 0;
	if(protocol_fileinfo_resp != NULL)
	{
		file_info.result = protocol_fileinfo_resp->get_result();
		file_info.fid = protocol_fileinfo_resp->get_fid();
		file_info.size = protocol_fileinfo_resp->get_fielsize();
		file_info.chunkinfo = protocol_fileinfo_resp->get_chunkinfo();
	}

	m_protocol_family.destroy_protocol(protocol_fileinfo);
	m_protocol_family.destroy_protocol(protocol_fileinfo_resp);

	return file_info.result == 0?false:true;
}

Protocol* File::query_master(Protocol *protocol)
{
	TransSocket trans_socket(m_master_addr.c_str(), m_master_port);
	if(!trans_socket.open(1000))
	{
		SLOG_ERROR("connect sfs failed.");
		return NULL;
	}

	//编码
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	int header_length = header->get_header_length();

	ByteBuffer *byte_buffer = new ByteBuffer;
	//1. 预留协议头空间
	byte_buffer->get_append_buffer(header_length);
	byte_buffer->set_append_size(header_length);
	//2. 编码协议体数据
	if(!protocol->encode_body(byte_buffer))
	{
		SLOG_ERROR("encode body failed.");
		delete byte_buffer;
		return false;
	}
	//3. 编码协议头
	int body_length = byte_buffer->size()-header_length;
	char *header_buffer = byte_buffer->get_data();
	if(!header->encode(header_buffer, body_length))
	{
		SLOG_ERROR("encode header failed.");
		delete byte_buffer;
		return NULL;
	}

	//发送数据
	if(trans_socket.send_data_all(byte_buffer->get_data(), byte_buffer->size()) == TRANS_ERROR)
	{
		SLOG_ERROR("send data error");
		delete byte_buffer;
		return NULL;
	}

	//接收数据
	byte_buffer->clear();

	header = (DefaultProtocolHeader *)m_protocol_family.create_protocol_header();
	header_length = header->get_header_length();
	char *buff = byte_buffer->get_append_buffer(header_length);
	if(trans_socket.recv_data_all(buff, header_length) == TRANS_ERROR)
	{
		SLOG_ERROR("receive header data error");
		delete byte_buffer;
		m_protocol_family.destroy_protocol_header(header);
		return NULL;
	}
	byte_buffer->set_append_size(header_length);

	body_length = 0;
	if(header->decode(buff, body_length) == false)
	{
		SLOG_ERROR("decode header error");
		delete byte_buffer;
		m_protocol_family.destroy_protocol_header(header);
		return NULL;
	}
	buff = byte_buffer->get_append_buffer(body_length);
	if(trans_socket.recv_data_all(buff, body_length) == TRANS_ERROR)
	{
		SLOG_ERROR("receive body data error");
		delete byte_buffer;
		m_protocol_family.destroy_protocol_header(header);
		return NULL;
	}

	Protocol *resp_protocol = m_protocol_family.create_protocol_by_header(header);
	if(resp_protocol == NULL)
	{
		SLOG_ERROR("create protocol error");
		delete byte_buffer;
		m_protocol_family.destroy_protocol_header(header);
		return NULL;
	}
	resp_protocol->set_protocol_family(&m_protocol_family);
	resp_protocol->attach_protocol_header(header);

	if(resp_protocol->decode_body(buff, body_length) == false)
	{
		SLOG_ERROR("decode protocol error");
		delete byte_buffer;
		m_protocol_family.destroy_protocol(resp_protocol);
		return NULL;
	}
	resp_protocol->attach_raw_data(byte_buffer);
	return resp_protocol;
}

