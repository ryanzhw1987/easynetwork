#include "Socket.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "slog.h"
///////////////////////////////////////////////////////////////
///////////////             Socket          ///////////////
///////////////////////////////////////////////////////////////
void Socket::copy_ip(const char *ip)
{
	m_ip[0] = '\0';
	if(ip==NULL || strlen(ip)<=0)
		return;
	int size = sizeof(m_ip)-1;
	strncpy(m_ip, ip, size);
	m_ip[size] = '\0';
}

Socket::Socket(SocketHandle socket_handle/*=SOCKET_INVALID*/, int port/*=-1*/, const char *ip/*=NULL*/, BlockMode block_mode/*=NOBLOCK*/)
{
	m_socket_handle = socket_handle;
	m_port = port;
	copy_ip(ip);
	m_block_mode = block_mode;
}

Socket::~Socket()
{
	if(m_socket_handle != SOCKET_INVALID)
	{
		SLOG_TRACE("close fd=%d in ~Socket()", m_socket_handle);
		close(m_socket_handle);
		m_socket_handle = SOCKET_INVALID;
	}
}

bool Socket::assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode)
{
	if(m_socket_handle != SOCKET_INVALID)
		return false;

	m_socket_handle = socket_handle;
	m_port = port;
	copy_ip(ip);
	m_block_mode = block_mode;
	return true;
}


///////////////////////////////////////////////////////////////
///////////////      ListenSocket        ///////////////
///////////////////////////////////////////////////////////////
bool ListenSocket::open(int timeout_ms/*=2000*/)
{
	if(m_socket_handle!=SOCKET_INVALID || m_port<=0)
		return false;

	//1. 创建socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
		return false;

	//2. 设置属性
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1 )
	{
		SLOG_ERROR("fcntl<get> failed. errno=%d", errno);
		close(fd);
		return false;
	}

	if(m_block_mode == NOBLOCK)  //non block mode
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	flags |= FD_CLOEXEC;  //close on exec
	if(fcntl(fd, F_SETFL, flags) == -1)
	{
		SLOG_ERROR("<set> failed. errno=%d", errno);
		close(fd);
		return false;
	}

	//set reuse
	int reuse = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == -1)
	{
		SLOG_ERROR("set socket SO_REUSEADDR option failed, errno=%d",errno);
		close(fd);
		return false;
	}

	//3. 绑定到端口
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);
	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		SLOG_ERROR("bind failed, errno=%d",errno);
		close(fd);
		return false;
	}

	//4. 监听端口.监听队列中等待accept的最大连接数设置为默认值
	if(listen(fd, 128) == -1)
	{
		SLOG_ERROR("listen failed, errno=%d",errno);
		close(fd);
		return false;
	}

	m_socket_handle = fd;
	return true;
}

///////////////////////////////////////////////////////////////
///////////////       TransSocket       ///////////////
///////////////////////////////////////////////////////////////
bool TransSocket::open(int timeout_ms/*=2000*/)
{
	if(m_socket_handle!=SOCKET_INVALID || strlen(m_ip)<=0 || m_port <=0)
		return false;

	//1. 创建socket
	m_socket_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket_handle < 0)
	{
		SLOG_ERROR("create active socket error");
		m_socket_handle = SOCKET_INVALID;
		return false;
	}

	//2. 初始化
	if(init_active_socket() == -1)
	{
		SLOG_ERROR("init active socket error. close fd=%d", m_socket_handle);
		close(m_socket_handle);
		m_socket_handle = SOCKET_INVALID;
		return false;
	}

	//3. 连接
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_ip);
	addr.sin_port = htons(m_port);
	if(connect(m_socket_handle, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		if(m_block_mode==NOBLOCK && (errno==EINPROGRESS||errno==EINTR))	//非阻塞并且等待建立连接
		{
			struct timeval tval;
			fd_set rset, wset;
			FD_ZERO(&rset);
			FD_SET(m_socket_handle, &rset);
			wset = rset;

			tval.tv_sec  = timeout_ms/1000;
			tval.tv_usec = (timeout_ms%1000)*1000;

			int tmp = select(m_socket_handle+1, (fd_set*)&rset, (fd_set*)&wset, (fd_set*)NULL, &tval);
			if (tmp <= 0)
			{
				SLOG_ERROR("select failed when connecting server:%s:%d. errno=%d", m_ip, m_port, errno);
				close(m_socket_handle);
				m_socket_handle = SOCKET_INVALID;
				return false;
			}
			if(FD_ISSET(m_socket_handle,&rset) || FD_ISSET(m_socket_handle,&wset))
			{
				int error;
				int len = sizeof(error);
				tmp = getsockopt(m_socket_handle, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t*)&len);
				if(tmp<0 || (tmp==0&&error!=0))
				{
					SLOG_ERROR("other error when connecting server:%s:%d. errno=%d", m_ip, m_port, error);
					close(m_socket_handle);
					m_socket_handle = SOCKET_INVALID;
					return false;
				}
			}
		}		
		else
		{
			SLOG_ERROR("connect server:%s:%d. errno=%d", m_ip, m_port, errno);
			close(m_socket_handle);
			m_socket_handle = SOCKET_INVALID;
			return false;
		}
	}

	return true;
}

