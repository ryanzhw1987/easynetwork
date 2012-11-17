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
	Protocol* create_protocol(ProtocolHeader *header);
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

/////////////////////////////////////////////////////////////////////
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
