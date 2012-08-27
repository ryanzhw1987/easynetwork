#ifndef _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN
#define _LIB_PROTOCOL_DEFAULT_H_20120728_LIUYONGJIN

#include "Protocol.h"
#include "ProtocolType.h"

#include "MemManager.h"

/**************************** ����ṩ��Э���ʵ�� **************************/

///////////////////////////////////  DefaultHeader  ///////////////////////////////////
//Э��ͷ��maigc number
#define MAGIC_NUM	0X1F0696D9 //int:520525529
#define VERSION		1

class DefaultHeader:public ProtocolHeader
{
public: //ʵ�ָ��ຯ�� 
    virtual int get_header_size(){return sizeof(m_magic_num)+sizeof(m_version)+sizeof(m_body_size)+sizeof(m_type);	}
    virtual int get_body_size(){	return m_body_size;	}

   //��ͷ����.�ɹ�����0. ���򷵻�-1.
    virtual int encode(char *buf, int buf_size);
    //��ͷ����.�ɹ�����0,ʧ�ܷ���-1.
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
static MemCache<DefaultHeader> g_DefaultHeader_MemCache;

class Command
{
public:
	Command(ProtocolType type):m_type(type){}
	virtual ~Command(){}

	ProtocolType get_type(){return m_type;}

	//�ɹ�����0, ʧ�ܷ���-1;
	virtual int decode(const char* buf, int buf_size)=0;

	//����Э�������ݵ�io_buffer:
	//�ɹ����ر����ĳ���(����0)
	//ʧ�ܷ���-1, io_bufferû�б仯.(Э�������󳤶�Ϊ0ʱ,����ʧ�ܴ���,Ҳ����-1);
	virtual int encode(IOBuffer *io_buffer)=0;
private:
	int m_type;	
};

class DefaultProtocol: public Protocol
{
///////////////////////////////////////////////
///////      ���ി�麯��             /////////
///////////////////////////////////////////////
public:
	//Э�����,�ɹ�����0; ���򷵻�-1;
	virtual int encode(IOBuffer *io_buffer);
    //��ȡЭ��ͷ��С
    virtual int get_header_size(){return m_header->get_header_size();}
	//����Э��ͷ.�ɹ�����body_size�Ĵ�С.���򷵻�-1
    virtual int decode_header(const char* buf, int buf_size){return m_header->decode(buf, buf_size);}

	//��ȡЭ���峤��.
    virtual int get_body_size(){return m_header->get_body_size();}
    //�������.�ɹ�����0,���򷵻�-1;
    virtual int decode_body(const char* buf, int buf_size);

///////////////////////////////////////////////
///////      ��Э���Ա����           /////////
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

	//��ȡЭ������
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
	//virtual DefaultHeader* new_header(){return new DefaultHeader;}
	//virtual void destory_header(DefaultHeader* header){delete header;}
	virtual DefaultHeader* new_header(){return g_DefaultHeader_MemCache.Alloc();}
	virtual void destory_header(DefaultHeader* header){g_DefaultHeader_MemCache.Free(header);}
private:
	DefaultHeader *m_header;
	Command *m_body; //�����Э����;
};

class DefaultProtocolFamily:public ProtocolFamily
{
public:
	Protocol* create_protocol(){ return new DefaultProtocol;}
	Protocol* create_protocol(Command *cmd)
	{
	    DefaultProtocol *default_protocol = (DefaultProtocol*)create_protocol();
	    if(cmd != NULL)
	        default_protocol->attach_cmd(cmd);
	    return default_protocol;
	}
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
//����Commandʵ��                                   //
//����ʵ��decode��encode!!!                         //
//1. SimpleCmd                                      //
//2.                                                //
//3.                                                //
//4.                                                //
//5.                                                //
//////////////////////////////////////////////////////

//�򵥵�Э��.ֱ�Ӵ���buf����
class SimpleCmd:public Command
{
public://ʵ�ָ��ຯ��
   //�ɹ�����0, ʧ�ܷ���-1;
	int decode(const char* buf, int buf_size);

	//����Э����: ��encode_bufƫ��offset��ʼ����Э��������.
    //�ɹ�����0:  �����encode_buf�Ĵ�СΪЭ���峤����offset֮��.
    //ʧ�ܷ���-1: encode_buf��Ч
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
	int get_size(){return m_size;};
	
    int set_data(const char *data, int size);
private:
    char *m_buf;
    int m_size;
};

#endif
