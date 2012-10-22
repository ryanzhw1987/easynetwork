/*
 * ProtocolFamily.h
 *
 *  Created on: 2012-10-18
 *      Author: LiuYongJin
 */
 
#ifndef _LIB_PROTOCOL_FAMILY_H_
#define _LIB_PROTOCOL_FAMILY_H_

#include "IOBuffer.h"
#include <stdio.h>
#include <assert.h>

class Protocol;
class ProtocolHeader;
class ProtocolFamily;

////////////////////////////////////////////////////////////
/////////////////                          /////////////////
/////////////////      ProtocolHeader      /////////////////
/////////////////                          /////////////////
////////////////////////////////////////////////////////////
class ProtocolHeader
{
public:
	virtual ~ProtocolHeader(){}
	
	//协议头长度
	virtual int get_header_length()=0;

	//编码协议头.成功返回true,失败返回false
	//io_buffer:存放协议头编码后数据的缓冲区
	//body_length:协议头长度
	virtual bool encode(IOBuffer *io_buffer, int body_length)=0;

	//解码协议头.成功返回true,失败返回false
	//io_buffer:待解码的协议头数据
	//body_length:解码后返回的协议体长度
	virtual bool decode(IOBuffer *io_buffer, int &body_length)=0;
};


/////////////////////////////////////////////////////////////
/////////////////                           /////////////////
/////////////////      ProtocolFamily       /////////////////
/////////////////                           /////////////////
/////////////////////////////////////////////////////////////
class ProtocolFamily
{
public:
	virtual ~ProtocolFamily(){}
	//创建协议头
	virtual ProtocolHeader* create_protocol_header()=0;
	//销毁协议头
	virtual void destroy_protocol_header(ProtocolHeader *header)=0;

	//创建协议(根据协议头包含的信息创建具体的协议)
	virtual Protocol* create_protocol(ProtocolHeader *header)=0;
	//销毁协议
	virtual void destroy_protocol(Protocol *protocol)=0;
};


/////////////////////////////////////////////////////////////
/////////////////                           /////////////////
/////////////////         Protocol          /////////////////
/////////////////                           /////////////////
/////////////////////////////////////////////////////////////
class Protocol
{
public:
	Protocol(ProtocolFamily *family=NULL, ProtocolHeader *header=NULL)
		:m_protocol_family(family)
		,m_protocol_header(header)
		,m_raw_data(NULL)
	{}

	virtual ~Protocol()
	{
		if(m_protocol_header != NULL)
		{
			assert(m_protocol_family != NULL);
			m_protocol_family->destroy_protocol_header(m_protocol_header);
			m_protocol_header = NULL;
		}

		if(m_raw_data != NULL)
			delete m_raw_data;
		m_raw_data = NULL;
	}

	ProtocolFamily* get_protocol_family(){return m_protocol_family;}
	void set_protocol_family(ProtocolFamily *family){m_protocol_family = family;}

	//将header依附到protocol.如果没有调用deatch的话,protocol会在析构的时候会自动delete掉header
	//成功返回true,失败返回false.
	bool attach_protocol_header(ProtocolHeader *header)
	{
		if(m_protocol_header != NULL)
			return false;
		m_protocol_header = header;
		return true;
	}
	//将header从protocol中分离出来,由调用者自行处理header
	ProtocolHeader* deatch_protocol_header()
	{
		ProtocolHeader *temp = m_protocol_header;
		m_protocol_header = NULL;
		return temp;
	}
	//简单获取header
	ProtocolHeader* get_protocol_header(){return m_protocol_header;}

	//将raw_data依附到protocol.如果没有调用detach的话,protocol在析构的时候会自动delete掉raw_data
	//成功返回true,失败返回false
	bool attach_raw_data(IOBuffer *raw_data)
	{
		if(m_raw_data!=NULL)
			return false;
		m_raw_data=raw_data;
		return true;
	}
	//将raw_data从protocol中分离开来,由调用者自行处理raw_data
	IOBuffer* detach_raw_data()
	{
		IOBuffer* temp = m_raw_data;
		m_raw_data = NULL;
		return temp;
	}
	//简单获取raw_data
	IOBuffer* get_raw_data(){return m_raw_data;}

	//编码协议体(编码后数据保存在m_raw_data中).成功返回true,失败返回false.
	bool encode();
	//解码协议体(m_raw_data中存放待解码的数据).成功返回true,失败返回false.
	bool decode();
private:
	ProtocolHeader *m_protocol_header;
	ProtocolFamily *m_protocol_family;
	IOBuffer *m_raw_data;
public:
	//协议的描述信息
	virtual const char* details(){return "";}
	//编码协议体数据到io_buffer,成功返回true,失败返回false.
	virtual bool encode(IOBuffer *io_buffer)=0;
	//解码协议体数据io_buffer.成功返回true,失败返回false.
	virtual bool decode(IOBuffer *io_buffer)=0;
};
//////////////////////////
////  implementation  ////
//////////////////////////
inline
bool Protocol::encode()
{
	if(m_raw_data != NULL) //已经编码过(或者attach_raw_data过)
		return true;

	m_raw_data = new IOBuffer;
	bool ret = encode(m_raw_data);
	if(ret == false)
	{
		delete m_raw_data;
		m_raw_data = NULL;
	}
	return ret;
}

inline
bool Protocol::decode()
{
	if(m_raw_data == NULL)
		return false;
	return decode(m_raw_data);
}

#endif //_LIB_PROTOCOL_FAMILY_H_