//初始化主动连接
int TransSocket::init_active_socket()
{
	//设置socket属性
	int flags = fcntl(m_socket_handle, F_GETFL, 0);
	if(flags == -1 )
	{
		SLOG_ERROR("fcntl<get> active socket faile. errno=%d", errno);
		return -1;
	}

	if(m_block_mode==NOBLOCK) //non block mode
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if (fcntl(m_socket_handle, F_SETFL, flags) == -1 )
	{
		SLOG_ERROR("fcntl<set> active socket faile. errno=%d", errno);
		return -1;
	}

	return 0;
}

//返回值:
//大于0:成功返回读取的字节数(可能是部分数据).
//TRANS_CLOSE: 连接正常关闭
//TRANS_NODATA: 没有数据
//TRANS_ERROR: 失败
int TransSocket::recv_data(char *buffer, int len)
{
	assert(len>0);
	assert(buffer!=NULL);

	int ret = 0;
	int read_size = 0; //已读数据大小
	int need_size = 0; //剩下未读数据大小

	while(read_size<len)
	{
		need_size = len-read_size;
		ret = recv(m_socket_handle, buffer+read_size, need_size, 0);
		if(ret > 0)
			read_size += ret;
		else if(ret == 0) //对端断开连接
			return TRANS_CLOSE;
		else
		{
			SLOG_TRACE("receive data return -1. errno=%d",errno);
			if(errno==EINTR || errno==EWOULDBLOCK || errno==EAGAIN)
				break;
			return TRANS_ERROR; 	//失败
		}
	}

	if(read_size > 0)
	{
		SLOG_TRACE("receive data succ. len=%d", read_size);
		return read_size;
	}
	else
		return TRANS_NODATA;
}

//发送指定长度的数据(全部发送)
//返回值:
//大于0: 发送的字节数
//TRANS_ERROR: 失败
int TransSocket::send_data(char *buffer, int len)
{
	assert(len>0);
	assert(buffer!=NULL);

	int ret = 0;
	int send_size = 0;
	int need_size = 0;

	while(send_size < len)
	{
		need_size = len-send_size;
		ret = send(m_socket_handle, buffer+send_size, need_size, 0);
		if(ret > 0)
			send_size += ret;
		else
		{
			SLOG_TRACE("send_data return -1. errno=%d", errno);
			if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  //中断,重试
				continue;
			return TRANS_ERROR;
		}
	}

	SLOG_TRACE("send data succ. len=%d", send_size);
	return send_size;
}


//接收所有数据到输入缓冲区. !!!***仅用于非阻塞模式***!!!
//返回值:
//TRANS_OK:成功
//TRANS_NOMEM: 没有内存
//TRANS_ERROR: 错误
//TRANS_CLOSE: 对端关闭链接
//TRANS_BLOCK: 当前是阻塞模式
TransStatus TransSocket::recv_buffer()
{
	if(m_block_mode == BLOCK)
		return TRANS_BLOCK;

	unsigned int read_size;
	char * buffer;
	while(true)
	{
		read_size = 1024;
		buffer = m_recv_buffer.write_open(read_size);
		if(buffer == NULL)
		{
			SLOG_ERROR("get write buffer error.");
			return TRANS_NOMEM;
		}

		read_size = recv(m_socket_handle, buffer, read_size, 0);
		if(read_size < 0)
		{
			if(errno == EINTR)  //被中断,继续读
			{
				SLOG_TRACE("recv data interupt,continue to read");
				continue;
			}
			else if(errno==EWOULDBLOCK || errno==EAGAIN)  //暂无数据
			{
				SLOG_TRACE("recv data. no data now");
				break;
			}
			else  //错误
			{
				SLOG_ERROR("recv data error. errno=%d",errno);
				return TRANS_ERROR;
			}
		}
		else if(read_size == 0) //对端断开连接
		{
			SLOG_ERROR("client closed the connect gracefully.fd=%d", m_socket_handle);
			return TRANS_CLOSE;
		}

		m_recv_buffer.write_close(read_size);
		if(read_size < 1024) //已经读完数据
			break;
	}

	return TRANS_OK;
}

//尝试发送输出缓冲区中的所有数据,直到发送完成或者发送不出去.
//返回值:
//TRANS_OK:成功
//TRANS_PENDING: 只发送部分数据,缓冲区还有数据待发送
//TRANS_ERROR: 错误
TransStatus TransSocket::send_buffer()
{
	unsigned int size;
	const char *buffer = m_send_buffer.read_open(size);
	if(buffer == NULL)
		return TRANS_OK;
	while(true)
	{
		unsigned int ret = send(m_socket_handle, buffer, size, 0);
		if(ret > 0)
		{
			m_send_buffer.read_close(ret);
			if(ret < size)
				return TRANS_PENDING;  //只发送了一部分数据
			else
				return TRANS_OK;		//全部发送
		}
		else if(errno == EINTR) //中断重试
		{
			SLOG_TRACE("send buffer data interrupted,retry to send.");
			continue;
		}
		else if(errno == EWOULDBLOCK || errno == EAGAIN) //暂时发送不出去
		{
			SLOG_DEBUG("send buffer data pending,return");
			return TRANS_PENDING;
		}
		else
		{
			SLOG_ERROR("send buffer data error. errno=%d", errno);
			return TRANS_ERROR;
		}
	}

	return TRANS_OK;
}

