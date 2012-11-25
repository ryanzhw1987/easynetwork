/*
 * DownloadServer.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_SFS_MASTER_SERVER_H_
#define APP_SFS_MASTER_SERVER_H_

#include "ConnectThread.h"
#include "ConnectThreadPool.h"

#include <stdint.h>
#include <string>
#include <map>
using std::string;
using std::map;

class MasterServer:public ConnectThread
{
protected:
	////由应用层实现 -- 创建具体的协议族
	virtual ProtocolFamily* create_protocol_family();
	////由应用层实现 -- 销毁协议族
	virtual void delete_protocol_family(ProtocolFamily* protocol_family);

	////由应用层实现 -- 接收协议函数
	bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol);
	////由应用层实现 -- 协议发送错误处理函数
	bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	////由应用层实现 -- 协议发送成功处理函数
	bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	////由应用层实现 -- 连接错误处理函数
	bool on_socket_handle_error(SocketHandle socket_handle);
	////由应用层实现 -- 连接超时处理函数
	bool on_socket_handle_timeout(SocketHandle socket_handle);
	////由应用层实现 -- 已经收到一个新的连接请求
	virtual bool on_socket_handler_accpet(SocketHandle socket_handle);
public:
	////由应用层实现 -- net interface实例启动入口
	bool start_server();

private:
	map<string, ChunkInfo> m_chunk_manager;
	void add_chunk(ChunkInfo &chunk_ping);
	bool get_chunk(ChunkInfo &chunk_info);

	map<string, FileInfo> m_fileinfo_cache;
};

class MasterThreadPool:public ConnectThreadPool
{
public:
	MasterThreadPool(unsigned int thread_num):ConnectThreadPool(thread_num){}
protected:
	//实现创建一个线程
	Thread<SocketHandle>* create_thread();
};

#endif //APP_SERVER_DOWNLOAD_H_


