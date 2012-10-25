/*
 * NetInterface.cpp
 *
 *  Created on: 2012-9-7
 *      Author: LiuYongjin
 */

#include "NetInterface.h"
#include "slog.h"
#include <assert.h>

using std::pair;
using std::make_pair;

NetInterface::NetInterface()
				:m_io_demuxer(NULL)
				,m_socket_manager(NULL)
				,m_protocol_family(NULL)
				,m_socket_idle_timeout_ms(12000)
{}

bool NetInterface::start_instance()
{
	m_io_demuxer = create_io_demuxer();
	m_socket_manager = create_socket_manager();
	m_protocol_family = create_protocol_family();

	on_start_instance();
	return true;
}

bool NetInterface::stop_instance()
{
	delete_io_demuxer(m_io_demuxer);
	m_io_demuxer = NULL;
	delete_socket_manager(m_socket_manager);
	m_socket_manager = NULL;
	delete_protocol_family(m_protocol_family);
	m_protocol_family = NULL;

	on_stop_instance();
	return true;
}

NetInterface::~NetInterface()
{
	ProtocolMap::iterator it;
	for(it=m_protocol_map.begin(); it!=m_protocol_map.end(); ++it)
	{
		queue<Protocol*> &protocol_queue = it->second;
		queue <Protocol*>::size_type size =  protocol_queue.size();
		while(size>0)
		{
			Protocol *protocol = (Protocol *)protocol_queue.front();
			delete protocol;
			protocol_queue.pop();
			size = protocol_queue.size();
		}
	}
	m_protocol_map.clear();
}

//实现ConnectAccepter:接收一个新的连接请求
bool NetInterface::accept(SocketHandle trans_fd)
{
	SLOG_DEBUG("accept trans fd=%d", trans_fd);
	if(m_io_demuxer->register_event(trans_fd, EVENT_READ|EVENT_PERSIST, 0, this) == -1)
	{
		SLOG_ERROR("register trans socket event failed.fd=%d", trans_fd);
		return false;
	}

	Socket* socket = m_socket_manager->add_passive_trans_socket(trans_fd);
	if(socket == NULL)
	{
		SLOG_ERROR("socket manager add passive socket failed");
		m_io_demuxer->unregister_event(trans_fd);
		return false;
	}

	SLOG_TRACE("net interface accept new connect for ip:%s, fd=%d", socket->get_ip(), trans_fd);
	on_socket_handler_accpet(trans_fd);
	return true;
}

//要求fd是非阻塞的
HANDLE_RESULT NetInterface::on_readable(int fd)
{
	SLOG_TRACE("socket on_readable. fd=%d", fd);

	TransSocket* trans_socket = (TransSocket*)m_socket_manager->find_trans_socket(fd);
	if(trans_socket == NULL)
	{
		SLOG_ERROR("can't find trans socket in socket manager. fd=%d", fd);
		return HANDLE_ERROR;
	}

	ByteBuffer *raw_data_buffer = NULL;

	unsigned int header_length = 0;
	ProtocolHeader *header = m_protocol_family->create_protocol_header();
	assert(header != NULL);
	header_length = header->get_header_length();
	assert(header_length > 0);

	//1. 检查是否有未处理数据
	raw_data_buffer = trans_socket->pop_recv_buffer();
	if(raw_data_buffer == NULL) //新的协议包
		raw_data_buffer = new ByteBuffer();

	unsigned int raw_data_size = raw_data_buffer->size();
	if(raw_data_size < header_length)	//读协议头数据
	{
		int need_size = header_length-raw_data_size;
		int recv_size = trans_socket->recv_buffer(raw_data_buffer, need_size, false);
		if(recv_size == TRANS_ERROR)
		{
			SLOG_ERROR("receive protocol header error. fd=%d", fd);
			delete raw_data_buffer;
			m_protocol_family->destroy_protocol_header(header);
			return HANDLE_ERROR;
		}
		else if(recv_size < need_size) //还有部分协议头数据未接收
		{
			SLOG_DEBUG("protocol header incompleted[need=%d,recv=%d]. waiting for the remaining data.", need_size, recv_size);
			trans_socket->push_recv_buffer(raw_data_buffer);
			m_protocol_family->destroy_protocol_header(header);
			return HANDLE_OK;
		}
		raw_data_size += recv_size;
	}
	//2. 解码协议头
	int body_length = 0;
	char *header_buffer = raw_data_buffer->get_data(0, header_length);
	assert(header_buffer != NULL);
	if(header->decode(header_buffer, body_length) == false)
	{
		SLOG_ERROR("decode protocol header error. fd=%d", fd);
		delete raw_data_buffer;
		m_protocol_family->destroy_protocol_header(header);
		return HANDLE_ERROR;
	}

	//3. 接收协议体数据
	if(body_length > 0) //允许空协议体
	{
		int need_size = body_length+header_length-raw_data_size;
		int recv_size = trans_socket->recv_buffer(raw_data_buffer, need_size, false);
		if(recv_size == TRANS_ERROR)
		{
			SLOG_ERROR("receive protocol body error. fd=%d", fd);
			delete raw_data_buffer;
			m_protocol_family->destroy_protocol_header(header);
			return HANDLE_ERROR;
		}
		else if(recv_size < need_size) //还有部分协议体数据未接收
		{
			SLOG_DEBUG("protocol body incompleted[need=%d,recv=%d]. waiting for the remaining data.", need_size, recv_size);
			trans_socket->push_recv_buffer(raw_data_buffer);
			m_protocol_family->destroy_protocol_header(header);
			return HANDLE_OK;
		}
	}

	//4. 解码协议体
	Protocol *protocol = m_protocol_family->create_protocol_by_header(header);
	if(protocol == NULL)
	{
		SLOG_ERROR("create protocol error. fd=%d", fd);
		delete raw_data_buffer;
		m_protocol_family->destroy_protocol_header(header);
		return HANDLE_ERROR;
	}
	protocol->set_protocol_family(m_protocol_family);
	/*****由protocol托管header到释放******/
	protocol->attach_protocol_header(header);

	char *body_buffer = NULL;
	if(body_length > 0)
		body_buffer = raw_data_buffer->get_data(header_length, body_length);
	if(protocol->decode_body(body_buffer, body_length) == false)
	{
		SLOG_ERROR("decode protocol body error. fd=%d", fd);
		delete raw_data_buffer;
		m_protocol_family->destroy_protocol(protocol);
		return HANDLE_ERROR;
	}
	/***** 由protocol托管raw_data的释放 ******/
	protocol->attach_raw_data(raw_data_buffer);

	//6. 调用回调函数向应用层发协议
	bool detach_protocol = false;
	int result = on_recv_protocol(fd, protocol, detach_protocol);
	if(result==-1 || detach_protocol==false)
		m_protocol_family->destroy_protocol(protocol);
	return HANDLE_OK;
}

