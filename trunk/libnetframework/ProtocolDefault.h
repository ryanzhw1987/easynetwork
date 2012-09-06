#ifndef _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN
#define _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN

#include "Protocol.h"
#include "ProtocolType.h"

#include "MemManager.h"

/**************************** 框架提供的协议的实现 **************************/

///////////////////////////////////  DefaultHeader  ///////////////////////////////////
//协议头的maigc number
#define MAGIC_NUM	0X1F0696D9 //int:520525529
#define VERSION		1

class DefaultHeader:public ProtocolHeader
{
public: //实现父类函数 
	virtual int get_header_size(){return sizeof(m_magic_num)+sizeof(m_version)+sizeof(m_body_size)+sizeof(m_type);	}
	virtual int get_body_size(){	return m_body_size;	}

   //包头编码.成功返回0. 否则返回-1.
	virtual int encode(char *buf, int buf_size);
	//包头解码.成功返回0,失败返回-1.
	virtual int decode(const char *buf, int buf_size);

public:
	DefaultHeader(ProtocolType type=PROTOCOL_INVALID, int version=VERSION, int magic_num=0X1F0696D9)
							:m_magic_num(magic_num)
							,m_version(version)
							,m_body_size(0)
							,m_type(type){}
	virtual ~DefaultHeader(){}
	int get_magic_num(){return m_magic_num;}
	int get_version(){return m_version;}

	ProtocolType get_type(){return m_type;}
	void set_type(ProtocolType type){m_type = type;}
	void set_body_size(int body_size){m_body_size = body_size;}
private:
	int m_magic_num;
	int m_version;
	int m_body_size;
	int m_type;
};

class Command
{
public:
	Command(ProtocolType type):m_type(type){}
	virtual ~Command(){}

	ProtocolType get_type(){return m_type;}

	//成功返回0, 失败返回-1;
	virtual int decode(const char* buf, int buf_size)=0;

	//编码协议体数据到io_buffer:
	//成功返回编码后的长度(大于0)
	//失败返回-1, io_buffer没有变化.(协议体编码后长度为0时,当作失败处理,也返回-1);
	virtual int encode(IOBuffer *io_buffer)=0;
private:
	int m_type;	
};

class DefaultProtocol: public Protocol
{
///////////////////////////////////////////////
///////      基类纯虚函数             /////////
///////////////////////////////////////////////
public:
	//协议编码,成功返回0; 否则返回-1;
	virtual int encode(IOBuffer *io_buffer);
	//获取协议头大小
	virtual int get_header_size(){return m_header->get_header_size();}
	//解码协议头.成功返回body_size的大小.否则返回-1
	virtual int decode_header(const char* buf, int buf_size){return m_header->decode(buf, buf_size);}

	//获取协议体长度.
	virtual int get_body_size(){return m_header->get_body_size();}
	//解码包体.成功返回0,否则返回-1;
	virtual int decode_body(const char* buf, int buf_size);

///////////////////////////////////////////////
///////      本协议成员函数           /////////
///////////////////////////////////////////////
public:
	DefaultProtocol()
	{
		m_header = new_header();
		m_body = NULL;		
	}
	virtual ~DefaultProtocol()
	{
		destory_header(m_header);
		m_header = NULL;

		if(m_body != NULL)
			delete m_body;
		m_body = NULL;
	}

	//获取协议类型
	ProtocolType get_type(){return m_header->get_type();}
	Command* get_cmd(){return m_body;}
	Command* attach_cmd(Command *cmd)
	{
		Command* temp = m_body;
		m_body = cmd;
		m_header->set_type(m_body->get_type());
		return temp;
	}
	Command* detach_cmd()
	{
		Command* cmd = m_body;
		m_body = NULL;
		return cmd;
	}
protected:
	virtual DefaultHeader* new_header()
	{
		//return new DefaultHeader;
		return m_defautlheader_memcache.Alloc();
	}
	virtual void destory_header(DefaultHeader* header)
	{
		//delete header;
		m_defautlheader_memcache.Free(header);
	}
private:
	DefaultHeader *m_header;
	Command *m_body; //具体的协议体;

	//memcache for class DefaultHeader;
	static MemCache<DefaultHeader> m_defautlheader_memcache;
};

class DefaultProtocolFamily:public ProtocolFamily
{
public:
	Protocol* create_protocol()
	{
		//return new DefaultProtocol;
		return m_protocol_memcache.Alloc();
	}
	Protocol* create_protocol(Command *cmd)
	{
		DefaultProtocol *default_protocol = (DefaultProtocol*)create_protocol();
		if(cmd != NULL)
			default_protocol->attach_cmd(cmd);
		return default_protocol;
	}

	bool destroy_protocol(Protocol* protocol)
	{
		//delete protocol;
		//return true;

		DefaultProtocol *temp = (DefaultProtocol *)protocol;
		return m_protocol_memcache.Free(temp);
	}
private:
	//memcache for class DefaultProtocol;
	MemCache<DefaultProtocol> m_protocol_memcache;
};

//smaple
/*
	DefalutProtocol *protocol = new DefalutProtocol;
	//DefalutProtocol *protocol = default_protocol_family.create_protocol();

	Command *cmd = create_cmd();
	//use cmd;
	protocol->set_cmd(cmd);

	EncodeBuffer encode_buffer;
	protocol->endcode(&encode_buffer);
*/


//////////////////////////////////////////////////////
//具体Command实现                                   //
//必须实现decode和encode!!!                         //
//1. SimpleCmd                                      //
//2.                                                //
//3.                                                //
//4.                                                //
//5.                                                //
//////////////////////////////////////////////////////

//简单的协议.直接传送buf数据
class SimpleCmd:public Command
{
public://实现父类函数
   //成功返回0, 失败返回-1;
	int decode(const char* buf, int buf_size);

	//编码协议体: 从encode_buf偏移offset开始放置协议体数据.
	//成功返回0:  编码后encode_buf的大小为协议体长度与offset之和.
	//失败返回-1: encode_buf无效
	int encode(IOBuffer *io_buffer);

public:
	SimpleCmd():Command(PROTOCOL_SIMPLE)
	{
		m_buf = NULL;
		m_size = 0;
	}
	
	~SimpleCmd()
	{
		if(m_buf != NULL)
			delete m_buf;
		m_buf = NULL;
		m_size = 0;
	}
	const char* get_data(){return m_buf;}
	int get_size(){return m_size;}
	
	int set_data(const char *data, int size);
private:
	char *m_buf;
	int m_size;
};

#endif
