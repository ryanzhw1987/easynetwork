#ifndef	_LIB_SOCKET_H_20120612_LIUYONGJIN
#define	_LIB_SOCKET_H_20120612_LIUYONGJIN

#include <stdio.h>
#include "SocketType.h"
#include "IOBuffer.h"

class Socket
{
public:
	//socket_handle:socket描述符
	//port:监听/连接的端口号
	//ip:连接的地址
	//block_mode:阻塞模式. BLOCK(阻塞)/NOBLOCK(非阻塞)
	Socket(SocketHandle socket_handle=SOCKET_INVALID, int port=-1, const char *ip=NULL, BlockMode block_mode=NOBLOCK);
	virtual ~Socket();
	//对socket进行赋值.成功返回true, 否则返回false(socket已经含有一个有效的socket_handle)
	bool assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode);

	SocketHandle get_handle(){return m_socket_handle;}
	int get_port(){return m_port;}
	const char* get_ip(){return m_ip;}
	BlockMode get_block_mode(){return m_block_mode;}

	//成功返回true, 失败返回false
	//timeout_out: 打开socket的超时时间.
	virtual bool open(int timeout_ms=2000)=0;
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
	//block:是否阻塞.true(阻塞)/false(非阻塞);默认非阻塞模式
	ListenSocket(int port=-1, BlockMode block_mode=NOBLOCK):Socket(SOCKET_INVALID, port, NULL, block_mode){}
	virtual ~ListenSocket(){}

	//开始监听.成功返回true, 失败返回false
	//timeout_out: 打开socket的超时时间.
	virtual bool open(int timeout_ms=2000);
};

///////////////////////////////////////////////////////////////////////////
typedef enum
{
	TRANS_OK=0,		//正常
	TRANS_CLOSE=-1,	//链接关闭
	TRANS_ERROR=-2,	//错误
	TRANS_NOMEM=-3,	//没有内存
	TRANS_PENDING=-4,	//数据发送不出去
	TRANS_BLOCK=-5,   //阻塞模式
	TRANS_NODATA=-6,  //暂无数据
}TransStatus;


//data transmission socket.
//Just receive/send data. The data may be transmited unfully,and the caller should deal with this situation.
class TransSocket: public Socket
{
public:
	TransSocket():Socket(SOCKET_INVALID, -1, NULL, NOBLOCK){}
	TransSocket(const char *ip, int port, BlockMode block_mode=NOBLOCK):Socket(SOCKET_INVALID, port, ip, block_mode){}
	virtual ~TransSocket(){}

	//打开主动连接. 成功返回true, 失败返回false.
	//timeout_out: 打开socket的超时时间.
	virtual bool open(int timeout_ms=2000);

	//尝试接收指定长度的数据.
	//返回值:
	//大于0:成功返回读取的字节数(可能是部分数据).
	//TRANS_CLOSE: 连接正常关闭
	//TRANS_NODATA: 没有数据
	//TRANS_ERROR: 失败
	virtual int recv_data(char *buffer, int len);

	//发送指定长度的数据(全部发送)
	//返回值:
	//大于0: 发送的字节数
	//TRANS_ERROR: 失败
	virtual int send_data(char *buffer, int len);

	//获取输入缓冲区
	IOBuffer* get_recv_buffer(){return &m_recv_buffer;}
	//获取输出缓冲区
	IOBuffer* get_send_buffer(){return &m_send_buffer;}

	//接收所有数据到输入缓冲区.!!!***仅用于非阻塞模式***!!!
	//返回值:
	//TRANS_OK:成功
	//TRANS_NOMEM: 没有内存
	//TRANS_ERROR: 错误
	//TRANS_CLOSE: 对端关闭链接
	//TRANS_BLOCK: 当前是阻塞模式
	TransStatus recv_buffer();

	//尝试发送输出缓冲区中的所有数据,直到发送完成或者发送不出去.
	//返回值:
	//TRANS_OK:成功
	//TRANS_PENDING: 只发送部分数据,缓冲区还有数据待发送
	//TRANS_ERROR: 错误
	TransStatus send_buffer();

protected:
	virtual int init_active_socket(); //初始化主动连接, 成功返回0, 失败返回-1
private:
	IOBuffer m_recv_buffer;
	IOBuffer m_send_buffer;
};

#endif  //_LIB_SOCKET_H_20120612_LIUYONGJIN