HANDLE_RESULT NetInterface::on_writeable(int fd)
{
	SLOG_TRACE("socket on_writeabble. fd=%d", fd);

	TransSocket* trans_socket = (TransSocket*)m_socket_manager->find_trans_socket(fd);
	if(trans_socket == NULL)
	{
		SLOG_ERROR("can't find trans socket in socket manager. fd=%d", fd);
		return HANDLE_ERROR;
	}
	int ret = trans_socket->send_buffer();
	if(ret == TRANS_ERROR)
		return HANDLE_ERROR;
	else if(ret > 0)
	{
		SLOG_INFO("remain %d bytes data wait for sending on socket fd=%d",ret, fd);
		if(m_io_demuxer->register_event(fd, EVENT_WRITE, m_socket_idle_timeout_ms, this) != 0)
		{
			SLOG_ERROR("register write event error. fd=%d", fd);
			return HANDLE_ERROR;
		}
		return HANDLE_OK;
	}
	//移出一个待发送的协议
	Protocol* protocol = get_wait_to_send_protocol(fd);
	if(protocol != NULL)
	{
		if(protocol->encode() == false)
		{
			SLOG_ERROR("protocol encode error. fd=%d", fd);
			return HANDLE_ERROR;
		}
		ByteBuffer *raw_data = protocol->detach_raw_data();	//脱离raw_data
		trans_socket->push_send_buffer(raw_data);
		ret = trans_socket->send_buffer();
		if(ret == TRANS_ERROR)
			return HANDLE_ERROR;
		else if(ret > 0)
		{
			SLOG_INFO("send %s. remain %d bytes data wait for sending on socket fd=%d", protocol->details(), ret, fd);
			if(m_io_demuxer->register_event(fd, EVENT_WRITE, m_socket_idle_timeout_ms, this) != 0)
			{
				SLOG_ERROR("register write event error. fd=%d", fd);
				return HANDLE_ERROR;
			}
			return HANDLE_OK;
		}
		on_protocol_send_succ(fd, protocol);

		if(get_wait_to_send_protocol_number(fd) > 0)
		{
			if(m_io_demuxer->register_event(fd, EVENT_WRITE, m_socket_idle_timeout_ms, this) != 0)
			{
				SLOG_ERROR("register write event error. fd=%d", fd);
				return HANDLE_ERROR;
			}
		}
	}
	return HANDLE_OK;
}

HANDLE_RESULT NetInterface::on_timeout(int fd)
{
	SLOG_TRACE("socket on_timeout. fd=%d", fd);
	//取消所有fd上面的带发送协议
	cancal_wait_to_send_protocol(fd);
	m_socket_manager->remove_trans_socket(fd);       //从socket manager中删除掉

	on_socket_handle_timeout(fd);  //通知应用层socket超时
	return HANDLE_OK;
}

