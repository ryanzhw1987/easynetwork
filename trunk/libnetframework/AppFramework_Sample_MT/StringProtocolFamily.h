/*
 * StringProtocolFamily.h
 *
 *  Created on: 2012-10-21
 *      Author: LiuYongJin
 */
#ifndef _STRING_PROTOCOL_FAMILY_H_
#define _STRING_PROTOCOL_FAMILY_H_
#include <string>
using std::string;

#include "DefaultProtocolFamily.h"
#define PROTOCOL_STRING 1

class StringProtocolFamily: public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
};

//直接使用attach_raw_data和detach_raw_data
class StringProtocol:public Protocol
{
public:
	//协议的描述信息
	const char* details(){return "string_protocol";}
	//编码协议体数据到io_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer)
	{
		int body_length = m_string.size()+1;
		char *body_buffer = byte_buffer->get_append_buffer(body_length);
		if(body_buffer == NULL)
			return false;
		memcpy(body_buffer, m_string.c_str(), body_length);
		byte_buffer->set_append_size(body_length);
		return true;
	}
	//解码协议体数据io_buffer.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size)
	{
		m_string.assign(buf, size);
		return true;
	}
public:
	void set_string(string &str){m_string = str;}
	string& get_string(){return m_string;}
private:
	string m_string;
};

inline
Protocol* StringProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	DefaultProtocolHeader* default_header = (DefaultProtocolHeader*)header;
	if(default_header->get_protocol_type() == PROTOCOL_STRING)
		return new StringProtocol;
	return NULL;
}

inline
void StringProtocolFamily::destroy_protocol(Protocol *protocol)
{
	delete protocol;
}

#endif //_STRING_PROTOCOL_FAMILY_H_
