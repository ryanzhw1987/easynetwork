#ifndef _LIB_PROTOCOL_H_20120615_LIUYONGJIN
#define _LIB_PROTOCOL_H_20120615_LIUYONGJIN

#include <stdio.h>
#include "IOBuffer.h"

//协议头接口
class ProtocolHeader
{
public:    
	virtual ~ProtocolHeader(){}

	//返回协议头大小.
	virtual int get_header_size()=0;
	//返回包体长度.
	virtual int get_body_size()=0;
	//包头编码.成功返回0. 否则返回-1.
	virtual int encode(char *buf, int buf_size)=0;
	//包头解码.成功返回0,失败返回-1.
	virtual int decode(const char *buf, int buf_size)=0;
};
//sample
/*
	//1. 解码
	ProtocolHeader *header = create_header();
	int header_size = header->get_header_size();
	read(fd, buf,header_size);

	int ret = header->decode(buf, header_size);
	if(ret == 0)
	{
		int body_size = header->get_body_size();
		read(fd, buf, body_size);
		//decode body
		//...
	}
	delete header;

	//2. 编码
	ProtocolHeader *header = create_header();
	int header_size = header->get_header_size();
	header->encode(buf, header_size);

	delete header;
*/


class Protocol
{
public:
	virtual ~Protocol(){}
	///////////////////////////////////////// 编码 //////////////////////////
	//协议编码,成功返回0; 否则返回-1;
	virtual int encode(IOBuffer *io_buffer) = 0;
	///////////////////////////////////////// 解码 //////////////////////////
	//获取协议头大小
	virtual int get_header_size()=0;
	//解码协议头.否则返回-1
	virtual int decode_header(const char* buf, int buf_size)=0;
	//获取协议体长度.
	virtual int get_body_size()=0;
	//解码包体.成功返回0,否则返回-1;
	virtual int decode_body(const char* buf, int buf_size)=0;
};

//用于创建具体protocol的工厂类
class ProtocolFamily
{
public:
	virtual Protocol* create_protocol()=0;
};

//sample
/*
	//1. 解码
	Protocol *protocol = protocol_family->create_protocol();

	//decode header
	int header_size = protocol->get_header_size();
	read(fd, buf, header_size);
	if(protocol->decode_header(buf, header_size) != 0)
	{
		delete protocol;
	}
	
	//decode body
	int body_size = protocol->get_body_size();
	read(fd, buf, body_size);
	if(protocol->decode_body(buf, body_size) != 0)
	{
		delete protocol;
	}
	//use protocol
	//...


	//2. 编码
	Protocol *protocol = protocol_family->create_protocol();
	//Protocol *protocol = create_protocol();
	
	//use protocol
	//....

	EncodeBuffer encode_buffer;
	if(protocol->encode(&encode_buffer) == 0)
	{
		//use encode_buffer
		//...
	}
	delete protocol;
*/

#endif

