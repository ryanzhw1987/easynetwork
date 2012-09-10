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
	virtual void set_body_size(int body_size)=0;
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
	//delete header;

	//2. 编码
	ProtocolHeader *header = create_header();
	int header_size = header->get_header_size();
	int ret = header->encode(buf, header_size);
	//delete header;
*/

//协议接口
class Protocol
{
public:
	Protocol(ProtocolHeader *header=NULL):m_header(header){}
	virtual ~Protocol(){}

	ProtocolHeader* get_header()
	{
		return m_header;
	}
	ProtocolHeader* set_header(ProtocolHeader *header)
	{
		ProtocolHeader *old_header = m_header;
		m_header = header;
		return old_header;
	}
	//获取协议头大小.
	int get_header_size()
	{
		return m_header==NULL?-1:m_header->get_header_size();
	};
	//获取解码后协议体大小.
	int get_body_size()
	{
		return m_header==NULL?-1:m_header->get_body_size();
	}

	//解码协议头.成功返回0,失败返回-1
	int decode_header(const char* buf, int buf_size)
	{
		return m_header==NULL?-1:m_header->decode(buf, buf_size);
	}

	//编码包括协议头和协议体到io_buffer:
	//成功返回编码后的长度(大于0);
	//失败返回-1,io_buffer没有变化.(协议编码后长度为0时,当作失败处理,也返回-1);
	int encode(IOBuffer *io_buffer);

	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	virtual int encode_body(IOBuffer *io_buffer)=0;
	//解码包体.成功返回0,否则返回-1;
	virtual int decode_body(const char* buf, int buf_size)=0;
private:
	ProtocolHeader *m_header;
};
/* 1. 解码
	//sample 1. 头部单独解码
	Protocol *protocol = create_protocol();
	ProtocolHeader *header = create_header();

	int header_size = header->get_header_size();
	read(fd, buf, header_size);
	int result = header->decode(buf, header_size);
	if(result == 0)
	{
		int body_size = header->get_body_size();
		read(fd, buf, body_size);
		result = protocol->decode_body(buf, body_size);
		if(result == 0)
		{
			protocol->set_header(header);
			//to use protocol...
		}
	}

	//sample 2. 使用protocol解码头部
	Protocol *protocol = create_protocol();
	ProtocolHeader *header = create_header();
	protocol->set_header(header);

	int header_size = protocol->get_header_size();
	read(fd, buf, header_size);
	int result = protocol->decode_header(buf, header_size);
	if(result == 0)
	{
		int body_size = protocol->get_body_size();
		read(fd, buf, body_size);
		result = protocol->decode_body(buf, body_size);
		if(result == 0)
		{
			//to use protocol...
		}
	}
*/


//用于创建具体protocol的工厂类
class ProtocolFamily
{
public:
	virtual ~ProtocolFamily(){}

	virtual ProtocolHeader* create_header()=0;
	virtual int destroy_header(ProtocolHeader *header)=0;

	//根据protocol_header创建协议. 成功协议头attach到protocol中.失败返回NULL.
	virtual Protocol* create_protocol(ProtocolHeader* protocol_header)=0;
	//销毁协议. 成功返回0, 失败返回-1.
	virtual int destroy_protocol(Protocol* protocol)=0;
};

//sample_1. 解码
/*
	Protocol *protocol = protocol_family->create_protocol();

	//decode header
	int header_size = protocol_family->get_header_size();
	read(fd, buf, header_size);
	if(protocol_family->decode_header(buf, header_size) != 0)
	{
		protocol_family->destroy_protocol(protocol);
		return;
	}
	
	//decode body
	int body_size = protocol->get_body_size();
	read(fd, buf, body_size);
	if(protocol->decode_body(buf, body_size) != 0)
	{
		protocol_family->destroy_protocol(protocol);
		return;
	}
	//use protocol
*/

//sample_2. 编码
/*
	Protocol *protocol = protocol_family->create_protocol();
	//to use protocol

	IObuffer io_buffer;
	if(protocol->encode(&io_buffer) == 0)
	{
		//to use io_buffer
	}
	protocol_family->destroy_protocol(protocol);
*/

#endif

