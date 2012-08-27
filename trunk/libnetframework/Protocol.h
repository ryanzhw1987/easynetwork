#ifndef _LIB_PROTOCOL_H_20120615_LIUYONGJIN
#define _LIB_PROTOCOL_H_20120615_LIUYONGJIN

#include <stdio.h>
#include "IOBuffer.h"

//Э��ͷ�ӿ�
class ProtocolHeader
{
public:    
    virtual ~ProtocolHeader(){}

    //����Э��ͷ��С.
    virtual int get_header_size()=0;
	//���ذ��峤��.
	virtual int get_body_size()=0;
    //��ͷ����.�ɹ�����0. ���򷵻�-1.
    virtual int encode(char *buf, int buf_size)=0;
    //��ͷ����.�ɹ�����0,ʧ�ܷ���-1.
    virtual int decode(const char *buf, int buf_size)=0;
};
//sample
/*
	//1. ����
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

	//2. ����
	ProtocolHeader *header = create_header();
	int header_size = header->get_header_size();
	header->encode(buf, header_size);

	delete header;
*/


class Protocol
{
public:
	virtual ~Protocol(){}
	///////////////////////////////////////// ���� //////////////////////////
    //Э�����,�ɹ�����0; ���򷵻�-1;
	virtual int encode(IOBuffer *io_buffer) = 0;
	///////////////////////////////////////// ���� //////////////////////////
    //��ȡЭ��ͷ��С
    virtual int get_header_size()=0;
    //����Э��ͷ.���򷵻�-1
    virtual int decode_header(const char* buf, int buf_size)=0;
	//��ȡЭ���峤��.
    virtual int get_body_size()=0;
    //�������.�ɹ�����0,���򷵻�-1;
    virtual int decode_body(const char* buf, int buf_size)=0;
};

//���ڴ�������protocol�Ĺ�����
class ProtocolFamily
{
public:
    virtual Protocol* create_protocol()=0;
};

//sample
/*
	//1. ����
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


	//2. ����
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

