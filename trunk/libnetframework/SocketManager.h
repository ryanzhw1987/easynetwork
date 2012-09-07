#ifndef _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_
#define _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

#include "Socket.h"
#include "MemManager.h"

#include <map>
using std::map;
typedef map<SocketHandle, Socket*> SocketMap;

class SocketManager
{
public:
	SocketManager(BlockMode block_mode=NOBLOCK):m_block_mode(bloc_mode){}
	virtual ~SocketManager();

	//创建主动传输连接
	virtual SocketHandle create_active_trans_socket(const char *ip, int port);
	virtual Socket* add_passive_trans_socket(SocketHandle trans_fd);
	//删除(主/被动)传输连接,取消掉堵塞在其队列上的协议,同时通知应用层(主要由框架调用)
	virtual int delete_trans_socket(SocketHandle socket_handle);
	//查找(主/被动)传输连接.(主要由框架调用)
	virtual Socket* find_trans_socket(SocketHandle socket_handle);

protected:
	virtual Socket* new_trans_socket();	//创建具体的trans socket
	virtual int init_passive_trans_socket(SocketHandle socket_handle, BlockMode block_mode); //对添加的trans socket进行必要的初始化
private:
	BlockMode m_block_mode;	//连接的阻塞模式
	SocketMap m_trans_sockets_map;    //传输socket
	MemCache<TransSocket> m_trans_socket_memcache;
	virtual int delete_trans_socket(TransSocket *socket_handle);
};

#endif // _LIB_SOCKET_MANAGER_H_20120619_LIUYONGJIN_

