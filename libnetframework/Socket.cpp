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
		SLOG_TRACE("~Socket() close socket. fd=%d", m_socket_handle);
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
bool ListenSocket::open(int /*timeout_ms=2000*/)
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
		SLOG_ERROR("fcntl<get> failed. errno=%d(%s)", errno, strerror(errno));
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
		SLOG_ERROR("<set> failed. errno=%d(%s)", errno, strerror(errno));
		close(fd);
		return false;
	}

	//set reuse
	int reuse = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == -1)
	{
		SLOG_ERROR("set socket SO_REUSEADDR option failed, errno=%d(%s)",errno, strerror(errno));
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
		SLOG_ERROR("bind failed, errno=%d(%s)",errno, strerror(errno));
		close(fd);
		return false;
	}

	//4. 监听端口.监听队列中等待accept的最大连接数设置为默认值
	if(listen(fd, 128) == -1)
	{
		SLOG_ERROR("listen failed, errno=%d(%s)",errno, strerror(errno));
		close(fd);
		return false;
	}

	m_socket_handle = fd;
	return true;
}

///////////////////////////////////////////////////////////////
///////////////       TransSocket       ///////////////
///////////////////////////////////////////////////////////////
TransSocket::~TransSocket()
{
	while(!m_send_queue.empty())
	{
		ByteBuffer *byte_buffer = m_send_queue.front();
		m_send_queue.pop_front();
		delete byte_buffer;
	}
	SLOG_DEBUG("~TransSocket().remain %d bytes data unsend", m_send_queue_size);

	while(!m_recv_queue.empty())
	{
		ByteBuffer *byte_buffer = m_recv_queue.front();
		m_recv_queue.pop_front();
		delete byte_buffer;
	}
	SLOG_DEBUG("~TransSocket().remain %d bytes data unrecv", m_recv_queue_size);
}

bool TransSocket::open(int timeout_ms/*=2000*/)
{
	if(m_socket_handle!=SOCKET_INVALID || strlen(m_ip)<=0 || m_port <=0)
		return false;

	//1. 创建socket
	m_socket_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket_handle < 0)
	{
		SLOG_ERROR("create active socket error, errno=%d(%s)", errno, strerror(errno));
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
				SLOG_ERROR("select failed when connecting server[ip=%s,port=%d]. errno=%d(%s)", m_ip, m_port, errno, strerror(errno));
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
					SLOG_ERROR("other error when connecting server[ip=%s,port=%d]. errno=%d(%s)", m_ip, m_port, error, strerror(errno));
					close(m_socket_handle);
					m_socket_handle = SOCKET_INVALID;
					return false;
				}
			}
		}		
		else
		{
			SLOG_ERROR("connect server[ip=%s,port=%d]failed. errno=%d(%s)", m_ip, m_port, errno, strerror(errno));
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
		SLOG_ERROR("fcntl<get> active socket faile. errno=%d(%s)", errno, strerror(errno));
		return -1;
	}

	if(m_block_mode==NOBLOCK) //non block mode
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if (fcntl(m_socket_handle, F_SETFL, flags) == -1 )
	{
		SLOG_ERROR("fcntl<set> active socket faile. errno=%d(%s)", errno, strerror(errno));
		return -1;
	}

	return 0;
}

//尝试接收指定长度的数据(可能只接收部分数据).
//返回值:
//成功: 返回收到的字节数(大于等于0).
//错误: 返回TRANS_ERROR
int TransSocket::recv_data(char *buffer, int len)
{
	assert(buffer!=NULL && len>0);
	int ret = recv(m_socket_handle, buffer, len, 0);
	if(ret > 0)
		return ret;
	if(ret == 0)
	{
		SLOG_ERROR("peer close socket gracefully. fd=%d", m_socket_handle);
		return TRANS_ERROR;
	}
	if(errno==EINTR || errno==EWOULDBLOCK || errno==EAGAIN)
		return 0;

	SLOG_ERROR("receive data error. errno=%d(%s)",errno, strerror(errno));
	return TRANS_ERROR;
}

//接收指定长度的数据(全部接收).
//返回值:
//成功: 返回指定接收的数据大小len.
//错误: 返回TRANS_ERROR
int TransSocket::recv_data_all(char *buffer, int len)
{
	assert(buffer!=NULL && len>0);
	int ret = 0;
	int read_size = 0; //已读数据大小
	int need_size = 0; //剩下未读数据大小
	while(read_size < len)
	{
		need_size = len-read_size;
		ret = recv_data(buffer+read_size, need_size);
		if(ret == TRANS_ERROR)
			return ret;
		read_size += ret;
	}

	return read_size;
}

