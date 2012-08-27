#ifndef	_LIB_SOCKET_H_20120612_LIUYONGJIN
#define	_LIB_SOCKET_H_20120612_LIUYONGJIN

#include <stdio.h>
#include "IOBuffer.h"

typedef int SocketHandle;
#define SOCKET_INVALID -1

typedef enum
{
    BLOCK,
    NOBLOCK
}BlockMode;

///////////////////////////////////////////////////////////////////////////
//Socket
class Socket
{
public:
    //socket_handle:socket������
    //port:����/���ӵĶ˿ں�
    //ip:���ӵĵ�ַ
    //block_mode:����ģʽ. BLOCK(����)/NOBLOCK(������)
    Socket(SocketHandle socket_handle=SOCKET_INVALID, int port=-1, const char *ip=NULL, BlockMode block_mode=NOBLOCK);
    virtual ~Socket();
    //��socket���и�ֵ.�ɹ�����0, ���򷵻�-1(socket�Ѿ�����һ����Ч��socket_handle)
	int assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode);

	SocketHandle get_handle(){return m_socket_handle;}
	int get_port(){return m_port;}
	const char* get_ip(){return m_ip;}
	BlockMode get_block_mode(){return m_block_mode;}

protected:
	SocketHandle m_socket_handle;
	int m_port;
	char m_ip[20];
	BlockMode m_block_mode;
	void copy_ip(const char* ip);
};

///////////////////////////////////////////////////////////////////////////
//listen Socket.
class ListenSocket: public Socket
{
public:
    //port:�����˿�
    //block:�Ƿ�����.true(����)/false(������);Ĭ�Ϸ�����ģʽ
	ListenSocket(int port=-1, BlockMode block_mode=NOBLOCK):Socket(SOCKET_INVALID, port, NULL, block_mode){}
	virtual ~ListenSocket(){}

	//��ʼ����, �ɹ�����0, ʧ�ܷ���-1;
	virtual int open();
	virtual SocketHandle accept_connect();
};

typedef enum
{
	TRANS_OK=0,		//����
	TRANS_CLOSE=-1,	//���ӹر�
	TRANS_ERROR=-2,	//����
	TRANS_NOMEM=-3,	//û���ڴ�
	TRANS_PENDING=-4,	//���ݷ��Ͳ���ȥ
	TRANS_BLOCK=-5,   //����ģʽ
	TRANS_NODATA=-6,  //��������
}TransStatus;

///////////////////////////////////////////////////////////////////////////
//data transmission socket.
//Just receive/send data. The data may be transmited unfully,and the caller should deal with this situation.
class TransSocket: public Socket
{
public:
    TransSocket():Socket(SOCKET_INVALID, -1, NULL, NOBLOCK){}
	TransSocket(const char *ip, int port, BlockMode block_mode=NOBLOCK):Socket(SOCKET_INVALID, port, ip, block_mode){}
	virtual ~TransSocket(){}

	//������������.��ʱtimeout_ms����(Ĭ��2s)δ���ϵ�������ʧ��.
	//�ɹ�����0, ʧ�ܷ���-1
    virtual int connect_server(int timeout_ms=2000);

	//���Խ���ָ�����ȵ�����.
	//����ֵ:
	//����0:�ɹ����ض�ȡ���ֽ���(�����ǲ�������).
	//TRANS_CLOSE: ���������ر�
	//TRANS_NODATA: û������
	//TRANS_ERROR: ʧ��
	virtual int recv_data(char *buffer, int len);

	//����ָ�����ȵ�����(ȫ������)
	//����ֵ:
	//����0: ���͵��ֽ���
	//TRANS_ERROR: ʧ��
	virtual int send_data(char *buffer, int len);

	//��ȡ���뻺����
	IOBuffer* get_recv_buffer(){return &m_recv_buffer;}
	//��ȡ���������
	IOBuffer* get_send_buffer(){return &m_send_buffer;}
	
	//�����������ݵ����뻺����.!!!***�����ڷ�����ģʽ***!!!
	//����ֵ:
	//TRANS_OK:�ɹ�
	//TRANS_NOMEM: û���ڴ�
	//TRANS_ERROR: ����
	//TRANS_CLOSE: �Զ˹ر�����
	//TRANS_BLOCK: ��ǰ������ģʽ
	TransStatus recv_buffer();

	//���Է�������������е���������,ֱ��������ɻ��߷��Ͳ���ȥ.
	//����ֵ:
	//TRANS_OK:�ɹ�
	//TRANS_PENDING: ֻ���Ͳ�������,�������������ݴ�����
	//TRANS_ERROR: ����
	TransStatus send_buffer();

protected:
    virtual int init_active_socket(); //��ʼ����������, �ɹ�����0, ʧ�ܷ���-1
private:
	IOBuffer m_recv_buffer;
	IOBuffer m_send_buffer;
};

#endif  //_LIB_SOCKET_H_20120612_LIUYONGJIN


