/*
 * DownloadProtocol.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadProtocol.h"
#include <string.h>

////////////////  请求文件大小  ///////////////////
//编码协议体数据到byte_buffer,成功返回true,失败返回false.
bool RequestSize::encode_body(ByteBuffer *byte_buffer)
{
	int size = m_file_name.size();
	char *buffer = byte_buffer->get_append_buffer(size);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), size);
	byte_buffer->set_append_size(size);
	return true;
}

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
bool RequestSize::decode_body(const char* buf, int buf_size)
{
	m_file_name.assign(buf, buf_size);
	return true;
}

////////////////  回复文件大小  ////////////////////
//编码协议体数据到byte_buffer,成功返回true,失败返回false.
bool RespondSize::encode_body(ByteBuffer *byte_buffer)
{
	int size = sizeof(m_file_size)+m_file_name.size();
	char *buffer = byte_buffer->get_append_buffer(size);
	memcpy((void*)buffer, (void*)&m_file_size, sizeof(m_file_size));
	buffer += sizeof(m_file_size);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size());
	byte_buffer->set_append_size(size);
	return true;
}

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
bool RespondSize::decode_body(const char* buf, int buf_size)
{
	int size = sizeof(m_file_size);
	if(buf_size < size)
		return false;
	memcpy((void*)&m_file_size, buf, size);
	buf += size;
	m_file_name.assign(buf, buf_size-size);
	return true;
}

////////////////  请求文件数据  ///////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
bool RequestData::encode_body(ByteBuffer *byte_buffer)
{
	int size = sizeof(m_start_pos)+sizeof(m_size)+m_file_name.size();
	char *buffer = byte_buffer->get_append_buffer(size);
	if(buffer == NULL)
		return false;
	memcpy((void*)buffer, (void*)&m_start_pos, sizeof(m_start_pos));
	buffer += sizeof(m_start_pos);
	memcpy((void*)buffer, (void*)&m_size, sizeof(m_size));
	buffer += sizeof(m_size);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size());
	byte_buffer->set_append_size(size);
	return true;
}

//解码包体.成功返回0,否则返回-1;
bool RequestData::decode_body(const char* buf, int buf_size)
{
	int size = sizeof(m_start_pos)+sizeof(m_size);
	if(buf_size < size)
		return false;
	memcpy((void*)&m_start_pos, buf, sizeof(m_start_pos));
	buf += sizeof(m_start_pos);
	memcpy((void*)&m_size, buf, sizeof(m_size));
	buf += sizeof(m_size);
	m_file_name.assign(buf, buf_size-size);
	return true;
}

//////////////////////  回复数据  /////////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
bool RespondData::encode_body(ByteBuffer *byte_buffer)
{
	int name_len = m_file_name.size();
	int size  = sizeof(m_start_pos)+sizeof(m_size)+sizeof(name_len)+m_file_name.size();

	char *buffer = byte_buffer->get_append_buffer(size);
	if(buffer == NULL)
		return false;

	memcpy((void*)buffer, (void*)&m_start_pos, sizeof(m_start_pos)); //start pos
	buffer += sizeof(m_start_pos);
	memcpy((void*)buffer, (void*)&m_size, sizeof(m_size));  //request size
	buffer += sizeof(m_size);
	memcpy((void*)buffer, (void*)&name_len, sizeof(name_len));	//length of name
	buffer += sizeof(name_len);
	memcpy((void*)buffer, (void*)m_file_name.c_str(), m_file_name.size()); //filename
	byte_buffer->set_append_size(size);
	return true;
}

//解码包体.成功返回0,否则返回-1;
bool RespondData::decode_body(const char* buf, int buf_size)
{
	int name_len = 0;
	int size = sizeof(m_start_pos)+sizeof(m_size)+sizeof(name_len);
	if(buf_size < size)
		return false;
	memcpy((void*)&m_start_pos, buf, sizeof(m_start_pos));
	buf += sizeof(m_start_pos);
	memcpy((void*)&m_size, buf, sizeof(m_size));
	buf += sizeof(m_size);
	memcpy((void*)&name_len, buf, sizeof(name_len));
	buf += sizeof(name_len);
	m_file_name.assign(buf, name_len);

	return true;
}

const char* RespondData::get_data()
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)get_protocol_header();
	int header_length = header->get_header_length();
	int name_len = 0;
	int size = sizeof(m_start_pos)+sizeof(m_size)+sizeof(name_len);

	ByteBuffer *raw_data = get_raw_data();
	return raw_data->get_data(m_size, header_length+size);
}

/////////////////////////////////////////////////////////////////////////////////
Protocol* DownloadProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
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

	return protocol;
}

void DownloadProtocolFamily::destroy_protocol(Protocol* protocol)
{
	if(protocol == NULL)
		return;
	DefaultProtocolHeader *header = (DefaultProtocolHeader *)protocol->get_protocol_header();
	switch(header->get_protocol_type())
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
}
