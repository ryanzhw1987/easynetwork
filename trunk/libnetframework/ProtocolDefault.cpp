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

	SLOG_TRACE("encode header succ. magic:%d, ver:%d, pro_type:%d, body_size:%d", m_magic_num, m_version, m_type, m_body_size);
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

	return 0;
}


/**********************************************  DefaultProtocol  **********************************************/
MemCache<DefaultHeader> DefaultProtocol::m_defautlheader_memcache;
//协议编码,成功返回0; 否则返回-1;
int DefaultProtocol::encode(IOBuffer *io_buffer)
{
	if(io_buffer == NULL)
		return -1;
	
	//1. 分配一个header_size大小的空间,暂时不写入数据
	int header_size = get_header_size();
	char *header_buffer = io_buffer->write_begin(header_size);
	bool result = io_buffer->write_end(header_size);
	assert(result == true);
	
	//2. 编码协议体
	int body_size = m_body->encode(io_buffer); //body size
	if(body_size == -1)  //失败, 回滚分配的header空间
	{
		result = io_buffer->write_rollback(header_size);
		assert(result == true);
		return -1;
	}

	//3. 编码协议头
	m_header->set_body_size(body_size);
	if(m_header->encode(header_buffer, header_size) != 0)	//头部编码失败,回滚协议体和协议体占用的空间
	{
		result = io_buffer->write_rollback(header_size+body_size);
		assert(result == true);
		return -1;
	}
	return 0;
}


int DefaultProtocol::decode_body(const char* buf, int buf_size)
{
	ProtocolType type = get_type();
	switch(type)
	{
	case PROTOCOL_SIMPLE:
		m_body = new SimpleCmd();
		break;
	default:
		return -1;
	}

	if(m_body->decode(buf, buf_size) != 0)
	{
		delete m_body;
		m_body = NULL;
		return -1;
	}

	return 0;
}

/********************* simple cmd *********************/
int SimpleCmd::set_data(const char *buf, int size)
{
	if(buf==NULL || size<=0)
		return -1;
	if(m_buf != NULL)
	{
		if(m_size < size)
		{
			delete[] m_buf;
			m_buf = new char[size];
		}
	}
	else
	{
		m_buf = new char[size];
	}

	memcpy(m_buf, buf, size);
	m_size = size;
	return 0;
}

int SimpleCmd::decode(const char *buf, int size)
{
	return set_data(buf, size);
}

int SimpleCmd::encode(IOBuffer *io_buffer)
{
	if(io_buffer==NULL || m_buf==NULL || m_size<=0)
		return -1;
	char *buffer = io_buffer->write_begin(m_size);
	if(buffer == NULL)
		return -1;
	memcpy(buffer, m_buf, m_size);
	io_buffer->write_end(m_size);

	return m_size;
}

