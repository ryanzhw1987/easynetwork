/*
 * DownloadProtocol.cpp
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#include "DownloadProtocol.h"
#include <string.h>

/////////////////////////////////////////////////////////////
//编码字符
#define ENCODE_CHAR(c) do{ \
	if(!byte_buffer->append(c)) \
		return false; \
}while(0)
//解码字符
#define DECODE_CHAR(c) do{ \
	if(size < sizeof(c)) return false; \
	c = buf[0]; ++buf; --size; \
}while(0)

//编码整数
#define ENCODE_INT(i) do{ \
	if(!byte_buffer->append((const char*)&i, sizeof(i))) \
		return false; \
}while(0)
//解码整数
#define DECODE_INT(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

//编码64位整数
#define ENCODE_INT64(i) ENCODE_INT(i)
//解码整数
#define DECODE_INT64(i) do{ \
	if(size < sizeof(i)) return false; \
	i = *(int64_t*)buf; buf+=sizeof(i); size-=sizeof(i); \
}while(0)

//编码字符串
#define ENCODE_STRING(str) do{\
	len = str.size(); \
	if(!byte_buffer->append((const char*)&len, sizeof(len))) \
		return false; \
	if(len > 0 && !byte_buffer->append(str.c_str())) \
		return false; \
}while(0)
//解码字符串
#define DECODE_STRING(str) do{\
	DECODE_INT(len); \
	if(len<0 || size<len) return false; \
	if(len > 0) \
		str.assign(buf, len); buf+=len; size-=len; \
}while(0)

////////////////  请求文件大小  ///////////////////
//编码协议体数据到byte_buffer,成功返回true,失败返回false.
bool RequestSize::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//file name
	ENCODE_STRING(m_file_name);

	return true;
}

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
bool RequestSize::decode_body(const char* buf, int size)
{
	int len = 0;
	//file name
	DECODE_STRING(m_file_name);

	return true;
}

////////////////  回复文件大小  ////////////////////
//编码协议体数据到byte_buffer,成功返回true,失败返回false.
bool RespondSize::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//file size
	ENCODE_INT64(m_file_size);
	//file name
	ENCODE_STRING(m_file_name);

	return true;
}

//解码大小为size的协议体数据buf.成功返回true,失败返回false.
bool RespondSize::decode_body(const char* buf, int size)
{
	int len = 0;
	//file size
	DECODE_INT64(m_file_size);
	//file name
	DECODE_STRING(m_file_name);

	return true;
}

////////////////  请求文件数据  ///////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
bool RequestData::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//file name
	ENCODE_STRING(m_file_name);
	//start pos
	ENCODE_INT64(m_start_pos);
	//size
	ENCODE_INT(m_size);

	return true;
}

//解码包体.成功返回0,否则返回-1;
bool RequestData::decode_body(const char* buf, int size)
{
	int len = 0;
	//file name
	DECODE_STRING(m_file_name);
	//start pos
	DECODE_INT64(m_start_pos);
	//size
	DECODE_INT(m_size);

	return true;
}

//////////////////////  回复数据  /////////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
bool RespondData::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	//file name
	ENCODE_STRING(m_file_name);
	//start pos
	ENCODE_INT64(m_start_pos);
	//size
	ENCODE_INT(m_size);

	return true;
}

//解码包体.成功返回0,否则返回-1;
bool RespondData::decode_body(const char* buf, int size)
{
	int len = 0;
	//file name
	DECODE_STRING(m_file_name);
	//start pos
	DECODE_INT64(m_start_pos);
	//size
	DECODE_INT(m_size);
	//data
	m_data = buf;

	return true;
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
