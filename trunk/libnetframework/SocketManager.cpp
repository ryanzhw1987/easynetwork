#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "SocketManager.h"
#include "slog.h"

using std::pair;
using std::make_pair;

class ListenHandler:public EventHandler
{
public:
	ListenHandler(SocketManager *socket_mgr)
	{
		assert(socket_mgr != NULL);
		m_socket_mgr = socket_mgr;
	}

	HANDLE_RESULT on_readable(int fd);
	//HANDLE_RESULT on_writeabble(int fd){return HANDLE_OK;}	//to do deal with timeout
	//HANDLE_RESULT on_timeout(int fd){return HANDLE_OK;} 		//to do deal with timeout
	//HANDLE_RESULT on_error(int fd){return HANDLE_OK;} 		//to do deal with error
private:
	SocketManager *m_socket_mgr;
};

class TransHandler:public EventHandler
{
public:
	TransHandler(SocketManager *socket_mgr)
	{
		assert(socket_mgr != NULL);
		m_socket_mgr = socket_mgr;
	}

	HANDLE_RESULT on_readable(int fd);
	HANDLE_RESULT on_writeabble(int fd);
	HANDLE_RESULT on_timeout(int fd);  //to do deal with timeout
	HANDLE_RESULT on_error(int fd); //to do deal with error
private:
	SocketManager *m_socket_mgr;
};

///////////////////////////////////////////
///////////////////////////////////////////
//////                               //////
//////         SocketManager         //////
//////                               //////
///////////////////////////////////////////
///////////////////////////////////////////
SocketManager::SocketManager(BlockMode block_mode/*=NOBLOCK*/)
{	
	m_listen_socket = NULL;
	m_trans_handler = new_trans_handler();
	m_listen_handler = new_listen_handler();

	m_block_mode = block_mode;
}

EventHandler* SocketManager::new_trans_handler()
{
	return (EventHandler*)new TransHandler(this);
}

EventHandler* SocketManager::new_listen_handler()
{
	return (EventHandler*)new ListenHandler(this);
}

SocketManager::~SocketManager()
{	
	if(m_trans_handler != NULL)
		delete m_trans_handler;
	m_trans_handler = NULL;

	if(m_listen_handler != NULL)
		delete m_listen_handler;
	m_listen_handler = NULL;

	if(m_listen_socket != NULL)
		delete m_listen_socket;
	m_listen_socket = NULL;

	{//delete sockets
		SocketMap::iterator it;
		for(it=m_trans_sockets_map.begin();it!=m_trans_sockets_map.end(); ++it)
		{
			Socket* trans_socket = it->second;
			delete trans_socket;
		}
		m_trans_sockets_map.clear();
	}

	{//delete protocol
		SendTaskMap::iterator it;
		for(it=m_send_tasks_map.begin(); it!=m_send_tasks_map.end(); ++it)
		{
			queue<Protocol*> &protocol_queue = it->second;
			queue <Protocol*>::size_type size =  protocol_queue.size();
			while(size>0)
			{
				Protocol *protocol = (Protocol *)protocol_queue.front();
				delete protocol;
				protocol_queue.pop();
				size =  protocol_queue.size();
			}
		}
		m_send_tasks_map.clear();
	}
}

int SocketManager::listen(int port)
{
	if(m_listen_socket == NULL)
	{
		m_listen_socket = (ListenSocket*)new_listen_socket();
		assert(m_listen_socket!=NULL && m_listen_socket->get_handle()==SOCKET_INVALID);
		m_listen_socket->assign(SOCKET_INVALID, port, NULL, m_block_mode);
		if(m_listen_socket->open() == -1)
		{
			SLOG_ERROR("open listen socket failed. port=%d", port);
			delete m_listen_socket;
			m_listen_socket = NULL;
			return -1;
		}

		SocketHandle socket_handle = m_listen_socket->get_handle();assert(socket_handle!=SOCKET_INVALID);
		IODemuxer *io_demuxer = get_io_demuxer();assert(io_demuxer!=NULL);		
		if(io_demuxer->register_event(socket_handle, EVENT_READ|EVENT_PERSIST, -1, m_listen_handler) == -1)
		{
			SLOG_ERROR("register listen socket failed. delete it");
			delete m_listen_socket;
			m_listen_socket = NULL;
			return -1;
		}

		SLOG_TRACE("listen on port=%d succ. fd=%d", port, socket_handle);
		return 0;
	}
	else if(m_listen_socket->get_port() == port)
	{
		SLOG_WARN("already listen on port=%d", port);
		return 0;
	}
	else
	{
		SLOG_ERROR("try to listen on port=%d, but already listen on port=%d", port, m_listen_socket->get_port());
		return -1;
	}
}

