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

class DownloadThread:public PipeThread<DownloadTask* >, public NetInterface
{
public://实现Thread的接口
	bool do_task();
protected://实现Thread的接口
	void run();
public:
	DownloadThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
			:PipeThread<DownloadTask*>(io_demuxer)
			,NetInterface(io_demuxer, protocol_family, socket_manager){}

protected:
	bool send_download_task(DownloadTask*);
private:
	DownloadMap m_downloading_task;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数  //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
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



