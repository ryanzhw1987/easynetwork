/*
 * DownloadManager.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_DOWNLOAD_MANAGER_H_
#define APP_DOWNLOAD_MANAGER_H_

#include <string>
unsing std::string;

#include "ThreadPool.h"
#include "NetInterface.h"

typedef struct _download_task_
{
	string file_name;	//文件名
	unsigned long long start_pos;	//要下载的偏移
	unsigned int size;				//要下载的大小
	int task_index;					//任务索引号
}DownloadTask;

class ThreadPipeHandler:public EventHandler
{
public: //实现EventHandler的接口
	virtual HANDLE_RESULT on_readable(int fd)
	{
		//接收消息,把管道到数据全部读取出来
		//很快,一般只循环一次;链接发得太快,导致很多消息堵塞...但是没有关系
		char buf[100];
		while(read(fd, buf, 100) > 0)
			;
		SLOG_DEBUG("Thread[ID=%d, Addr=%x] pipe fd=%d reable able", m_thread->get_id(),m_thread,fd);
		m_thread->do_task(); //处理任务
		return HANDLE_OK;
	}
public:
	ThreadPipeHandler(Thread<SocketHandle> *thread):m_thread(thread){}
private:
	Thread<SocketHandle> *m_thread;
};

class DownloadThread:public Thread<DownloadTask *>, public NetInterface
{
public://实现Thread的接口
	bool do_task();
protected://实现Thread的接口
	bool notify_add_task();
	void run();

public:
	DownloadThread(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager);
private:
	int m_pipe[2];
	ThreadPipeHandler m_pipe_handler;

////////////////////////////////////////////////
////////////////////////////////////////////////
//////////                            //////////
//////////   应用层重写事件响应函数  //////////
//////////                            //////////
////////////////////////////////////////////////
////////////////////////////////////////////////
public:
	//////////////////由应用层重写 接收协议函数//////////////////
	virtual int on_recv_protocol(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送错误处理函数//////////
	virtual int on_protocol_send_error(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 协议发送成功处理函数//////////
	virtual int on_protocol_send_succ(SocketHandle socket_handle, Protocol *protocol)=0;
	//////////////////由应用层重写 连接错误处理函数//////////////
	virtual int on_socket_handle_error(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 连接超时处理函数//////////////
	virtual int on_socket_handle_timeout(SocketHandle socket_handle)=0;
	//////////////////由应用层重写 收到一个新的连接请求////////
	virtual int on_socket_handler_accpet(SocketHandle socket_handle){return 0;}
};

class DownloadThreadPool:public ThreadPool<DownloadTask *>
{
protected://实现基类接口
	//创建一个线程
	Thread<DownloadTask*>* create_thread();
public:
	DownloadThreadPool(unsigned int thread_num):ThreadPool<DownloadTask *>(thread_num){}
};

inline
Thread<DownloadTask*>* DownloadThreadPool::create_thread()
{

}

#endif //APP_DOWNLOAD_MANAGER_H_



