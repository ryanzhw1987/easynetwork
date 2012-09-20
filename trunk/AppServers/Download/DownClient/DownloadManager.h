/*
 * DownloadManager.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_DOWNLOAD_MANAGER_H_
#define APP_DOWNLOAD_MANAGER_H_

#include <string>
using std::string;

#include "PipeThread.h"
#include "ThreadPool.h"
#include "NetInterface.h"

#include <stdio.h>
#include <map>
using std::map;
using std::make_pair;

//分片下载任务
typedef struct _download_task_
{
	string file_name;	//文件名
	unsigned long long start_pos;	//要下载的偏移
	unsigned int size;				//要下载的大小
	int task_index;					//任务索引号
	FILE* fp;
	unsigned int down_size;
}DownloadTask;

typedef map<string, DownloadTask*> DownloadMap;

class DownloadThread:public NetInterface, public PipeThread<DownloadTask* >
{
protected:
	//实现接口:线程实际运行的入口
	void run_thread();
	//实现接口:响应添加任务事件
	bool on_notify_add_task();
	//实现接口:注册管道事件
	bool register_notify_handler(int write_pipe, EVENT_TYPE event_type, EventHandler* event_handler);
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
	int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送错误处理函数//////////
	int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 协议发送成功处理函数//////////
	int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol);
	//////////////////由应用层重写 连接错误处理函数//////////////
	int on_socket_handle_error(SocketHandle socket_handle);
	//////////////////由应用层重写 连接超时处理函数//////////////
	int on_socket_handle_timeout(SocketHandle socket_handle);
	//////////////////由应用层重写 收到一个新的连接请求////////
	int on_socket_handler_accpet(SocketHandle socket_handle){return 0;}

public:
	DownloadThread():m_is_downloading(false){}

protected:
	bool send_download_task(SocketHandle socket_handle=SOCKET_INVALID);
	bool send_download_task(SocketHandle socket_handle, DownloadTask* download_task);
private:
	DownloadMap m_downloading_task;
	bool m_is_downloading;	//是否正在下载
};

class DownloadThreadPool:public ThreadPool<DownloadTask *>
{
protected://实现基类接口
	//创建一个线程
	Thread<DownloadTask*>* create_thread();
public:
	DownloadThreadPool(unsigned int thread_num):ThreadPool<DownloadTask *>(thread_num){}
};


#endif //APP_DOWNLOAD_MANAGER_H_