SocketHandle SocketManager::create_active_trans_socket(const char *ip, int port)
{
	TransSocket* active_socket = (TransSocket*)new_trans_socket();
	assert(active_socket!=NULL);
	active_socket->assign(SOCKET_INVALID, port, ip, m_block_mode);
	if(active_socket->connect_server() == -1)
	{
		SLOG_ERROR("active connect failed.");
		delete active_socket;
		return SOCKET_INVALID;
	}

	SocketHandle socket_handle = active_socket->get_handle();
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it != m_trans_sockets_map.end())
	{
		SLOG_ERROR("active fd already exist in socket manager. delete it. fd=%d", socket_handle);
		delete (TransSocket*)it->second;
		m_trans_sockets_map.erase(it);

		delete active_socket;
		return SOCKET_INVALID;
	}
	else
	{
		pair<SocketMap::iterator, bool> pair_ret = m_trans_sockets_map.insert(make_pair(socket_handle, active_socket));
		if(pair_ret.second == false)
		{
			SLOG_ERROR("insert active socket error. delete it");
			delete active_socket;
			return SOCKET_INVALID;
		}

		IODemuxer *io_demuxer = get_io_demuxer();assert(io_demuxer!=NULL);
		if(io_demuxer->register_event(socket_handle, EVENT_READ|EVENT_PERSIST, 12000, m_trans_handler)==-1)
		{
			SLOG_ERROR("register active socket error. delete it. fd=%d", socket_handle);
			m_trans_sockets_map.erase(socket_handle);
			delete active_socket;
			return SOCKET_INVALID;
		}
	}

	return socket_handle;
}

int SocketManager::init_passive_trans_socket(SocketHandle socket_handle, BlockMode block_mode)
{
	int flags = fcntl(socket_handle, F_GETFL, 0);
	if(flags == -1)
	{
		SLOG_ERROR("fcntl<get> passive_socket faile. errno=%d.", errno);
		return -1;
	}

	if(block_mode == NOBLOCK) //non block mode
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(socket_handle, F_SETFL, flags) == -1 )
	{
		SLOG_ERROR("fcntl<set> passive_socket faile. errno=%d.", errno);
		return -1;
	}

	return 0;
}

int SocketManager::add_passive_trans_socket(const char *peer_ip, SocketHandle socket_handle)
{
	if(socket_handle==SOCKET_INVALID || init_passive_trans_socket(socket_handle, m_block_mode) == -1)
		return -1;

	TransSocket *passive_socket = NULL;
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it == m_trans_sockets_map.end())
	{
		passive_socket = (TransSocket*)new_trans_socket();
		assert(passive_socket!=NULL && passive_socket->get_handle()==SOCKET_INVALID);
		//先保存/注册再赋值, 防止socket被关闭掉
		pair<SocketMap::iterator, bool> pair_ret = m_trans_sockets_map.insert(make_pair(socket_handle, passive_socket));
		if(pair_ret.second == false)
		{
			SLOG_ERROR("passive socket insert into map failed");
			delete passive_socket;
			return -1;
		}

		IODemuxer *io_demuxer = get_io_demuxer();assert(io_demuxer!=NULL);
		if(io_demuxer->register_event(socket_handle, EVENT_READ|EVENT_PERSIST, 0, m_trans_handler) == -1)
		{
			SLOG_ERROR("register trans socket failed. fd=%d", socket_handle);
			m_trans_sockets_map.erase(pair_ret.first);
			delete passive_socket;
			return -1;
		}
		//此时再赋值, 防止socket被关闭掉
		passive_socket->assign(socket_handle, -1, peer_ip, m_block_mode);
		on_socket_handler_accpet(socket_handle);
	}
	else
		SLOG_WARN("passive trans socket already exist in socket manager. fd=%d", socket_handle);
	
	return 0;
}

int SocketManager::delete_trans_socket(SocketHandle socket_handle)
{	
	SLOG_DEBUG("remove trans socket from socket manager. fd=%d", socket_handle);

	//取消所有fd上面的带发送协议
	cancal_wait_to_send_protocol(socket_handle);

	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it != m_trans_sockets_map.end())
	{
		TransSocket *trans_socket = (TransSocket*)it->second;
		m_trans_sockets_map.erase(it);
		delete trans_socket;
	}

	return 0;
}

