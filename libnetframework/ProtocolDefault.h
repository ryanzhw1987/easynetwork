#ifndef _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN
#define _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN

#include "Protocol.h"
#include "ProtocolType.h"
#include "MemManager.h"

#include <string>
using std::string;

//////////////////////////////////////////////////////
/////////////                           //////////////
/////////////       DefaultHeader       //////////////
/////////////                           //////////////
//////////////////////////////////////////////////////
class DefaultHeader:public ProtocolHeader
{
public: //实现父类函数
	virtual int get_header_size(){return sizeof(m_magic_num)+sizeof(m_version)+sizeof(m_body_size)+sizeof(m_type);}
	virtual int get_body_size(){return m_body_size;}
	virtual void set_body_size(int body_size){m_body_size = body_size;}

   //包头编码.成功返回0. 否则返回-1.
	virtual int encode(char *buf, int buf_size);
	//包头解码.成功返回0,失败返回-1.
	virtual int decode(const char *buf, int buf_size);

public:
	DefaultHeader(ProtocolType type=PROTOCOL_INVALID, int version=-1, int magic_num=-1)
							:m_type(type)
							,m_version(version)
							,m_magic_num(magic_num)
							,m_body_size(0)
	{}

	virtual ~DefaultHeader(){}
	void init(int magic_num, int version)
	{
		m_version = version;
		m_magic_num = magic_num;
	}

	int get_version(){return m_version;}
	int get_magic_num(){return m_magic_num;}

	ProtocolType get_type(){return m_type;}
	void set_type(ProtocolType type){m_type = type;}
private:
	int m_magic_num;
	int m_version;
	int m_body_size;
	int m_type;
};

class DefaultProtocol: public Protocol
{
public:
	DefaultProtocol(ProtocolType type):m_type(type){}
	virtual ~DefaultProtocol(){}

	//获取协议类型
	ProtocolType get_type(){return m_type;}
private:
	ProtocolType m_type;
};

//////////////////////////////////////////////////////
/////////////                           //////////////
/////////////   DefaultProtocolFamily   //////////////
/////////////                           //////////////
//////////////////////////////////////////////////////
//协议头的maigc number
#define MAGIC_NUM	0X1F0696D9 //int:520525529
#define VERSION	1
class StringProtocol;
class DefaultProtocolFamily:public ProtocolFamily
{
public: //实现父类的纯虚函数
	ProtocolHeader* create_header();
	int destroy_header(ProtocolHeader *header);
	//根据protocol_header创建协议. 成功协议头attach到protocol中.失败返回NULL.
	Protocol* create_protocol(ProtocolHeader* protocol_header);
	//销毁协议. 成功返回0, 失败返回-1.
	int destroy_protocol(Protocol* protocol);
public:
	DefaultProtocolFamily(int magic_num=MAGIC_NUM, int version=VERSION):m_magic_num(magic_num), m_version(version){}
	virtual ~DefaultProtocolFamily(){}

	//根据protocol_type创建协议头
	ProtocolHeader* create_header(ProtocolType protocol_type);
	//根据protoco_type创建对应的protocol.new_header为true时同时创建一个协议头,false时不创建协议头.
	Protocol* create_protocol(ProtocolType protocol_type, bool new_header=true);
private:
	int m_magic_num;
	int m_version;

	//memory cache for class DefaultHeader;
	MemCache<DefaultHeader> m_defautlheader_memcache;

	//memory cache: StringProtocol
	MemCache<StringProtocol> m_string_protocol_memcache;
};

////////////////////////////////    !!!*** smaple ***!!!     /////////////////////////////
/*  1. 解码(未知协议类型)
	ProtocolHeader *protocol_header = m_protocol_family->create_header();
	int header_size = protocol_header->get_header_size();
	read(fd, buf, header_size);
	if(protocol_header->decode(buf, header_size) == -1)
	{
		m_protocol_family->destroy_header(protocol_header);
		return ;
	}

	Protocol *protocol = m_protocol_family->create_protocol(protocol_header);
	if(protocol == NULL)
	{
		m_protocol_family->destroy_header(protocol_header);
		return ;
	}

	int body_size = protocol->get_body_size();
	read(fd, buf, body_size);
	if(protocol->decode_body(buf, body_size) == -1)
	{
		 m_protocol_family->destroy_protocol(protocol);
		 return;
	}
*/
/*  2. 解码(已知协议类型)
	ProtocolType protoco_type = get_type();

	Protocol *protocol = m_protocol_family->create_protocol(protoco_type);
	if(protocol == NULL)
		return ;

	int header_size = protocol->get_header_size();
	read(fd, buf, header_size);
	if(protocol->decode_header(buf, header_size) == -1)
	{
		m_protocol_family->destroy_protocol(protocol);
		return ;
	}

	int body_size = protocol->get_body_size();
	read(fd, buf, body_size);
	if(protocol->decode_body(buf, body_size) == -1)
	{
		 m_protocol_family->destroy_protocol(protocol);
		 return;
	}
*/
/* 3. 编码
	IOBuffer *io_buffer = get_io_buffer();
	Protocol *protocol = m_protocol_family->create_protocol(protoco_type);
	//to use protocol ...
	if(protocol->encode(io_buffer) == 0)
	{
		//to use io_buffer ...
	}
	m_protocol_family->destroy_protocol(protocol);
 */


//////////////////////////////////////////////////////
//必须实现decode和encode!!!                        //
//1. StringProtocol                                 //
//2.                                                //
//3.                                                //
//4.                                                //
//5.                                                //
//////////////////////////////////////////////////////

//简单的协议.直接传送buf数据char str[1024];
class StringProtocol:public DefaultProtocol
{
public:
	//编码协议体数据到io_buffer.成功返回编码后协议体长度(大于0),失败返回-1;
	int encode_body(IOBuffer *io_buffer);
	//解码包体.成功返回0,否则返回-1;
	int decode_body(const char* buf, int buf_size);
public:
	StringProtocol():DefaultProtocol(PROTOCOL_STRING){}
	void set_string(string data){m_data=data;}
	void set_string(const char *data){m_data.assign(data);}
	string get_string(){return m_data;}
private:
	string m_data;
};

#endif
