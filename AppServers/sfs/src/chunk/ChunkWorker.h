/*
 * ServerAppFramework.h
 *
 *  Created on: 2012-11-09
 *      Author: LiuYongJin
 */

#ifndef APP_SFS_CHUNK_WORKER_H_20121109
#define APP_SFS_CHUNK_WORKER_H_20121109

#include "ConnectThread.h"
#include "ConnectThreadPool.h"

class ChunkWorker:public ConnectThread
{
protected:
	//////////////////由应用层重写 创建IODemuxer//////////////////
	virtual IODemuxer* create_io_demuxer();
	//////////////////由应用层重写 销毁IODemuxer//////////////////
	virtual void delete_io_demuxer(IODemuxer* io_demuxer);
	//////////////////由应用层重写 创建SocketManager//////////////
	virtual SocketManager* create_socket_manager();
	//////////////////由应用层重写 销毁SocketManager//////////////
	virtual void delete_socket_manager(SocketManager* socket_manager);
	//////////////////由应用层重写 创建具体的协议族//////////////
	virtual ProtocolFamily* create_protocol_family();
	//////////////////由应用层重写 销毁协议族////////////////////
	virtual void delete_protocol_family(ProtocolFamily* protocol_family);

	//////////////////由应用层重写 接收协议函数//////////////////
	bool on_recv_protocol(SocketHandle socket_handle, Protocol *protocol, bool &detach_protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	bool on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	bool on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	bool on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	bool on_socket_handle_timeout(SocketHandle socket_handle);
};

class ChunkWorkerPool:public ConnectThreadPool
{
public:
	ChunkWorkerPool(unsigned int thread_num):ConnectThreadPool(thread_num){}
protected:
	//实现创建一个线程
	Thread<SocketHandle>* create_thread();
};

class TimerHandler:public EventHandler
{
public:
	TimerHandler(IODemuxer *demuxer):m_demuxer(demuxer){;}
	HANDLE_RESULT on_timeout(int fd);
private:
	IODemuxer *m_demuxer;
};

#endif //APP_SFS_CHUNK_WORKER_H_20121109
