#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "SocketManager.h"
#include "slog.h"

using std::pair;
using std::make_pair;

///////////////////////////////////////////
///////////////////////////////////////////
//////                               //////
//////         SocketManager         //////
//////                               //////
///////////////////////////////////////////
///////////////////////////////////////////
SocketManager::~SocketManager()
{
	SocketMap::iterator it;
	for(it=m_trans_sockets_map.begin();it!=m_trans_sockets_map.end(); ++it)
	{
		TransSocket* trans_socket = (TransSocket*)it->second;
		m_trans_socket_memcache.Free(trans_socket);
	}
	m_trans_sockets_map.clear();
}

Socket* SocketManager::new_trans_socket()
{
	//return (Socket*)new TransSocket();
	Socket* socket = (Socket*)m_trans_socket_memcache.Alloc();
	SLOG_DEBUG("create TransSocket[%x] from trans_socket_memcache", socket);
	return socket;
}

void SocketManager::delete_trans_socket(Socket *socket)
{
	SLOG_DEBUG("free TransSocket[%x] to trans_socket_memcache", socket);
	TransSocket* trans_socket = (TransSocket*)socket;
	m_trans_socket_memcache.Free(trans_socket);
}

Socket* SocketManager::add_active_trans_socket(const char *ip, int port)
{
	TransSocket* active_socket = (TransSocket*)new_trans_socket();
	assert(active_socket!=NULL);
	active_socket->assign(SOCKET_INVALID, port, ip, m_block_mode);
	if(!active_socket->open())
	{
		SLOG_ERROR("active connect failed.");
		delete_trans_socket(active_socket);
		return NULL;
	}

	SocketHandle socket_handle = active_socket->get_handle();
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it != m_trans_sockets_map.end())
	{
		SLOG_ERROR("active fd already exist in socket manager. delete it. fd=%d", socket_handle);
		delete_trans_socket((TransSocket*)it->second);
		m_trans_sockets_map.erase(it);

		active_socket->assign(SOCKET_INVALID, 0, NULL, m_block_mode); //防止删除到时候重复close掉fd
		delete_trans_socket(active_socket);
		return NULL;
	}
	else
	{
		pair<SocketMap::iterator, bool> pair_ret = m_trans_sockets_map.insert(make_pair(socket_handle, active_socket));
		if(pair_ret.second == false)
		{
			SLOG_ERROR("insert active socket error. delete it");
			delete_trans_socket(active_socket);
			return NULL;
		}
	}

	return (Socket*)active_socket;
}

 Socket* SocketManager::add_passive_trans_socket(SocketHandle trans_fd)
{
	const char *peer_ip = "Unknow ip!!!";
	struct sockaddr_in peer_addr;
	int socket_len = sizeof(peer_addr);
	if(getpeername(trans_fd, (struct sockaddr*)&peer_addr, (socklen_t*)&socket_len) == 0)
		peer_ip = inet_ntoa(peer_addr.sin_addr);
	SLOG_INFO("receive net connect from ip:%s, fd=%d", peer_ip, trans_fd);

	if(init_passive_trans_socket(trans_fd, m_block_mode) == -1)
		return NULL;
	TransSocket *passive_socket = NULL;
	SocketMap::iterator it = m_trans_sockets_map.find(trans_fd);
	if(it == m_trans_sockets_map.end())
	{
		passive_socket = (TransSocket*)new_trans_socket();
		assert(passive_socket!=NULL && passive_socket->get_handle()==SOCKET_INVALID);
		//先保存/注册再赋值, 防止socket被关闭掉
		pair<SocketMap::iterator, bool> pair_ret = m_trans_sockets_map.insert(make_pair(trans_fd, passive_socket));
		if(pair_ret.second == false)
		{
			SLOG_ERROR("passive socket insert into map failed");
			delete_trans_socket(passive_socket);
			return NULL;
		}
		passive_socket->assign(trans_fd, -1, peer_ip, m_block_mode);
	}
	else
		SLOG_WARN("passive trans socket already exist in socket manager. fd=%d", trans_fd);

	return passive_socket;
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

int SocketManager::remove_trans_socket(SocketHandle socket_handle)
{	
	SLOG_DEBUG("remove trans socket from socket manager. close fd=%d", socket_handle);
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it != m_trans_sockets_map.end())
	{
		TransSocket *trans_socket = (TransSocket *)it->second;
		m_trans_sockets_map.erase(it);
		m_trans_socket_memcache.Free(trans_socket);
	}

	return 0;
}


Socket* SocketManager::find_trans_socket(SocketHandle socket_handle)
{
	SocketMap::iterator it = m_trans_sockets_map.find(socket_handle);
	if(it == m_trans_sockets_map.end())
		return NULL;
	else
		return it->second;
}