Socket* SocketManager::new_listen_socket()
{
	return (Socket*)new ListenSocket();
}

Socket* SocketManager::new_trans_socket()
{
	return (Socket*)new TransSocket();
}

Socket* SocketManager::find_listen_socket(SocketHandle socket_handle)
{
	if(m_listen_socket==NULL || m_listen_socket->get_handle()!=socket_handle)
		return NULL;
	else
		return m_listen_socket;
}

Socket* SocketManager::find_trans_socket(SocketHandle socket_handle)
{
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it == m_trans_sockets_map.end())
		return NULL;
	else
		return it->second;
}


////////////////////////////////////////////////  协议发送  ////////////////////////////////////////////
//发送协议.成功返回0,protocol放入等待发送队列; 失败返回-1,需要自行处理protocol.
int SocketManager::send_protocol(SocketHandle socket_handle, Protocol *protocol, bool has_resp)
{
	if(socket_handle==SOCKET_INVALID || protocol==NULL)
		return -1;

	//检查对应的socket是否存在
	Socket* trans_socket = find_trans_socket(socket_handle);
	if(trans_socket == NULL)
	{
		SLOG_WARN("can't find socket of fd:%d", socket_handle);
		return -1;
	}

	//添加到fd对应任务队列
	SendTaskMap::iterator it = m_send_tasks_map.find(socket_handle);
	if(it == m_send_tasks_map.end())
	{
		queue<Protocol*> pro_queue;
		pair<SendTaskMap::iterator, bool> ret_pair = m_send_tasks_map.insert(make_pair(socket_handle, pro_queue));
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

	IODemuxer *io_demuxer = get_io_demuxer();assert(io_demuxer!=NULL);
	return io_demuxer->register_event(socket_handle, events, 12000, m_trans_handler);	
}

Protocol* SocketManager::get_wait_to_send_protocol(SocketHandle socket_handle)
{
	SendTaskMap::iterator it =  m_send_tasks_map.find(socket_handle);
	if(it == m_send_tasks_map.end())
		return NULL;

	queue<Protocol*> &protocol_queue = it->second;
	if(protocol_queue.empty())
	{
		SLOG_DEBUG("protocol queue of fd:% is empty.remove from map", socket_handle);
		m_send_tasks_map.erase(it);
		return NULL;
	}

	Protocol* protocol = protocol_queue.front();
	protocol_queue.pop();

	return protocol;
}

int SocketManager::get_wait_to_send_protocol_number(SocketHandle socket_handle)
{
	SendTaskMap::iterator it =  m_send_tasks_map.find(socket_handle);
	if(it == m_send_tasks_map.end())
		return 0;
	queue<Protocol*> &protocol_queue = it->second;
	if(protocol_queue.empty())
	{
		SLOG_DEBUG("protocol queue of fd:%d is empty.remove from map", socket_handle);
		m_send_tasks_map.erase(it);
		return 0;
	}
	return (int)protocol_queue.size();
}

int SocketManager::cancal_wait_to_send_protocol(SocketHandle socket_handle)
{
	SLOG_DEBUG("cancal protocols waitting to send.");
	SendTaskMap::iterator it = m_send_tasks_map.find(socket_handle);
	if(it != m_send_tasks_map.end())
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


//////////////////////////  应用层提供的响应函数  ////////////////////////////
/*
int SocketManager::on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, int *has_delete)
{
    	SLOG_DEBUG("Default socket manager receive protocol. fd=%d, header size=%d, body size=%d.", socket_handle, protocol->get_header_size(), protocol->get_body_size());
    	return 0;
}

int SocketManager::on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_ERROR("send protocol error. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int SocketManager::on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)
{
	SLOG_DEBUG("send protocol succ. fd=%d, protocol=%x", socket_handle, protocol);
	delete protocol;
	return 0;
}

int SocketManager::on_socket_handle_error(SocketHandle socket_handle)
{
	SLOG_DEBUG("socket handler error. fd=%d", socket_handle);
	return 0;
}

int SocketManager::on_socket_handle_timeout(SocketHandle socket_handle)
{
	SLOG_DEBUG("socket handler error. fd=%d", socket_handle);
	return 0;
}
*/

///////////////////////////////////////////
///////////////////////////////////////////
//////                               //////
//////         ListenHandler         //////
//////                               //////
///////////////////////////////////////////
///////////////////////////////////////////

HANDLE_RESULT ListenHandler::on_readable(int fd)
{
	ListenSocket *listen_socket = (ListenSocket *)m_socket_mgr->find_listen_socket(fd);
	if(listen_socket == NULL)
	{
		SLOG_ERROR("can't find listen socket in socket manager.");
		return HANDLE_ERROR;
	}

	//接收连接
	SocketHandle socket_handle = listen_socket->accept_connect();
	if(socket_handle == SOCKET_INVALID)
	{
		SLOG_WARN("accept connect error in listen event.");
		return HANDLE_OK;
	}

	//获取对端地址
	const char *ip = "Unknow ip!!!";
	struct sockaddr_in peer_addr;
	int socket_len = sizeof(peer_addr);
	if(getpeername(socket_handle, (struct sockaddr*)&peer_addr, (socklen_t*)&socket_len) == 0)
		ip = inet_ntoa(peer_addr.sin_addr);
	SLOG_DEBUG("accept connect from ip:%s. fd=%d", ip, socket_handle);

	//添加到socket管理器
	if(m_socket_mgr->add_passive_trans_socket(ip, socket_handle) == -1)
	{
		SLOG_ERROR("add passive trans socket to socket manager failed. close it. fd=%d", socket_handle);
		close(socket_handle);
	}

	return HANDLE_OK;
}


///////////////////////////////////////////
///////////////////////////////////////////
//////                               //////
//////         TransHandler          //////
//////                               //////
///////////////////////////////////////////
///////////////////////////////////////////
HANDLE_RESULT TransHandler::on_readable(int fd)
{
	SLOG_TRACE("socket on_readable. fd=%d", fd);
	
	TransSocket* trans_socket = (TransSocket*)m_socket_mgr->find_trans_socket(fd);
	if(trans_socket == NULL)
	{
		SLOG_ERROR("can't find trans socket in socket manager. fd=%d", fd);
		return HANDLE_ERROR;
	}

	//读取所有数据
	TransStatus trans_status = trans_socket->recv_buffer();
	if(trans_status != TRANS_OK)
	{
		SLOG_ERROR("socket recv all error. fd=%d", fd);
		return HANDLE_ERROR;
	}
	IOBuffer *recv_buffer = trans_socket->get_recv_buffer(); //获取socket的recv buffer

	int body_size = 0;
	int header_size = 0;
	Protocol* protocol = NULL;
	ProtocolFamily *protocol_family = m_socket_mgr->get_protocol_family();
	while(true)
	{
		unsigned int size = 0;
		char *buffer = recv_buffer->read_begin(&size);
		if(buffer == NULL)  //无数据可读
			break;
		
		if(header_size == 0) //创建protocol,获取头部大小(头部大小固定,只获取一次;只提前生成一次protocol)
		{
			protocol = protocol_family->create_protocol();
			header_size = protocol->get_header_size();
		}

		if(header_size > size) //头部数据不够
		{
			if(protocol != NULL)  //第一次生成的protocol
				delete protocol;
			break;
		}

 		//第一次获取头部大小时提前生成了protocol,之后当有足够的头部数据时才会生成
		if(protocol == NULL)
			protocol = protocol_family->create_protocol();

		//1. decode header
		if(protocol->decode_header(buffer, header_size) != 0)
		{
			SLOG_ERROR("decode header error.");
			delete protocol;
			return HANDLE_ERROR;
		}

		//2. decode body
		body_size = protocol->get_body_size();
		if(header_size + body_size > size) //数据不够
		{
			SLOG_DEBUG("no enougth data now, return and wait for more data.");
			delete protocol;
			return HANDLE_OK;
		}
		if(protocol->decode_body(buffer+header_size, body_size) != 0)
		{
			SLOG_ERROR("decode body error.");
			delete protocol;
			return HANDLE_ERROR;
		}
		recv_buffer->read_end(header_size+body_size);  //清空已经读取的数据

		//3. 调用回调函数向应用层发协议
		int has_delete = 0;
		int ret = m_socket_mgr->on_recv_protocol(fd, protocol, &has_delete);
		if(ret!=0 || has_delete == 0) //应用层处理失败或者未是否protocol
			delete protocol;
		protocol = NULL;
	}

	return HANDLE_OK;
}

HANDLE_RESULT TransHandler::on_writeabble(int fd)
{
	SLOG_TRACE("socket on_writeabble. fd=%d", fd);

	TransSocket* trans_socket = (TransSocket*)m_socket_mgr->find_trans_socket(fd);
	if(trans_socket == NULL)
		return HANDLE_ERROR;

	//移出一个待发送的协议
	Protocol* protocol = m_socket_mgr->get_wait_to_send_protocol(fd);
	if(protocol != NULL)
	{
		if(protocol->encode(trans_socket->get_send_buffer()) != 0)
		{
			SLOG_ERROR("encode protocol error.");
			m_socket_mgr->on_protocol_send_error(fd, protocol); //通知应用层发送协议失败, 由应用层处理protocol. 不用返回Handle_ERROR, 因为是数据的问题而不是连接问题
		}
	}

	//发送缓冲区数据
	TransStatus trans_status = trans_socket->send_buffer();
	if(protocol != NULL)
	{
		if(trans_status == TRANS_OK)
			m_socket_mgr->on_protocol_send_succ(fd, protocol);
		else if(trans_status == TRANS_PENDING)
		{
			SLOG_WARN("copy protocol data to send buffer and send pending.");
			m_socket_mgr->on_protocol_send_succ(fd, protocol);
		}
		else
		{
			SLOG_ERROR("send protocol error");
			m_socket_mgr->on_protocol_send_error(fd, protocol);
		}
	}

	if(trans_status == TRANS_ERROR)
		return HANDLE_ERROR;

	//发送剩下的协议(如果有的话,注册可写事件)
	if(trans_status==TRANS_PENDING || m_socket_mgr->get_wait_to_send_protocol_number(fd)>0)
	{
		IODemuxer *io_demuxer = m_socket_mgr->get_io_demuxer();assert(io_demuxer!=NULL);
		if(io_demuxer->register_event(fd, EVENT_WRITE, -1, this) != 0)
		{
			SLOG_ERROR("register write event error");
			return HANDLE_ERROR;
		}
	}

	return HANDLE_OK;
/*	
	//移出一个待发送的协议
	Protocol* protocol = m_socket_mgr->get_wait_to_send_protocol(fd);
	if(protocol != NULL)
	{
		//1. 编码
		EncodeBuffer encode_buf;
		if(protocol->encode(&encode_buf) != 0)
		{
			SLOG_ERROR("encode protocol error.");
			m_socket_mgr->on_protocol_send_error(fd, protocol); //通知应用层发送协议失败, 由应用层处理protocol. 不用返回Handle_ERROR, 因为是数据的问题而不是连接问题
		}
		else  //2. 发送数据
		{
			int ret = trans_socket->send_data(encode_buf.get_buffer(), encode_buf.get_size());
			if(ret != encode_buf.get_size())
			{
				m_socket_mgr->on_protocol_send_error(fd, protocol);
				return HANDLE_ERROR; //连接问题, 所以返回HANDLE_ERROR
			}
			m_socket_mgr->on_protocol_send_succ(fd, protocol);  //通知应用层发送成功, 由应用层处理protocol
		}
	}

	//发送剩下的协议(如果有的话,注册可写事件)
	if(m_socket_mgr->get_wait_to_send_protocol_number(fd) > 0)
	{
		IODemuxer *io_demuxer = m_socket_mgr->get_io_demuxer();assert(io_demuxer!=NULL);
		io_demuxer->register_event(fd, EVENT_WRITE, -1, this);
		//注册失败了要怎么处理呢:
		//socket超时调用time_out来处理?或者等待下次发送协议时重新注册?
	}

	return HANDLE_OK;
*/
}

HANDLE_RESULT TransHandler::on_timeout(int fd)
{
	SLOG_DEBUG("socket on_timeout. fd=%d", fd);
	m_socket_mgr->on_socket_handle_timeout(fd);  //通知应用层socket超时
	m_socket_mgr->delete_trans_socket(fd);       //从socket manager中删除掉

	return HANDLE_OK;
}

HANDLE_RESULT TransHandler::on_error(int fd)
{
	SLOG_DEBUG("socket on_error. fd=%d error", fd);	
	m_socket_mgr->on_socket_handle_error(fd);  //通知应用层socket错误
	m_socket_mgr->delete_trans_socket(fd);     //从socket manager中删除掉

	return HANDLE_OK;
}

