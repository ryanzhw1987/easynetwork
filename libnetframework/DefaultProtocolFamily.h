/*
 * DefaultProtocolFamily.h
 *
 *  Created on: 2012-10-18
 *      Author: LiuYongJin
 */

#ifndef _LIB_DEFAULT_PROTOCOL_FAMILY_H_
#define _LIB_DEFAULT_PROTOCOL_FAMILY_H_

#include "ProtocolFamily.h"
#include "slog.h"
#include <assert.h>
#include <arpa/inet.h>

/////////////////////////////////////////////////////////////
/////////////////                           /////////////////
/////////////////   DefaultProtocolHeader   /////////////////
/////////////////                           /////////////////
/////////////////////////////////////////////////////////////
class DefaultProtocolHeader: public ProtocolHeader
{
public:
	//协议头长度
	int get_header_length();

	//编码协议头.成功返回true,失败返回false
	//buf:存放协议头编码后数据的缓冲区
	//body_length:协议体长度
	virtual bool encode(char *buf, int body_length);

	//解码协议头.成功返回true,失败返回false
	//io_buffer:待解码的协议头数据
	//body_length:解码后返回的协议体长度
	virtual bool decode(const char *buf, int &body_length);
public:
	DefaultProtocolHeader(int magic_num=-1, int sequence=-1, int protocol_type=-1)
		:m_magic_num(magic_num)
		,m_sequence(sequence)
		,m_protocol_type(protocol_type)
	{}

	int get_magic_num(){return m_magic_num;}
	void set_magic_num(int magic_num){m_magic_num = magic_num;}

	int get_sequence(){return m_sequence;}
	void set_sequence(int sequence){m_sequence = sequence;}

	int get_protocol_type(){return m_protocol_type;}
	void set_protocol_type(int protocol_type){m_protocol_type = protocol_type;}
private:
	int m_magic_num;
	int m_sequence;
	int m_protocol_type;
};
//////////////////////////
////  implementation  ////
//////////////////////////
inline
int DefaultProtocolHeader::get_header_length()
{
	return 4*sizeof(int);	//magic_num,m_sequence,m_protocol_type和body_length的长度
}

inline
bool DefaultProtocolHeader::encode(char *buf, int body_length)
{
	if(buf == NULL)
	{
		SLOG_ERROR("header encode: no memory");
		return false;
	}
	int header_length = get_header_length();

	//1. magic number
	*(int*)buf = htonl(m_magic_num);
	buf += sizeof(int);

	//2. sequence
	*(int*)buf = htonl(m_sequence);
	buf += sizeof(int);

	//3. protocol type
	*(int*)buf = htonl(m_protocol_type);
	buf += sizeof(int);

	//4. body length
	*(int*)buf = htonl(body_length);

	SLOG_DEBUG("header encode succ: magic_num:%d, sequence=%d, protocol_type:%d, body_length:%d", m_magic_num, m_sequence, m_protocol_type, body_length);
	return true;
}

inline
bool DefaultProtocolHeader::decode(const char *buf, int &body_length)
{
	if(buf==NULL)
	{
		SLOG_ERROR("header decode: raw data no enough");
		return false;
	}
	//1. magic number
	int ret = *(int*)buf;
	ret = ntohl(ret);
	if(ret != m_magic_num)
	{
		SLOG_ERROR("header decode: invalid magic_num:%d, expected magic_num:%d", ret, m_magic_num);
		return false;
	}
	buf += sizeof(int);

	//2. sequence
	ret = *(int*)(buf);
	m_sequence = ntohl(ret);
	buf += sizeof(int);

	//3. protocol type
	ret = *(int*)(buf);
	m_protocol_type = ntohl(ret);
	buf += sizeof(int);

	//4. body length
	ret = *(const int*)(buf);
	body_length = ntohl(ret);

	SLOG_DEBUG("header decode succ. magic_num:%d, sequence=%d, protocol_type:%d, body_length:%d", m_magic_num, m_sequence, m_protocol_type, body_length);
	return true;
}

/////////////////////////////////////////////////////////////
/////////////////                           /////////////////
/////////////////   DefaultProtocolFamily   /////////////////
/////////////////                           /////////////////
/////////////////////////////////////////////////////////////
#define DEFAULT_MAGIC_NUM	0X1F0696D9 //int:520525529
class DefaultProtocolFamily:public ProtocolFamily
{
public: //基类纯虚方法
	//创建协议头(创建magic_num为m_magic_num的协议头)
	virtual ProtocolHeader* create_protocol_header(){return new DefaultProtocolHeader(m_magic_num, -1, -1);}
	//销毁协议头
	virtual void destroy_protocol_header(ProtocolHeader *header){delete header;}

	//由子类实现具体的创建/销毁protocol的方法
	//virtual Protocol* create_protocol_by_header(ProtocolHeader *header)=0;
	//virtual void destory_protocol(Protocol *protocol)=0;
public:
	DefaultProtocolFamily(int magic_num=DEFAULT_MAGIC_NUM):m_magic_num(magic_num){}
	virtual ~DefaultProtocolFamily(){}
	int get_magic_num(){return m_magic_num;}

	//创建具体的协议
	//protocol_type: 创建协议的类型;DefaultProtocolHeader的protocol_type字段值
	//sequence: DefaultProtocolHeader的sequence字段值
	Protocol* create_protocol(int protocol_type, int sequence=-1);
private:
	int m_magic_num;
};
//////////////////////////
////  implementation  ////
//////////////////////////
inline
Protocol* DefaultProtocolFamily::create_protocol(int protocol_type, int sequence/*=-1*/)
{
	DefaultProtocolHeader *header = (DefaultProtocolHeader*)create_protocol_header();
	header->set_sequence(sequence);
	header->set_protocol_type(protocol_type);
	Protocol *protocol = create_protocol_by_header(header);
	if(protocol != NULL)
	{
		protocol->attach_protocol_header(header);
		protocol->set_protocol_family(this);
	}
	else
	{
		destroy_protocol_header(header);
	}

	return protocol;
}

#endif //_LIB_DEFAULT_PROTOCOL_FAMILY_H_
