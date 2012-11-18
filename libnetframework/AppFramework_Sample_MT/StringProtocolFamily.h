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


class StringProtocol:public Protocol
{
//实现父类接口
public:
	//协议的描述信息
	const char* details(){return "raw_string_protocol";}
	//编码协议体数据到io_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	//解码协议体数据io_buffer.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);
public:
	void set_string(string &str){m_str = str;}
	string& get_string(){return m_str;}
private:
	string m_str;
};

inline
Protocol* StringProtocolFamily::create_protocol_by_header(ProtocolHeader *header)
{
	int protocol_type = ((DefaultProtocolHeader *)header)->get_protocol_type();
	Protocol *protocol = NULL;
	switch(protocol_type)
	{
	case PROTOCOL_STRING:
		protocol = new StringProtocol;
		break;
	}

	return protocol;
}

inline
void StringProtocolFamily::destroy_protocol(Protocol *protocol)
{
	delete protocol;
}

//编码协议体数据到io_buffer,成功返回true,失败返回false.
inline
bool StringProtocol::encode_body(ByteBuffer *byte_buffer)
{
	int len = 0;
	////m_str
	ENCODE_STRING(m_str);

	return true;
}
//解码协议体数据io_buffer.成功返回true,失败返回false.
inline
bool StringProtocol::decode_body(const char *buf, int size)
{
	int len = 0;
	////m_str
	DECODE_STRING(m_str);

	return true;
}

#endif //_STRING_PROTOCOL_FAMILY_H_
