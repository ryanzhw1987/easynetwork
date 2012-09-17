#include "ProtocolDefault.h"
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include "slog.h"

/**********************************************  DefaultHeader  **********************************************/
//包头编码.成功返回0. 否则返回-1;
int DefaultHeader::encode(char *buf, int buf_size)
{
	int header_size = get_header_size();
	if(buf==NULL || buf_size<header_size || m_body_size<=0)
		return -1;

	//1. magic number
	*(int*)buf = htonl(m_magic_num);
	buf += sizeof(m_magic_num);
	//2. 版本号
	*(int*)buf = htonl(m_version);
	buf += sizeof(m_version);

	//3. body type
	*(int*)buf = htonl((int)m_type);
	buf += sizeof(m_type);

	//4. body size
	*(int*)buf = htonl(m_body_size);	

	SLOG_TRACE("encode header succ. magic:%d, ver:%d, pro_type:%d, header_size:%d, body_size:%d", m_magic_num, m_version, m_type, header_size, m_body_size);
	return 0;
}

//包头解码.成功返回0,返回包体长度和包体类型. 失败返回负数
int DefaultHeader::decode(const char *buf, int buf_size)
{
	int header_size = get_header_size();
	if(buf==NULL || buf_size<header_size)
		return -1;

	//1. magic number
	int ret = *(const int*)buf;
	ret = ntohl(ret);
	if(ret != m_magic_num)
	{
		SLOG_ERROR("magic num error. MAGIC_NUM=%d, recevie magic_num=%d",m_magic_num, ret); 
		return -1;
	}
	buf += sizeof(m_magic_num);
	
	//2. 版本号
	ret = *(const int*)buf;
	ret = ntohl(ret);
	if(ret != m_version)
	{
		SLOG_ERROR("version error. VERSION=%d, recevie version=%d",m_version, ret); 
		return -1;
	}
	buf += sizeof(m_version);

	//3. type
	ret = *(const int*)(buf);
	m_type = (ProtocolType)ntohl(ret);
	buf += sizeof(int);

	//4. body size
	ret = *(const int*)(buf);
	ret = ntohl(ret);
	if(ret <= 0)
	{
		SLOG_ERROR("body size invalid. body size=%d", ret); 
		return -1;
	}
	m_body_size = ret;

	SLOG_TRACE("decode header succ. magic_num:%d, ver:%d, type:%d, header_size:%d, body_size:%d", m_magic_num, m_version, m_type, header_size, m_body_size);
	return 0;
}

/////////////////////////////  DefaultProtocolFamily  //////////////////////////
//父类虚函数
ProtocolHeader* DefaultProtocolFamily::create_header()
{
	DefaultHeader *default_header = m_defautlheader_memcache.Alloc();
	if(default_header != NULL)
		default_header->init(m_magic_num, m_version);
	SLOG_DEBUG("create DefaultHeader[%x] from defautlheader_memcache", default_header);
	return (ProtocolHeader*)default_header;
}

//父类虚函数
int DefaultProtocolFamily::destroy_header(ProtocolHeader *header)
{
	if(header == NULL)
		return -1;
	DefaultHeader* default_header = (DefaultHeader*)header;
	m_defautlheader_memcache.Free(default_header);
	SLOG_DEBUG("free DefaultHeader[%x] to defautlheader_memcache", header);
	return 0;
}

//父类虚函数
Protocol* DefaultProtocolFamily::create_protocol(ProtocolHeader* protocol_header)
{
	if(protocol_header == NULL)
		return NULL;
	Protocol *protocol = create_protocol(((DefaultHeader*)protocol_header)->get_type(), false);
	if(protocol != NULL)
		protocol->set_header(protocol_header);
	return protocol;
}

//父类虚函数
int DefaultProtocolFamily::destroy_protocol(Protocol* protocol)
{
	if(protocol == NULL)
		return 0;
	destroy_header(protocol->set_header(NULL));
	switch(((DefaultProtocol*)protocol)->get_type())
	{
	case PROTOCOL_STRING:
	{
		SLOG_DEBUG("free StringProtocol[%x] to string_protocol_memcache", protocol);
		StringProtocol* string_protocol = (StringProtocol*)protocol;
		m_string_protocol_memcache.Free(string_protocol);
		break;
	}
	default:
		break;
	}
	return 0;
}

//根据protocol_type创建协议头
ProtocolHeader* DefaultProtocolFamily::create_header(ProtocolType protocol_type)
{
	if(protocol_type == PROTOCOL_INVALID)
		return NULL;

	ProtocolHeader* protocol_header = create_header();
	if(protocol_header != NULL)
		((DefaultHeader*)protocol_header)->set_type(protocol_type);
	return protocol_header;
}

//根据protoco_type创建对应的protocol.new_header为true时同时创建一个协议头,false时不创建协议头.
Protocol* DefaultProtocolFamily::create_protocol(ProtocolType protocol_type, bool new_header/*=true*/)
{
	Protocol* protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_STRING:
		protocol = (Protocol*)m_string_protocol_memcache.Alloc();
		SLOG_DEBUG("create StringProtocol[%x] from string_protocol_memcache", protocol);
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

///////////////////////////////////////  StringProtocol  ///////////////////////////////
//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
int StringProtocol::encode_body(IOBuffer *io_buffer)
{
	if(io_buffer==NULL || m_data.empty())
		return -1;
	int size = m_data.length();
	char *buffer = io_buffer->write_open(size);
	if(buffer == NULL)
		return -1;
	memcpy(buffer, m_data.c_str(), size);
	io_buffer->write_close(size);
	return size;
}

//解码包体.成功返回0,否则返回-1;
int StringProtocol::decode_body(const char* buf, int buf_size)
{
	m_data.assign(buf, buf_size);
	return 0;
}
