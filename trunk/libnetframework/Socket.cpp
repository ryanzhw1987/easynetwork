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

int Socket::assign(SocketHandle socket_handle, int port, const char *ip, BlockMode block_mode)
{
	if(m_socket_handle != SOCKET_INVALID)
		return -1;
	m_socket_handle = socket_handle;
	m_port = port;
	copy_ip(ip);
	m_block_mode = block_mode;
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
int ListenSocket::open()
{
	if(m_socket_handle!=SOCKET_INVALID || m_port<=0)
		return -1;

	//1. ����socket
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
		return -1;

	//2. ��������
	int flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1 )
	{
		SLOG_ERROR("fcntl<get> failed. errno=%d", errno);
		close(fd);
		return -1;
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
		return -1;
	}

	//set reuse
	int reuse = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == -1)
	{
		SLOG_ERROR("set socket SO_REUSEADDR option failed, errno=%d",errno);
		close(fd);
		return -1;
	}

	//3. �󶨵��˿�
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);
	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		SLOG_ERROR("bind failed, errno=%d",errno);
		close(fd);
		return -1;
	}

	//4. �����˿�.���������еȴ�accept���������������ΪĬ��ֵ
	if(listen(fd, 128) == -1)
	{
		SLOG_ERROR("listen failed, errno=%d",errno);
		close(fd);
		return -1;
	}

	m_socket_handle = fd;
	return 0;
}

SocketHandle ListenSocket::accept_connect() 
{
	int fd = -1;
	while(true)
	{
		fd = accept(m_socket_handle, NULL, 0);
		if(fd == -1)
		{
			if(errno==EAGAIN || errno==EINPROGRESS || errno==EINTR)  //���ж�
				continue;
			SLOG_ERROR("accept client socket failed. errno=%d", errno);	
			return SOCKET_INVALID;
		}
		break;
	}
	return fd;
}

//////////////////////////////////////////////////////////////////////////////
int TransSocket::init_active_socket()
{
	//����socket����
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

int TransSocket::connect_server(int timeout_ms/*=2000*/)
{
	if(m_socket_handle!=SOCKET_INVALID || strlen(m_ip)<=0 || m_port <=0)
		return -1;

	//1. ����socket
	m_socket_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_socket_handle < 0)
	{
		SLOG_ERROR("create active socket error");
		m_socket_handle = SOCKET_INVALID;
		return -1;
	}

	//2. ��ʼ��
	if(init_active_socket() == -1)
	{
		SLOG_ERROR("init active socket error. close fd=%d", m_socket_handle);
		close(m_socket_handle);
		m_socket_handle = SOCKET_INVALID;
		return -1;
	}

	//3. ����
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_ip);
	addr.sin_port = htons(m_port);
	if(connect(m_socket_handle, (struct sockaddr*)&addr, sizeof(addr)) == -1)                                              
	{
		if(m_block_mode==NOBLOCK && (errno==EINPROGRESS||errno==EINTR))	//���������ҵȴ���������
		{
			struct timeval tval;
			fd_set rset, wset;
			FD_ZERO(&rset);
			FD_SET(m_socket_handle, &rset);
			wset = rset;

			tval.tv_sec  = timeout_ms/1000;
			tval.tv_usec = (timeout_ms%1000)*1000;

			int tmp = select(m_socket_handle+1,(fd_set*)&rset,(fd_set*)&wset,(fd_set*)NULL, &tval);
			if (tmp <= 0)
			{
				SLOG_ERROR("select failed when connecting server:%s:%d. errno=%d", m_ip, m_port, errno);
				close(m_socket_handle);
				m_socket_handle = SOCKET_INVALID;
				return -1;
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
					return -1;
				}
			}
		}		
		else
		{
			SLOG_ERROR("connect server:%s:%d. errno=%d", m_ip, m_port, errno);
			close(m_socket_handle);
			m_socket_handle = SOCKET_INVALID;
			return -1;
		}
	}

	return 0;
}