HANDLE_RESULT NetInterface::on_error(int fd)
{
	SLOG_TRACE("socket on_error. fd=%d error", fd);
	//取消所有fd上面的带发送协议
	cancal_wait_to_send_protocol(fd);
	m_socket_manager->remove_trans_socket(fd);     //从socket manager中删除掉

	on_socket_handle_error(fd);  //通知应用层socket错误
	return HANDLE_OK;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
SocketHandle NetInterface::get_active_trans_socket(const char *ip, int port)
{
	Socket* socket = m_socket_manager->add_active_trans_socket(ip, port);
	if(socket == NULL)
		return SOCKET_INVALID;

	SocketHandle socket_handle = socket->get_handle();
	if(m_io_demuxer->register_event(socket_handle, EVENT_READ|EVENT_PERSIST, m_socket_idle_timeout_ms, this) != 0)
	{
		SLOG_ERROR("register active socket error. delete it. fd=%d", socket_handle);
		m_socket_manager->remove_trans_socket(socket_handle);
		socket_handle = SOCKET_INVALID;
	}

	return socket_handle;
}

//释放链接
bool NetInterface::release_trans_socket(SocketHandle socket_handle)
{
	m_socket_manager->remove_trans_socket(socket_handle);
	return true;
}

//添加协议到发送队列.成功返回0.失败返回-1,需要自行处理protocol.
int NetInterface::send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp/*=false*/)
{
	if(socket_handle==SOCKET_INVALID || protocol==NULL)
		return -1;

	//检查对应的socket是否存在
	Socket* trans_socket = m_socket_manager->find_trans_socket(socket_handle);
	if(trans_socket == NULL)
	{
		SLOG_WARN("can't find socket of fd:%d", socket_handle);
		return -1;
	}

	//添加到fd对应任务队列
	ProtocolMap::iterator it = m_protocol_map.find(socket_handle);
	if(it == m_protocol_map.end())
	{
		queue<Protocol*> pro_queue;
		pair<ProtocolMap::iterator, bool> ret_pair = m_protocol_map.insert(make_pair(socket_handle, pro_queue));
		if(ret_pair.second == false)
		{
			SLOG_ERROR("insert protocol queue to map failed. fd:%d", socket_handle);
			return -1;
		}
		it = ret_pair.first;
	}
	//添加到队列
	queue<Protocol*> *pro_queue = &it->second;
	pro_queue->push(protocol);

	//注册等待可写事件
	int events = EVENT_WRITE;
	//if(has_resp)  //如果有回复,注册(一次)可读事件
	//	events |= EVENT_READ;

	return m_io_demuxer->register_event(socket_handle, events, 12000, this);
}

//获取等待队列中待发送的协议
Protocol* NetInterface::get_wait_to_send_protocol(SocketHandle socket_handle)
{
	ProtocolMap::iterator it = m_protocol_map.find(socket_handle);
	if(it == m_protocol_map.end())
		return NULL;

	queue<Protocol*> &protocol_queue = it->second;
	if(protocol_queue.empty())
	{
		SLOG_DEBUG("protocol queue of fd:% is empty.remove from map", socket_handle);
		m_protocol_map.erase(it);
		return NULL;
	}

	Protocol* protocol = protocol_queue.front();
	protocol_queue.pop();

	return protocol;
}

//获取等待队列中待发送的协议个数
int NetInterface::get_wait_to_send_protocol_number(SocketHandle socket_handle)
{
	ProtocolMap::iterator it = m_protocol_map.find(socket_handle);
	if(it == m_protocol_map.end())
		return 0;
	queue<Protocol*> &protocol_queue = it->second;
	if(protocol_queue.empty())
	{
		SLOG_TRACE("protocol queue of fd:%d is empty.remove from map", socket_handle);
		m_protocol_map.erase(it);
		return 0;
	}
	return (int)protocol_queue.size();
}

//取消所有待发送协议,同时调用on_protocol_send_error通知应用层
int NetInterface::cancal_wait_to_send_protocol(SocketHandle socket_handle)
{
	SLOG_TRACE("cancal protocols waitting to send.");
	ProtocolMap::iterator it = m_protocol_map.find(socket_handle);
	if(it != m_protocol_map.end())
	{
		Protocol* protocol = NULL;
		queue<Protocol*> &pro_queue = it->second;
		while(!pro_queue.empty())
		{
			protocol = pro_queue.front();
			if(protocol == NULL)
				SLOG_WARN("get NULL protocol from queue. fd=%d", socket_handle);
			else
				on_protocol_send_error(socket_handle, protocol);
			pro_queue.pop();
		}
	}

	return 0;
}

