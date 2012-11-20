/*
 * TemplateProtocolFamily.h
 *
 *  Created on: #CreateDate#
 *      Author: #Author#
 */

#ifndef _TemplateProtocolFamily_H_
#define _TemplateProtocolFamily_H_

#include "DefaultProtocolFamily.h"

#define TEST 0
#if TEST

#include <string>
using std::string;
#define PROTOCOL_TEST          0    //Test Protocol

#endif

////Define Your Protocol Type From Here
//////////////////////////////////




//////////////////////////////  ProtooclFamily  //////////////////////////////
class TemplateProtocolFamily: public DefaultProtocolFamily
{
public:
	Protocol* create_protocol_by_header(ProtocolHeader *header);
	void destroy_protocol(Protocol *protocol);
};

//////////////////////////////  Protocol  //////////////////////////////
#if TEST
class TemplateProtocol:public Protocol
{
////实现父类接口
public:
	////协议的描述信息
	const char* details(){return "TemplateProtocol";}
	////编码协议体数据到io_buffer,成功返回true,失败返回false.
	bool encode_body(ByteBuffer *byte_buffer);
	////解码协议体数据io_buffer.成功返回true,失败返回false.
	bool decode_body(const char *buf, int size);

public:
	void set_value(int value){m_value = value;}
	int get_value(){return m_value;}

	void set_string(string &str){m_str = str;}
	string& get_string(){return m_str;}
private:
	int m_value;
	string m_str;
};
#endif

////Add Your Protocol From Here
///////////////////////////////


#endif //_TemplateProtocolFamily_H_

