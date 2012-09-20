/*
 * DownloadServer.h
 *
 *  Created on: 2012-9-16
 *      Author: LiuYongjin
 */

#ifndef APP_SERVER_DOWNLOAD_SERVER_H_
#define APP_SERVER_DOWNLOAD_SERVER_H_

#include "ConnectThread.h"
#include "ConnectThreadPool.h"
#include "DownloadProtocol.h"

class DownloadServer:public ConnectThread
{
public:
	DownloadServer()
	{
		init_instance();
	}
	ProtocolFamily* create_protocol_family(){return new DownloadProtocolFamily;}


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

protected://实现Thread到纯虚函数
	void run();
};

class DownloadThreadPool:public ConnectThreadPool
{
public:
	DownloadThreadPool(unsigned int thread_num):ConnectThreadPool(thread_num){}
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

#endif //APP_SERVER_DOWNLOAD_H_


