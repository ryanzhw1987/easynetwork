#ifndef	_LIB_SOCKET_H_20120612_LIUYONGJIN
#define	_LIB_SOCKET_H_20120612_LIUYONGJIN

#include <stdio.h>
#include "SocketType.h"
#include "IOBuffer.h"

#include <deque>
using std::deque;

class Socket
{
public:
	//socket_handle:socket描述符
	//port:监听/连接的端口号
	//ip:连接的地址
	//block_mode:阻塞模式
	Socket(SocketHandle socket_handle=SOCKET_INVALID, int port=-1, const char *ip=NULL, BlockMode block_mode=BLOCK);
	virtual ~Socket();
	//对socket进行赋值.成功返回true, 否则返回false(socket已经含有一个有效的socket_handle)
	bool assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode);

	SocketHandle get_handle(){return m_socket_handle;}
	int get_port(){return m_port;}
	const char* get_ip(){return m_ip;}
	BlockMode get_block_mode(){return m_block_mode;}

	//打开socket. 成功返回true, 失败返回false
	//timeout_ms: 打开socket的超时时间.
	virtual bool open(int timeout_ms)=0;
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
	//port:监听端口
	ListenSocket(int port=-1, BlockMode block_mode=BLOCK):Socket(SOCKET_INVALID, port, NULL, block_mode){}
	virtual ~ListenSocket(){}

	//开始监听.成功返回true, 失败返回false
	virtual bool open(int timeout_ms=2000);
};

///////////////////////////////////////////////////////////////////////////
#define TRANS_ERROR  -1
class TransSocket: public Socket
{
public:
	TransSocket()
		:Socket(SOCKET_INVALID, -1, NULL, BLOCK)
		,m_send_queue_size(0)
		,m_recv_queue_size(0)
	{}

	TransSocket(const char *ip, int port, BlockMode block_mode=BLOCK)
		:Socket(SOCKET_INVALID, port, ip, block_mode)
		,m_send_queue_size(0)
		,m_recv_queue_size(0)
	{}

	virtual ~TransSocket();

	//打开主动连接. 成功返回true, 失败返回false.
	//timeout_out: 打开socket的超时时间.
	virtual bool open(int timeout_ms=2000);

	//尝试接收指定长度的数据(可能只接收部分数据).
	//返回值:
	//成功: 返回收到的字节数(大于等于0).
	//错误: 返回TRANS_ERROR
	int recv_data(char *buffer, int len);

	//接收指定长度的数据(全部接收).
	//返回值:
	//成功: 返回指定接收的数据大小len.
	//错误: 返回TRANS_ERROR
	int recv_data_all(char *buffer, int len);

	//尝试发送指定长度的数据(可能只发送部分数据)
	//返回值:
	//成功: 返回发送的字节数(大于等于0)
	//失败: 返回TRANS_ERROR
	int send_data(char *buffer, int len);

	//发送指定长度的数据(全部发送)
	//返回值:
	//成功: 返回指定发送的数据大小len
	//失败: 返回TRANS_ERROR
	int send_data_all(char *buffer, int len);

	//添加待发送的数据到等待队列
	bool push_send_buffer(IOBuffer *io_buffer);

	//尽量发送等待队列中的数据
	//返回值:
	//大于等于0: 剩下未发送的数据大小
	//TRANS_ERROR: 错误
	int send_buffer();

	//接收数据
	//io_buffer: 接收数据的缓存区
	//len: 待接收的数据大小
	//wait_all: true(阻塞直到所有的size字节都收到), false(不等待)
	//返回值:
	//大于等于0: 返回接收的字节数
	//TRANS_ERROR: 错误
	int recv_buffer(IOBuffer *io_buffer, int len, bool wait_all);

	//将io_buffer保存到待接收队列末尾
	bool push_recv_buffer(IOBuffer *io_buffer);

	//从待接收队列头部获取一个io_buffer(该io_buffer从队列头移除)
	IOBuffer* pop_recv_buffer();
protected:
	virtual int init_active_socket(); //初始化主动连接, 成功返回0, 失败返回-1
private:
	deque<IOBuffer*> m_send_queue; //待发送队列
	int m_send_queue_size; //待发送队列中数据大小

	deque<IOBuffer*> m_recv_queue; //待接收队列,保存不完整的数据包(队列里面的数据按其在包中的顺序存放)
	int m_recv_queue_size;
};

#endif  //_LIB_SOCKET_H_20120612_LIUYONGJIN