//尝试发送指定长度的数据(可能只发送部分数据)
//返回值:
//成功: 返回发送的字节数(大于等于0)
//失败: 返回TRANS_ERROR
int TransSocket::send_data(char *buffer, int len)
{
	assert(buffer!=NULL && len>0);
	int ret = send(m_socket_handle, buffer, len, 0);
	if(ret > 0)//非阻塞时可能只发送部分数据
		return ret;
	if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  //中断,重试
		return 0;

	SLOG_ERROR("send_data return error. errno=%d(%s)", errno, strerror(errno));
	return TRANS_ERROR;
}

//发送指定长度的数据(全部发送)
//返回值:
//成功:返回指定发送的数据大小len
//失败: 返回TRANS_ERROR
int TransSocket::send_data_all(char *buffer, int len)
{
	assert(buffer!=NULL && len>0);
	int ret = 0;
	int send_size = 0;
	int need_size = 0;
	while(send_size < len)
	{
		need_size = len-send_size;
		ret = send_data(buffer+send_size, need_size);
		if(ret == TRANS_ERROR)
			return TRANS_ERROR;
		send_size += ret;
	}

	return send_size;
}

//添加待发送的数据到等待队列
bool TransSocket::push_send_buffer(ByteBuffer *byte_buffer)
{
	if(byte_buffer==NULL)
		return false;
	m_send_queue.push_back(byte_buffer);
	m_send_queue_size += byte_buffer->size();

	SLOG_DEBUG("total %d bytes data waiting to send on socket fd=%d", m_send_queue_size, m_socket_handle);
	return true;
}

//尝试发送等待队列中的数据
//返回值:
//大于等于0: 剩下未发送的数据大小
//TRANS_ERROR: 错误
int TransSocket::send_buffer()
{
	SLOG_DEBUG("try to send %d bytes data of socket fd=%d", m_send_queue_size, m_socket_handle);

	int ret = 0;
	int size = 0;
	char *buf = NULL;
	while(!m_send_queue.empty())
	{
		ByteBuffer *byte_buffer = m_send_queue.front();
		assert(byte_buffer != NULL);
		size = byte_buffer->size();
		if(size <= 0)
			continue;
		buf = byte_buffer->get_data();
		ret = send_data(buf+m_send_size, size-m_send_size);
		if(ret == TRANS_ERROR)
			return ret;
		m_send_queue_size -= ret;
		if(ret < size-m_send_size)
		{
			m_send_size += ret;
			break;
		}
		m_send_size = 0;
		m_send_queue.pop_front();
		delete byte_buffer;
	}
	return m_send_queue_size;
}

int TransSocket::recv_buffer(ByteBuffer *byte_buffer, int len, bool wait_all=true)
{
	assert(byte_buffer!=NULL && len>0);
	char *buf = byte_buffer->get_append_buffer(len);
	if(buf == NULL)
	{
		SLOG_ERROR("recv buffer no momory");
		return 0;
	}
	int ret = 0;
	if(wait_all)
	{
		ret = recv_data_all(buf, len);
		assert(ret == len);
	}
	else
		ret = recv_data(buf, len);

	if(ret == TRANS_ERROR)
		return ret;
	else
		byte_buffer->set_append_size(ret);
	return ret;
}

//将io_buffer保存到待接收队列末尾
bool TransSocket::push_recv_buffer(ByteBuffer *byte_buffer)
{
	if(byte_buffer == NULL)
		return false;
	m_recv_queue.push_back(byte_buffer);
	m_recv_queue_size += byte_buffer->size();

	SLOG_DEBUG("total %d bytes data waiting to recv on socket fd=%d", m_recv_queue_size, m_socket_handle);
	return true;
}

//从待接收队列头部获取一个io_buffer(该io_buffer从队列头移除)
ByteBuffer* TransSocket::pop_recv_buffer()
{
	if(m_recv_queue.empty())
		return NULL;
	ByteBuffer *byte_buffer = m_recv_queue.front();
	m_recv_queue.pop_front();
	m_recv_queue_size -= byte_buffer->size();

	SLOG_DEBUG("total %d bytes data waiting to recv on socket fd=%d", m_recv_queue_size, m_socket_handle);
	return byte_buffer;
}

