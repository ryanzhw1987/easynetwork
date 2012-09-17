/*
 * DownloadProtocol.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadProtocol.h"
#include <string.h>

////////////////  请求文件大小  ///////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
int RequestSize::encode_body(IOBuffer *io_buffer)
{
	int size = m_file_name.size();
	char *buffer = io_buffer->write_open(size);
	if(buffer == NULL)
		return -1;
	memcpy((void*)buffer, (void*)m_file_name.c_str(), size);
	io_buffer->write_close(size);
	return size;
}

//解码包体.成功返回0,否则返回-1;
int RequestSize::decode_body(const char* buf, int buf_size)
{
	m_file_name.assign(buf, buf_size);
	return 0;
}

////////////////  回复文件大小  ////////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
int RespondSize::encode_body(IOBuffer *io_buffer)
{
	int size = sizeof(m_file_size)+m_file_name.size();
	char *buffer = io_buffer->write_open(size);
	if(buffer == NULL)
		return -1;
	memcpy((void*)buffer, (void*)&m_file_size, sizeof(m_file_size));
	buffer += sizeof(m_file_size);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size());
	io_buffer->write_close(size);
	return size;
}

//解码包体.成功返回0,否则返回-1;
int RespondSize::decode_body(const char* buf, int buf_size)
{
	int size = sizeof(m_file_size);
	if(buf_size < size)
		return -1;
	memcpy((void*)&m_file_size, buf, size);
	buf += size;
	m_file_name.assign(buf, buf_size-size);
	return 0;
}

////////////////  请求文件数据  ///////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
int RequestData::encode_body(IOBuffer *io_buffer)
{
	int size = sizeof(m_start_pos)+sizeof(m_size)+m_file_name.size();
	char *buffer = io_buffer->write_open(size);
	if(buffer == NULL)
		return -1;
	memcpy((void*)buffer, (void*)&m_start_pos, sizeof(m_start_pos));
	buffer += sizeof(m_start_pos);
	memcpy((void*)buffer, (void*)&m_size, sizeof(m_size));
	buffer += sizeof(m_size);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size());
	io_buffer->write_close(size);
	return size;
}

//解码包体.成功返回0,否则返回-1;
int RequestData::decode_body(const char* buf, int buf_size)
{
	int size = sizeof(m_start_pos)+sizeof(m_size);
	if(buf_size < size)
		return -1;
	memcpy((void*)&m_start_pos, buf, sizeof(m_start_pos));
	buf += sizeof(m_start_pos);
	memcpy((void*)&m_size, buf, sizeof(m_size));
	buf += sizeof(m_size);
	m_file_name.assign(buf, buf_size-size);
	return 0;
}

//////////////////////  回复数据  /////////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
int RespondData::encode_body(IOBuffer *io_buffer)
{
	int name_len = m_file_name.size();
	int size  = sizeof(m_start_pos)+sizeof(m_size)+sizeof(name_len)+m_file_name.size()+m_data.size();

	char *buffer = io_buffer->write_open(size);
	if(buffer == NULL)
		return -1;
	memcpy((void*)buffer, (void*)&m_start_pos, sizeof(m_start_pos)); //start pos
	buffer += sizeof(m_start_pos);
	memcpy((void*)buffer, (void*)&m_size, sizeof(m_size));  //request size
	buffer += sizeof(m_size);
	memcpy((void*)buffer, (void*)&name_len, sizeof(name_len));	//length of name
	buffer += sizeof(name_len);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size()); //filename
	buffer += m_file_name.size();
	memcpy((void*)buffer, (void*)m_data.c_str(), m_data.size()); //data
	io_buffer->write_close(size);
	return size;
}

//解码包体.成功返回0,否则返回-1;
int RespondData::decode_body(const char* buf, int buf_size)
{
	int name_len = 0;
	int size  = sizeof(m_start_pos)+sizeof(m_size)+sizeof(name_len);
	if(buf_size < size)
		return -1;
	memcpy((void*)&m_start_pos, buf, sizeof(m_start_pos));
	buf += sizeof(m_start_pos);
	memcpy((void*)&m_size, buf, sizeof(m_size));
	buf += sizeof(m_size);
	memcpy((void*)&name_len, buf, sizeof(name_len));
	buf += sizeof(name_len);
	m_file_name.assign(buf, name_len);
	buf += name_len;
	m_data.assign(buf, buf_size-size-name_len);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
Protocol* DownloadProtocolFamily::create_protocol(ProtocolType protocol_type, bool new_header/*=true*/)
{
	Protocol *protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_REQUEST_SIZE:
		protocol = (Protocol*)m_request_size_memcache.Alloc();
		SLOG_DEBUG("create RequestSize[%x] from m_request_size_memcache", protocol);
		break;
	case PROTOCOL_RESPOND_SIZE:
		protocol = (Protocol*)m_respond_size_memcache.Alloc();
		SLOG_DEBUG("create RespondSize[%x] from m_respond_size_memcache", protocol);
		break;
	case PROTOCOL_REQUEST_DATA:
		protocol = (Protocol*)m_request_data_memcache.Alloc();
		SLOG_DEBUG("create RequestData[%x] from m_request_data_memcache", protocol);
		break;
	case PROTOCOL_RESPOND_DATA:
		protocol = (Protocol*)m_respond_data_memcache.Alloc();
		SLOG_DEBUG("create RespondData[%x] from m_respond_data_memcache", protocol);
		break;
	default:
		break;
	}

	if(protocol!=NULL && new_header==true)
	{
		ProtocolHeader *protocol_header = create_header(protocol_type);
		if(protocol_header == NULL)
		{
			destroy_protocol(protocol);
			return NULL;
		}
		protocol->set_header(protocol_header);
	}
	return protocol;
}

int DownloadProtocolFamily::destroy_protocol(Protocol* protocol)
{
	if(protocol == NULL)
		return 0;
	destroy_header(protocol->set_header(NULL));
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_REQUEST_SIZE:
	{
		SLOG_DEBUG("free DownloadRequest[%x] to m_request_size_memcache", protocol);
		RequestSize *temp_protocol = (RequestSize*)protocol;
		m_request_size_memcache.Free(temp_protocol);
		break;
	}
	case PROTOCOL_RESPOND_SIZE:
	{
		SLOG_DEBUG("free RespondSize[%x] to m_respond_size_memcache", protocol);
		RespondSize *temp_protocol = (RespondSize*)protocol;
		m_respond_size_memcache.Free(temp_protocol);
		break;
	}
	case PROTOCOL_REQUEST_DATA:
	{
		SLOG_DEBUG("free RequestData[%x] to m_request_data_memcache", protocol);
		RequestData *temp_protocol = (RequestData*)protocol;
		m_request_data_memcache.Free(temp_protocol);
		break;
	}
	case PROTOCOL_RESPOND_DATA:
	{
		SLOG_DEBUG("free RespondData[%x] to m_respond_data_memcache", protocol);
		RespondData *temp_protocol = (RespondData*)protocol;
		m_respond_data_memcache.Free(temp_protocol);
		break;
	}
	default:
		break;
	}

	return 0;
}
