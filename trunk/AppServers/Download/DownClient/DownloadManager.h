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

public:
	DownloadThread():m_is_downloading(false){}

protected:
	bool send_download_task(SocketHandle socket_handle=SOCKET_INVALID);
	bool send_download_task(SocketHandle socket_handle, DownloadTask* download_task);
private:
	DownloadMap m_downloading_task;
	bool m_is_downloading;	//是否正在下载
};

class DownloadWorkerPool:public ThreadPool<DownloadTask *>
{
protected://实现基类接口
	//创建一个线程
	Thread<DownloadTask*>* create_thread();
public:
	DownloadWorkerPool(unsigned int thread_num):ThreadPool<DownloadTask *>(thread_num){}
};


#endif //APP_DOWNLOAD_MANAGER_H_