//����ֵ:
//����0:�ɹ����ض�ȡ���ֽ���(�����ǲ�������).
//TRANS_CLOSE: ���������ر�
//TRANS_NODATA: û������
//TRANS_ERROR: ʧ��
int TransSocket::recv_data(char *buffer, int len)
{
	assert(len>0);
	assert(buffer!=NULL);

	int ret = 0;
	int read_size = 0; //�Ѷ����ݴ�С
	int need_size = 0; //ʣ��δ�����ݴ�С

	while(read_size<len)
	{
		need_size = len-read_size;
		ret = recv(m_socket_handle, buffer+read_size, need_size, 0);
		if(ret > 0)
			read_size += ret;
		else if(ret == 0) //�Զ˶Ͽ�����
			return TRANS_CLOSE;
		else
		{
			SLOG_TRACE("receive data return -1. errno=%d",errno);
			if(errno==EINTR || errno==EWOULDBLOCK || errno==EAGAIN)
				break;
			return TRANS_ERROR; 	//ʧ��
		}
	}

	if(read_size>0)
	{
		SLOG_TRACE("receive data succ. len=%d", read_size);
		return read_size;
	}
	else
		return TRANS_NODATA;
}

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
			if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)  //�ж�,����
				continue;
			return TRANS_ERROR;
		}
	}

	SLOG_TRACE("send data succ. len=%d", send_size);
	return send_size;
}


//�����������ݵ����뻺����. !!!***�����ڷ�����ģʽ***!!!
//����ֵ:
//TRANS_OK:�ɹ�
//TRANS_NOMEM: û���ڴ�
//TRANS_ERROR: ����
//TRANS_CLOSE: �Զ˹ر�����
//TRANS_BLOCK: ��ǰ������ģʽ
TransStatus TransSocket::recv_buffer()
{
	if(m_block_mode == BLOCK)
		return TRANS_BLOCK;

	unsigned int read_size;
	char * buffer;
	while(true)
	{
		read_size = 1024;
		buffer = m_recv_buffer.write_begin(read_size);
		if(buffer == NULL)
		{
			SLOG_ERROR("get write buffer error.");
			return TRANS_NOMEM;
		}

		read_size = recv(m_socket_handle, buffer, read_size, 0);
		if(read_size < 0)
		{
			if(errno == EINTR)  //���ж�,������
			{
				SLOG_TRACE("recv data interupt,continue to read");
				continue;
			}
			else if(errno==EWOULDBLOCK || errno==EAGAIN)  //��������
			{
				SLOG_TRACE("recv data. no data now");
				break;
			}
			else  //����
			{
				SLOG_ERROR("recv data error. errno=%d",errno);
				return TRANS_ERROR;
			}
		}
		else if(read_size == 0) //�Զ˶Ͽ�����
		{
			SLOG_ERROR("client closed the connect gracefully.fd=%d", m_socket_handle);
			return TRANS_CLOSE;
		}

		m_recv_buffer.write_end(read_size);
		if(read_size < 1024) //�Ѿ���������
			break;
	}

	return TRANS_OK;
}

//���Է�������������е���������,ֱ��������ɻ��߷��Ͳ���ȥ.
//����ֵ:
//TRANS_OK:�ɹ�
//TRANS_PENDING: ֻ���Ͳ�������,�������������ݴ�����
//TRANS_ERROR: ����
TransStatus TransSocket::send_buffer()
{
	unsigned int size;
	char *buffer = m_send_buffer.read_begin(&size);
	if(buffer == NULL)
		return TRANS_OK;
	while(true)
	{
		unsigned int ret = send(m_socket_handle, buffer, size, 0);
		if(ret > 0)
		{
			m_send_buffer.read_end(ret);
			if(ret < size)
				return TRANS_PENDING;  //ֻ������һ��������
			else
				return TRANS_OK;		//ȫ������
		}
		else if(errno == EINTR) //�ж����� || errno == EWOULDBLOCK || errno == EAGAIN)  //�ж�,����
		{
			SLOG_TRACE("send buffer data interrupted,retry to send.");
			continue;
		}
		else if(errno == EWOULDBLOCK || errno == EAGAIN) //��ʱ���Ͳ���ȥ
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
}

