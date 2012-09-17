/*
 * TaskManager.h
 *
 *  Created on: 2012-9-17
 *      Author: LiuYongjin
 */

#ifndef APP_DOWNLOAD_TASK_MANAGER_H_
#define APP_DOWNLOAD_TASK_MANAGER_H_

#include <string>
using std::string;

#include "PipeThread.h"
#include "NetInterface.h"
#include "DownloadManager.h"

class TaskManager: public PipeThread<string>, public NetInterface
{
public:		//实现thread到接口
	bool do_task();
protected:	//实现thread的接口
	void run();
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

////////////////////////////////////////////////
public:
	TaskManager(IODemuxer *io_demuxer, ProtocolFamily *protocol_family, SocketManager *socket_manager)
			:PipeThread<string>(io_demuxer)
			 ,NetInterface(io_demuxer, protocol_family, socket_manager)
	{
		m_download_pool = NULL;
	}

	void set_download_pool(DownloadThreadPool* download_pool)
	{
		m_download_pool = download_pool;
	}
protected:
	bool send_get_filesize_task(string &file_name);
	bool download_task(const string &file_name, unsigned long long file_size);
private:
	DownloadThreadPool *m_download_pool;

};

#endif //APP_DOWNLOAD_TASK_MANAGER_H_

